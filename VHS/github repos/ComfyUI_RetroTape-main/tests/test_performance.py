"""The fast paths, held to producing the same picture as the slow ones."""

from __future__ import annotations

import json
import math
import os
import threading

import numpy as np
import pytest
import torch
from scipy.signal import lfilter

from ComfyUI_RetroTape import ntsc_core


def _frames(count, width=48, height=32):
    x = torch.linspace(0, 1, width).view(1, 1, width, 1).repeat(count, height, 1, 3)
    y = torch.linspace(0, 1, height).view(1, height, 1, 1).repeat(count, 1, width, 3)
    frames = torch.clamp((x + y) * 0.5, 0, 1)
    # Own content per frame, so an out-of-order write would show.
    frames = frames + torch.linspace(0, 0.2, count).view(count, 1, 1, 1)
    return torch.clamp(frames, 0, 1)


LOUD = json.dumps(
    {
        "preset": "Damaged Tape",
        "seed": 4242,
        "vertical_roll": 0.35,
        "head_switching": True,
    }
)


@pytest.fixture
def threads(monkeypatch):
    """Set RETROTAPE_THREADS for one test and put the environment back."""

    def apply(value):
        if value is None:
            monkeypatch.delenv(ntsc_core.THREADS_ENV_VAR, raising=False)
        else:
            monkeypatch.setenv(ntsc_core.THREADS_ENV_VAR, str(value))

    return apply


def test_a_threaded_batch_is_the_same_bytes_as_a_sequential_one(threads):
    images = _frames(8)
    threads(1)
    sequential = ntsc_core.process_tensor_batch(images, LOUD)
    threads(4)
    assert ntsc_core.frame_worker_count(8) == 4
    threaded = ntsc_core.process_tensor_batch(images, LOUD)
    assert torch.equal(sequential, threaded)


def test_frames_keep_their_order_when_workers_finish_out_of_it(threads):
    images = _frames(8)
    threads(1)
    sequential = ntsc_core.process_tensor_batch(images, LOUD)
    threads(4)
    threaded = ntsc_core.process_tensor_batch(images, LOUD)
    for index in range(images.shape[0]):
        assert torch.equal(sequential[index], threaded[index]), f"frame {index} landed in the wrong slot"


def test_frame_offset_and_alpha_survive_the_threaded_path(threads):
    images = torch.rand(6, 24, 24, 4, generator=torch.Generator().manual_seed(3))
    threads(1)
    sequential = ntsc_core.process_tensor_batch(images, LOUD, 120)
    threads(4)
    threaded = ntsc_core.process_tensor_batch(images, LOUD, 120)
    assert threaded.shape == (6, 24, 24, 4)
    assert torch.equal(sequential, threaded)


@pytest.mark.parametrize(
    "value,expected",
    [("0", 1), ("1", 1), ("2", 2), ("3", 3), (" 4 ", 4), ("999", 8)],
)
def test_the_thread_count_environment_variable_is_honoured(threads, value, expected):
    threads(value)
    assert ntsc_core.frame_worker_count(8) == expected


def test_a_thread_count_above_the_default_really_runs_that_many_at_once(threads):
    """The count used to be honoured while the shared pool stayed fixed at eight."""
    threads(16)
    workers = ntsc_core.frame_worker_count(32)
    assert workers == 16
    pool = ntsc_core._frame_pool(workers)
    barrier = threading.Barrier(workers, timeout=20)
    for future in [pool.submit(barrier.wait) for _ in range(workers)]:
        future.result(timeout=20)


def test_the_shared_pool_stays_at_the_default_size_without_an_override(threads):
    threads(None)
    pool = ntsc_core._frame_pool(ntsc_core.frame_worker_count(64))
    assert pool._max_workers == ntsc_core.MAX_FRAME_THREADS


def test_the_frame_list_helper_keeps_its_order_across_worker_counts(threads):
    frames = [np.full((16, 24, 3), index * 20, np.uint8) for index in range(9)]
    threads(1)
    sequential = ntsc_core.process_rgb_frames(frames, LOUD, range(9))
    threads(4)
    streamed = ntsc_core.process_rgb_frames(frames, LOUD, range(9))
    assert len(streamed) == len(frames)
    for index in range(len(frames)):
        assert np.array_equal(sequential[index], streamed[index]), f"frame {index} landed in the wrong slot"


def test_an_unreadable_thread_count_falls_back_to_the_automatic_one(threads):
    threads("plenty")
    automatic = min(os.cpu_count() or 1, 8, 8)
    assert ntsc_core.frame_worker_count(8) == automatic


