"""The device choice, from the node's combo through to the preview endpoint.

A choice that cannot be met costs speed and not the run, and the preview renders
on the device the node will render on - the two paths draw different grain.
"""

from __future__ import annotations

import asyncio
import io
import json
import logging
import sys
from types import SimpleNamespace

import cv2
import numpy as np
import pytest
import torch
from PIL import Image

from ComfyUI_RetroTape import device as device_module
from ComfyUI_RetroTape import nodes, ntsc_core, ntsc_gpu, preview_cache, server
from conftest import fake_folder_paths, route_handlers

SETTINGS = json.dumps({"preset": "Rental Copy"})

HAS_CUDA = ntsc_gpu.is_available()


def _busy_frame(width, height, seed=7):
    """Detail at every spatial frequency, so a difference in the fine grain shows."""
    rng = np.random.default_rng(seed)
    x = np.linspace(0, 255, width, dtype=np.float32)[None, :]
    y = np.linspace(0, 255, height, dtype=np.float32)[:, None]
    frame = np.empty((height, width, 3), np.float32)
    frame[..., 0] = x * 0.5 + y * 0.5
    frame[..., 1] = 255 - y * 0.7
    frame[..., 2] = 40 + x * 0.6
    frame += rng.normal(0.0, 26.0, frame.shape)
    for column in range(0, width, max(1, width // 16)):
        frame[:, column : column + max(1, width // 96)] = 250
    return np.clip(frame, 0, 255).astype(np.uint8)


def _write_source(tmp_path, monkeypatch, frame, name="source.png"):
    monkeypatch.setitem(sys.modules, "folder_paths", fake_folder_paths(tmp_path))
    monkeypatch.delenv(server.EXTRA_ROOTS_ENV, raising=False)
    Image.fromarray(frame).save(tmp_path / name)
    preview_cache.CACHE.clear()
    return {"filename": name, "type": "input"}


def _decode(result, index=0):
    image = Image.open(io.BytesIO(result.body))
    image.seek(index)
    return np.asarray(image.convert("RGB"))


def _tensor(frame):
    return torch.from_numpy(frame.astype(np.float32) / 255.0)[None]


def _set_cuda(monkeypatch, present):
    """Pretend this machine has, or has not, a CUDA device."""
    monkeypatch.setattr(ntsc_gpu, "is_available", lambda: bool(present))
    monkeypatch.setattr(device_module, "_WARNED_NO_CUDA", False)
    monkeypatch.setattr(device_module, "_WARNED_FALLBACK", False)
    monkeypatch.setattr(device_module, "_GPU_UNUSABLE", False)




@pytest.mark.parametrize(
    "present, expected",
    [(True, {"auto": "cuda", "cpu": "cpu", "gpu": "cuda"}), (False, {"auto": "cpu", "cpu": "cpu", "gpu": "cpu"})],
)
def test_every_choice_resolves_with_and_without_cuda(monkeypatch, present, expected):
    _set_cuda(monkeypatch, present)
    for choice, resolved in expected.items():
        assert device_module.resolve_device(choice) == resolved


def test_an_unknown_or_missing_choice_is_auto(monkeypatch):
    """A workflow saved before this input existed carries no value at all."""
    _set_cuda(monkeypatch, True)
    for choice in (None, "", "TPU", 7):
        assert device_module.resolve_device(choice) == "cuda"
    _set_cuda(monkeypatch, False)
    assert device_module.resolve_device(None) == "cpu"


def test_gpu_without_cuda_warns_once_and_does_not_raise(monkeypatch, caplog):
    _set_cuda(monkeypatch, False)
    with caplog.at_level(logging.WARNING, logger=device_module.__name__):
        assert [device_module.resolve_device("gpu") for _ in range(5)] == ["cpu"] * 5
    assert len([record for record in caplog.records if "no CUDA device" in record.message]) == 1




def test_the_node_falls_back_to_the_cpu_when_cuda_faults(monkeypatch):
    _set_cuda(monkeypatch, True)
    calls = []

    def explode(*args, **kwargs):
        calls.append(args)
        raise ntsc_gpu.RetroTapeGpuError("no memory")

    monkeypatch.setattr(ntsc_gpu, "process_tensor_batch", explode)
    image = _tensor(_busy_frame(64, 48))

    result = nodes.RetroTape().apply(image, SETTINGS, device="gpu")

    assert len(calls) == 1, "the GPU was tried before the fallback"
    assert torch.equal(result["result"][0], ntsc_core.process_tensor_batch(image, SETTINGS, 0))


def _raw_cuda_errors():
    """The failures ntsc_gpu's own try does not wrap: its outer torch.empty and
    the alpha torch.cat sit outside it."""
    oom = getattr(torch, "OutOfMemoryError", None)
    errors = [RuntimeError("CUDA error: out of memory")]
    if oom is not None:
        errors.append(oom("CUDA out of memory."))
    return errors


@pytest.mark.parametrize("error", _raw_cuda_errors(), ids=lambda e: type(e).__name__)
def test_an_unwrapped_cuda_error_still_falls_back(monkeypatch, error):
    _set_cuda(monkeypatch, True)

    def explode(*args, **kwargs):
        raise error

    monkeypatch.setattr(ntsc_gpu, "process_tensor_batch", explode)
    image = _tensor(_busy_frame(48, 32))

    result = nodes.RetroTape().apply(image, SETTINGS, device="gpu")

    assert torch.equal(result["result"][0], ntsc_core.process_tensor_batch(image, SETTINGS, 0))


def test_an_unwrapped_cuda_error_still_falls_back_in_the_preview(monkeypatch):
    fake = _FakeGpu(fail=True, error=RuntimeError("CUDA error: an illegal memory access"))
    _install_fake_gpu(monkeypatch, fake)
    frame = _busy_frame(64, 48)

    rendered = server._encode_preview(frame[None], SETTINGS, max_side=64, device="gpu")

    assert rendered.device == "cpu"
    assert np.array_equal(_decode(rendered), ntsc_core.process_rgb_uint8(frame, SETTINGS, frame_index=0))


def test_a_failure_that_is_not_cuda_is_not_swallowed(monkeypatch):
    _set_cuda(monkeypatch, True)

    def explode(*args, **kwargs):
        raise ValueError("IMAGE tensors must carry 1, 3 or 4 channels.")

    monkeypatch.setattr(ntsc_gpu, "process_tensor_batch", explode)
    with pytest.raises(ValueError):
        nodes.RetroTape().apply(_tensor(_busy_frame(48, 32)), SETTINGS, device="gpu")


def test_a_poisoned_context_is_not_retried_on_the_gpu(monkeypatch):
    """A device-side assert leaves the CUDA context unusable, so the next render
    must not reach for the card again."""
    _set_cuda(monkeypatch, True)
    calls = []

    def explode(*args, **kwargs):
        calls.append(1)
        raise RuntimeError("CUDA error: device-side assert triggered")

    monkeypatch.setattr(ntsc_gpu, "process_tensor_batch", explode)
    image = _tensor(_busy_frame(48, 32))

    for _ in range(3):
        result = nodes.RetroTape().apply(image, SETTINGS, device="gpu")
        assert torch.equal(result["result"][0], ntsc_core.process_tensor_batch(image, SETTINGS, 0))
    assert len(calls) == 1


def test_the_fallback_does_not_start_a_second_progress_bar(monkeypatch):
    """ntsc_gpu ticks a bar of its own; a CPU re-render from zero ran it backwards."""
    _set_cuda(monkeypatch, True)
    bars = []

    class _Bar:
        def __init__(self, total):
            self.total = total
            self.done = 0
            bars.append(self)

        def update(self, count):
            self.done += count

    monkeypatch.setattr(ntsc_core, "_make_progress_bar", _Bar)

    def explode(*args, **kwargs):
        raise ntsc_gpu.RetroTapeGpuError("no memory")

    monkeypatch.setattr(ntsc_gpu, "process_tensor_batch", explode)
    nodes.RetroTape().apply(torch.rand(4, 24, 32, 3), SETTINGS, device="gpu")

    assert len(bars) == 1
    assert bars[0].done == 4


def test_every_device_choice_renders_on_the_cpu_without_cuda(monkeypatch):
    _set_cuda(monkeypatch, False)
    image = _tensor(_busy_frame(48, 32))
    for choice in ("auto", "cpu", "gpu"):
        result = nodes.RetroTape().apply(image, SETTINGS, device=choice)
        assert torch.equal(result["result"][0], ntsc_core.process_tensor_batch(image, SETTINGS, 0))




class _FakeGpu:
    """ntsc_gpu's stand-in; its render is a picture nothing else produces."""

    RetroTapeGpuError = ntsc_gpu.RetroTapeGpuError

    def __init__(self, fail=False, error=None):
        self.fail = bool(fail)
        self.error = error or ntsc_gpu.RetroTapeGpuError("no memory")
        self.frames = []

    def is_available(self):
        return True

    def process_rgb_uint8(self, frame, settings_json=None, *, frame_index=0, device="cuda"):
        if self.fail:
            raise self.error
        self.frames.append(int(frame_index))
        return np.full_like(frame, 200)


def _install_fake_gpu(monkeypatch, fake):
    # Availability is read through device.py and the render through server.py.
    monkeypatch.setattr(device_module, "ntsc_gpu", fake)
    monkeypatch.setattr(server, "ntsc_gpu", fake)
    monkeypatch.setattr(device_module, "_WARNED_FALLBACK", False)
    monkeypatch.setattr(device_module, "_GPU_UNUSABLE", False)


def test_the_preview_renders_on_the_device_it_was_asked_for(monkeypatch):
    fake = _FakeGpu()
    _install_fake_gpu(monkeypatch, fake)
    frames = _busy_frame(64, 48)[None]

    rendered = server._encode_preview(frames, SETTINGS, max_side=64, frame_offset=3, device="gpu")

    assert rendered.device == "cuda"
    assert fake.frames == [3], "the frame number has to survive the dispatch"
    assert np.array_equal(_decode(rendered), np.full((48, 64, 3), 200, np.uint8))


def test_the_preview_stays_on_the_cpu_when_the_node_is_on_the_cpu(monkeypatch):
    _install_fake_gpu(monkeypatch, _FakeGpu())
    frame = _busy_frame(64, 48)

    rendered = server._encode_preview(frame[None], SETTINGS, max_side=64, device="cpu")

    assert rendered.device == "cpu"
    assert np.array_equal(_decode(rendered), ntsc_core.process_rgb_uint8(frame, SETTINGS, frame_index=0))


def test_a_cuda_fault_mid_preview_falls_back_and_says_so(monkeypatch):
    _install_fake_gpu(monkeypatch, _FakeGpu(fail=True))
    frame = _busy_frame(64, 48)

    rendered = server._encode_preview(frame[None], SETTINGS, max_side=64, device="gpu")

    assert rendered.device == "cpu"
    assert np.array_equal(_decode(rendered), ntsc_core.process_rgb_uint8(frame, SETTINGS, frame_index=0))


def test_an_animated_preview_dispatches_every_frame_to_the_same_device(monkeypatch):
    fake = _FakeGpu()
    _install_fake_gpu(monkeypatch, fake)
    frames = np.stack([_busy_frame(64, 48, seed=n) for n in range(4)])

    rendered = server._encode_preview(frames, SETTINGS, max_side=64, frame_offset=10, device="gpu")

    assert rendered.device == "cuda"
    assert fake.frames == [10, 11, 12, 13]




def _handler(monkeypatch):
    return route_handlers(monkeypatch)[("POST", server.ROUTE_PATH)]


def _clear_request(payload=None):
    async def read():
        return payload

    return SimpleNamespace(json=read, can_read_body=payload is not None)


def test_the_clear_cache_route_frees_the_preview_cache(monkeypatch):
    handler = route_handlers(monkeypatch)[("POST", server.CLEAR_CACHE_ROUTE_PATH)]
    preview_cache.set_preview_frames("held", np.zeros((2, 64, 64, 3), np.uint8))
    held = preview_cache.CACHE.total_bytes
    assert held > 0

    response = asyncio.run(handler(_clear_request()))

    body = json.loads(response.body)
    assert body["freed_bytes"] == held
    assert body["node_entries"] == 1
    assert preview_cache.CACHE.total_bytes == 0
    assert preview_cache.get_preview_entry("held") is None


def test_the_clear_cache_route_can_spare_what_a_run_produced(monkeypatch):
    handler = route_handlers(monkeypatch)[("POST", server.CLEAR_CACHE_ROUTE_PATH)]
    preview_cache.CACHE.clear()
    preview_cache.set_preview_frames("held", np.zeros((2, 64, 64, 3), np.uint8))
    preview_cache.CACHE.put(("video", 1), np.zeros((2, 64, 64, 3), np.uint8), kind=preview_cache.SOURCE_KIND)

    response = asyncio.run(handler(_clear_request({"sources_only": True})))

    body = json.loads(response.body)
    assert (body["node_entries"], body["source_entries"]) == (0, 1)
    assert preview_cache.get_preview_entry("held") is not None
    preview_cache.CACHE.clear()


@pytest.mark.parametrize("payload", [{"sources_only": "yes"}, [1, 2], "nope"])
def test_the_clear_cache_route_refuses_a_body_it_cannot_read(monkeypatch, payload):
    handler = route_handlers(monkeypatch)[("POST", server.CLEAR_CACHE_ROUTE_PATH)]
    preview_cache.CACHE.clear()
    preview_cache.set_preview_frames("held", np.zeros((2, 64, 64, 3), np.uint8))

    response = asyncio.run(handler(_clear_request(payload)))

    assert response.status == 400
    assert preview_cache.get_preview_entry("held") is not None, "a refused body clears nothing"
    preview_cache.CACHE.clear()


def _request(payload):
    async def read():
        return payload

    return SimpleNamespace(json=read, transport=SimpleNamespace(is_closing=lambda: False))


@pytest.mark.parametrize(
    "sent, expected_choice, expected_header",
    [("gpu", "gpu", "cuda"), ("cpu", "cpu", "cpu"), (None, "auto", "cpu")],
)
def test_the_endpoint_passes_the_choice_through_and_reports_the_result(
    monkeypatch, sent, expected_choice, expected_header
):
    if server.web is None:
        pytest.skip("aiohttp is unavailable")
    seen = []

    def fake_render(settings_json, preview_mode, max_side, node_id, video, image, frame_offset=0, device="auto"):
        seen.append(device)
        return server.PreviewResult(b"body", "image/png", 1, 64, 48, 64, 48, expected_header)

    monkeypatch.setattr(server, "render_preview", fake_render)
    handler = _handler(monkeypatch)

    payload = {"node_id": "1", "settings_json": SETTINGS}
    if sent is not None:
        payload["device"] = sent
    response = asyncio.run(handler(_request(payload)))

    assert seen == [expected_choice]
    assert response.headers["X-RetroTape-Device"] == expected_header


def _fake_render(monkeypatch):
    def render(settings_json, preview_mode, max_side, node_id, video, image, frame_offset=0, device="auto"):
        render.seen = (frame_offset, device)
        return server.PreviewResult(b"body", "image/png", 1, 64, 48, 64, 48, "cpu")

    monkeypatch.setattr(server, "render_preview", render)
    return render


@pytest.mark.parametrize(
    "payload, status",
    [
        ({"device": "TPU"}, 400),
        ({"device": 7}, 400),
        ({"frame_offset": "seven"}, 400),
        ({"frame_offset": [1]}, 400),
        ({"frame_offset": float("nan")}, 400),
    ],
)
def test_a_bad_frame_offset_or_device_is_refused_like_every_other_field(monkeypatch, payload, status):
    """Both used to be coerced silently while max_side and node_id returned 400."""
    if server.web is None:
        pytest.skip("aiohttp is unavailable")
    _fake_render(monkeypatch)
    handler = _handler(monkeypatch)
    response = asyncio.run(handler(_request({"node_id": "1", **payload})))
    assert response.status == status


@pytest.mark.parametrize("sent, expected", [(-5, 0), (10**9, server.MAX_FRAME_OFFSET), (12, 12)])
def test_a_frame_offset_out_of_range_is_clamped(monkeypatch, sent, expected):
    if server.web is None:
        pytest.skip("aiohttp is unavailable")
    render = _fake_render(monkeypatch)
    handler = _handler(monkeypatch)
    asyncio.run(handler(_request({"node_id": "1", "frame_offset": sent})))
    assert render.seen[0] == expected


def test_every_response_forbids_caching(monkeypatch):
    """A cached 404 outlives the run that would have produced the frames."""
    if server.web is None:
        pytest.skip("aiohttp is unavailable")
    handler = _handler(monkeypatch)

    def missing(*args, **kwargs):
        raise server.PreviewSourceMissing(server._SOURCE_MISSING)

    monkeypatch.setattr(server, "render_preview", missing)
    for payload in ({"node_id": "1"}, {"node_id": "1", "device": "TPU"}, {}):
        response = asyncio.run(handler(_request(payload)))
        assert response.headers["Cache-Control"] == "no-store", payload




@pytest.mark.skipif(not HAS_CUDA, reason="no CUDA device")
def test_the_node_on_gpu_is_the_gpu_render(monkeypatch):
    image = _tensor(_busy_frame(256, 192))
    result = nodes.RetroTape().apply(image, SETTINGS, device="gpu")
    assert torch.equal(result["result"][0], ntsc_gpu.process_tensor_batch(image, SETTINGS, 0))


@pytest.mark.skipif(not HAS_CUDA, reason="no CUDA device")
def test_the_preview_on_gpu_is_the_gpu_output(tmp_path, monkeypatch):
    """Inside the budget the panel is the render, pixel for pixel, per device."""
    frame = _busy_frame(480, 320)
    source = _write_source(tmp_path, monkeypatch, frame)

    rendered = server.render_preview(SETTINGS, "full", 320, None, None, source, 0, "gpu")
    assert rendered.device == "cuda"
    assert (rendered.render_width, rendered.render_height) == (480, 320)

    output = ntsc_gpu.process_tensor_batch(_tensor(frame), SETTINGS, 0)
    output = (output[0].cpu().numpy() * 255.0).round().astype(np.uint8)
    expected = cv2.resize(output, (320, 213), interpolation=cv2.INTER_AREA)
    assert np.array_equal(_decode(rendered), expected)
    preview_cache.CACHE.clear()