def test_the_worker_count_never_exceeds_the_frames_in_the_batch(threads):
    threads(None)
    assert ntsc_core.frame_worker_count(2) <= 2
    assert ntsc_core.frame_worker_count(1) == 1
    assert ntsc_core.frame_worker_count(64) <= ntsc_core.MAX_FRAME_THREADS


def test_a_single_frame_still_renders_with_threading_switched_off(threads):
    threads(1)
    one = ntsc_core.process_tensor_batch(_frames(1), LOUD)
    assert one.shape == (1, 32, 48, 3)


class _Cancelled(Exception):
    pass


def test_an_interrupt_cancels_a_threaded_batch_without_rendering_it_all(threads, monkeypatch):
    images = _frames(64)
    threads(4)

    rendered = []
    original = ntsc_core._render_frame

    def counting_render(image, settings, frame_index):
        rendered.append(frame_index)
        return original(image, settings, frame_index)

    checks = []

    def interrupt_on_the_second_check():
        checks.append(1)
        if len(checks) > 1:
            raise _Cancelled()

    monkeypatch.setattr(ntsc_core, "_render_frame", counting_render)
    monkeypatch.setattr(ntsc_core, "_raise_if_interrupted", interrupt_on_the_second_check)

    with pytest.raises(_Cancelled):
        ntsc_core.process_tensor_batch(images, ntsc_core.DEFAULT_SETTINGS_JSON)

    # Only what a worker already holds may run; the rest is cancelled.
    assert len(rendered) < images.shape[0]
    assert len(rendered) <= 4 * 2 + 1


def test_a_worker_exception_reaches_the_caller(threads, monkeypatch):
    threads(4)
    original = ntsc_core._render_frame

    def fail_on_frame_three(image, settings, frame_index):
        if frame_index == 3:
            raise RuntimeError("frame 3 went wrong")
        return original(image, settings, frame_index)

    monkeypatch.setattr(ntsc_core, "_render_frame", fail_on_frame_three)
    with pytest.raises(RuntimeError, match="frame 3 went wrong"):
        ntsc_core.process_tensor_batch(_frames(16), ntsc_core.DEFAULT_SETTINGS_JSON)


def test_the_progress_bar_is_advanced_once_per_frame(threads, monkeypatch):
    threads(4)
    ticks = []
    monkeypatch.setattr(
        ntsc_core,
        "_make_progress_bar",
        lambda total: type("Bar", (), {"update": lambda self, n: ticks.append(n)})(),
    )
    ntsc_core.process_tensor_batch(_frames(8), ntsc_core.DEFAULT_SETTINGS_JSON)
    assert sum(ticks) == 8


def test_the_preview_frame_helper_matches_frame_by_frame_rendering(threads):
    frames = [(np.random.default_rng(n).random((32, 40, 3)) * 255).astype(np.uint8) for n in range(5)]
    indices = [0, 7, 11, 12, 30]
    threads(1)
    one_at_a_time = [
        ntsc_core.process_rgb_uint8(frame, LOUD, frame_index=n) for frame, n in zip(frames, indices)
    ]
    threads(4)
    together = ntsc_core.process_rgb_frames(frames, LOUD, indices)
    assert len(together) == len(one_at_a_time)
    for expected, actual in zip(one_at_a_time, together):
        assert np.array_equal(expected, actual)


def _three_one_pole_passes(rows, cutoff, h_scale):
    """The line filter as three identical passes in series."""
    tau = 1.0 / (cutoff * 2.0 * math.pi)
    interval = 1.0 / (ntsc_core.NTSC_RATE * max(0.05, h_scale))
    alpha = interval / (tau + interval)
    out = rows.astype(np.float32, copy=True)
    for _ in range(3):
        out = lfilter([alpha], [1.0, -(1.0 - alpha)], out, axis=1).astype(np.float32)
    return out


@pytest.mark.parametrize("cutoff", [300_000.0, 600_000.0, 1_300_000.0, 1_900_000.0])
@pytest.mark.parametrize("h_scale", [0.5, 1.0, 2.6667, 5.3333])
def test_the_cascaded_line_filter_matches_three_passes_in_series(cutoff, h_scale):
    rows = (np.random.default_rng(9).random((64, 512)).astype(np.float32) * 255.0) - 128.0
    reference = _three_one_pole_passes(rows, cutoff, h_scale)
    merged = ntsc_core._lowpass_rows(rows, cutoff, h_scale=h_scale)
    assert merged.dtype == np.float32
    # Convolving the sections is the same filter, minus two float32 roundings.
    assert float(np.abs(merged - reference).max()) < 0.01


def test_the_cascade_is_a_pure_function_of_its_arguments():
    first = ntsc_core._one_pole_cascade(0.25, 3)
    second = ntsc_core._one_pole_cascade(0.25, 3)
    assert np.array_equal(first[0], second[0])
    assert np.array_equal(first[1], second[1])


def test_the_line_filter_leaves_a_flat_picture_flat():
    """Unit gain at DC, as the three sections it replaces each had."""
    rows = np.full((8, 4096), 137.0, dtype=np.float32)
    for cutoff in (300_000.0, 1_900_000.0):
        out = ntsc_core._lowpass_rows(rows, cutoff, h_scale=4.0)
        assert float(np.abs(out[:, 3072:] - 137.0).max()) < 0.005


def _dropout_mask(seed, strength, shape):
    settings = dict(ntsc_core._compact_from_preset("RGM"))
    settings["dropouts"] = strength
    settings["_h_scale"] = shape[1] / ntsc_core.REFERENCE_WIDTH
    settings["_v_scale"] = shape[0] / ntsc_core.REFERENCE_HEIGHT
    y = np.full(shape, 128.0, dtype=np.float32)
    # Only the bands touch chroma, so a plane of ones maps where they landed.
    i = np.ones(shape, dtype=np.float32)
    q = np.ones(shape, dtype=np.float32)
    _, out_i, _ = ntsc_core._apply_dropouts(y, i, q, settings, np.random.default_rng(seed))
    return out_i != 1.0


@pytest.mark.parametrize("seed,strength,shape", [(77, 0.4, (480, 720)), (5, 0.58, (240, 320))])
def test_dropout_bands_are_pinned_to_the_seed(seed, strength, shape):
    """Where the head loses contact is deterministic tape mechanics, not grain:
    a seed puts the bands in the same rows, another seed does not."""
    touched = _dropout_mask(seed, strength, shape)
    assert np.array_equal(touched, _dropout_mask(seed, strength, shape))
    assert not np.array_equal(touched, _dropout_mask(seed + 1, strength, shape))

    rows = np.flatnonzero(touched.any(axis=1))
    bands = 1 + int((np.diff(rows) > 1).sum())
    assert bands > 1, "the head loses contact in several separate bands"
    assert rows.size < shape[0] * 0.2, "the bands are a small part of the frame"


def test_dropout_speckle_is_sparse_and_lands_on_luma_only():
    shape = (240, 320)
    settings = dict(ntsc_core._compact_from_preset("RGM"))
    settings["dropouts"] = 1.0
    settings["_h_scale"] = 1.0
    settings["_v_scale"] = 1.0
    y = np.full(shape, 128.0, dtype=np.float32)
    i = np.zeros(shape, dtype=np.float32)
    q = np.zeros(shape, dtype=np.float32)
    out_y, out_i, out_q = ntsc_core._apply_dropouts(y, i, q, settings, np.random.default_rng(3))
    changed = int((out_y != 128.0).sum())
    assert 0 < changed < out_y.size * 0.05
    assert np.array_equal(out_i, np.zeros(shape, dtype=np.float32))
    assert np.array_equal(out_q, np.zeros(shape, dtype=np.float32))


def test_the_grain_still_has_the_amplitude_the_control_asks_for():
    """float32 noise fields change the pattern for a seed, not what the control does."""
    shape = (256, 256)
    settings = dict(ntsc_core._compact_from_preset("RGM"))
    settings.update(
        {"_h_scale": 1.0, "_v_scale": 1.0, "chroma_noise": 0, "chroma_phase_noise": 0, "chroma_loss": 0}
    )
    flat = np.full(shape, 128.0, dtype=np.float32)
    zero = np.zeros(shape, dtype=np.float32)

    settings["video_noise"] = 9000
    loud, _, _ = ntsc_core._apply_noise(
        flat.copy(), zero.copy(), zero.copy(), settings, np.random.default_rng(1)
    )
    settings["video_noise"] = 2250
    quiet, _, _ = ntsc_core._apply_noise(
        flat.copy(), zero.copy(), zero.copy(), settings, np.random.default_rng(1)
    )
    assert abs(float((loud - flat).mean())) < 0.5
    ratio = float((loud - flat).std()) / float((quiet - flat).std())
    assert 3.8 < ratio < 4.2
