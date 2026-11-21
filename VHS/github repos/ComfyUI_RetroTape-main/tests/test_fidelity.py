"""The panel and the node, held to producing the same picture, not a similar one."""

from __future__ import annotations

import io
import json
import sys

import cv2
import numpy as np
import pytest
import torch
from PIL import Image

from ComfyUI_RetroTape import ntsc_core, preview_cache, server
from conftest import fake_folder_paths

SETTINGS = json.dumps({"preset": "Rental Copy"})

# The controls for which zero is not "off", skipped when a test nulls them all.
_NEUTRAL_AT_THEIR_OWN_VALUE = ("blend", "seed", "saturation")


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


def _node_output(frame, frame_offset=0):
    tensor = torch.from_numpy(frame.astype(np.float32) / 255.0)[None]
    rendered = ntsc_core.process_tensor_batch(tensor, SETTINGS, frame_offset)
    return (rendered[0].numpy() * 255.0).round().astype(np.uint8)


def _preview(*args, device="cpu"):
    """The CPU pairing; tests/test_device.py measures the GPU one the same way."""
    return server.render_preview(*args, device=device)


def _decode(result, index=0):
    image = Image.open(io.BytesIO(result.body))
    image.seek(index)
    return np.asarray(image.convert("RGB"))


def _write_source(tmp_path, monkeypatch, frame, name="source.png"):
    monkeypatch.setitem(sys.modules, "folder_paths", fake_folder_paths(tmp_path))
    monkeypatch.delenv(server.EXTRA_ROOTS_ENV, raising=False)
    Image.fromarray(frame).save(tmp_path / name)
    preview_cache.CACHE.clear()
    return {"filename": name, "type": "input"}




def test_a_still_inside_the_budget_previews_as_the_exact_render(tmp_path, monkeypatch):
    frame = _busy_frame(960, 640)
    source = _write_source(tmp_path, monkeypatch, frame)

    rendered = _preview(SETTINGS, "full", 480, None, None, source)
    assert (rendered.render_width, rendered.render_height) == (960, 640)
    assert (rendered.source_width, rendered.source_height) == (960, 640)

    output = _node_output(frame)
    expected = cv2.resize(output, (480, 320), interpolation=cv2.INTER_AREA)
    assert np.array_equal(_decode(rendered), expected)


def test_a_preview_honours_the_frame_offset(tmp_path, monkeypatch):
    frame = _busy_frame(480, 320)
    source = _write_source(tmp_path, monkeypatch, frame)

    rendered = _preview(SETTINGS, "full", 480, None, None, source, 7)
    assert np.array_equal(_decode(rendered), _node_output(frame, 7))
    assert not np.array_equal(_decode(rendered), _node_output(frame, 0))


def test_an_animated_frame_is_the_render_of_the_source_frame_it_came_from():
    """A long batch is sampled down, so preview frame 3 is not output frame 3."""
    preview_cache.CACHE.clear()
    rng = np.random.default_rng(2)
    batch = torch.from_numpy(
        rng.integers(0, 255, (200, 96, 128, 3), dtype=np.uint8).astype(np.float32) / 255.0
    )
    preview_cache.set_preview_frames("batch", batch)
    indices = preview_cache.get_preview_entry("batch").indices

    rendered = _preview(SETTINGS, "full", 128, "batch", None, None, 5)
    kept = np.linspace(0, len(indices) - 1, rendered.frame_count).round().astype(np.int64)
    shown = indices[kept]
    assert shown[1] > 1, "the sampled batch should not be a contiguous run"

    for slot, source_index in enumerate(shown):
        source = (batch[int(source_index)].numpy() * 255.0).round().astype(np.uint8)
        expected = ntsc_core.process_rgb_uint8(
            source, SETTINGS, frame_index=int(source_index) + 5
        )
        assert np.array_equal(_decode(rendered, slot), expected), "frame %d" % slot
    preview_cache.CACHE.clear()


def test_the_animated_transport_does_not_alter_the_picture():
    rng = np.random.default_rng(4)
    frames = rng.integers(0, 255, (4, 96, 128, 3), dtype=np.uint8)
    rendered = server._encode_preview(frames, SETTINGS, max_side=128, device="cpu")
    assert rendered.content_type == "image/webp"
    for index in range(rendered.frame_count):
        expected = ntsc_core.process_rgb_uint8(frames[index], SETTINGS, frame_index=index)
        assert np.array_equal(_decode(rendered, index), expected)


def test_the_quick_pass_leaves_nothing_behind_for_the_full_pass(tmp_path, monkeypatch):
    """The two passes share a source cache, so the quick one must leave no trace."""
    frame = _busy_frame(480, 320)
    source = _write_source(tmp_path, monkeypatch, frame)

    clean = _preview(SETTINGS, "full", 320, None, None, source)
    _preview(SETTINGS, "fast", 240, None, None, source)
    after_quick = _preview(SETTINGS, "full", 320, None, None, source)

    assert after_quick.render_width == clean.render_width
    assert np.array_equal(_decode(after_quick), _decode(clean))
    preview_cache.CACHE.clear()




def test_a_source_over_the_budget_reports_the_size_it_actually_rendered(tmp_path, monkeypatch):
    """Falling back to a reduced render is allowed; doing it silently is not."""
    monkeypatch.setattr(server, "FULL_PREVIEW_PIXEL_BUDGET", 40_000)
    frame = _busy_frame(400, 400)
    source = _write_source(tmp_path, monkeypatch, frame)

    rendered = _preview(SETTINGS, "full", 400, None, None, source)
    assert (rendered.source_width, rendered.source_height) == (400, 400)
    assert rendered.render_width * rendered.render_height <= 40_000
    assert rendered.render_width < rendered.source_width


def test_the_reported_source_size_is_the_one_the_node_was_given():
    preview_cache.CACHE.clear()
    preview_cache.set_preview_frames("huge", np.zeros((1, 3000, 4000, 3), np.uint8))
    entry = preview_cache.get_preview_entry("huge")
    assert entry.source_size == (4000, 3000)
    assert max(entry.frames.shape[1:3]) <= preview_cache.MAX_CACHED_SIDE

    rendered = _preview(SETTINGS, "fast", 240, "huge", None, None)
    assert (rendered.source_width, rendered.source_height) == (4000, 3000)
    assert rendered.render_width < 4000
    preview_cache.CACHE.clear()


def test_one_cached_batch_cannot_take_the_whole_cache(monkeypatch):
    """Eviction never drops the only entry, so an entry needs a ceiling of its own."""
    preview_cache.CACHE.clear()
    monkeypatch.setattr(preview_cache, "MAX_NODE_ENTRY_BYTES", 400_000)
    preview_cache.set_preview_frames("long", np.zeros((20, 480, 640, 3), np.uint8))
    stored = preview_cache.get_preview_entry("long").frames
    assert preview_cache.CACHE.total_bytes <= preview_cache.MAX_NODE_ENTRY_BYTES
    assert stored.shape[0] == 20, "frames are reduced in size, never dropped"
    assert stored.shape[2] > stored.shape[1], "the aspect ratio has to survive"
    preview_cache.CACHE.clear()


def test_a_batch_is_stored_only_as_deep_as_the_encoder_can_show(monkeypatch):
    """96 frames used to be kept per node while the encoder never showed past 24."""
    preview_cache.CACHE.clear()
    preview_cache.set_preview_frames("long", np.zeros((120, 240, 320, 3), np.uint8))
    entry = preview_cache.get_preview_entry("long")
    assert entry.frames.shape[0] == server.MAX_PREVIEW_FRAMES == preview_cache.MAX_CACHED_FRAMES
    assert int(entry.indices[0]) == 0 and int(entry.indices[-1]) == 119
    assert preview_cache.CACHE.total_bytes == 24 * 240 * 320 * 3
    preview_cache.CACHE.clear()




def test_a_sixteen_bit_source_decodes_to_the_levels_comfyui_loads(tmp_path, monkeypatch):
    """PIL keeps the low byte of a 16-bit sample, which is a wrapped gradient."""
    raw = (np.arange(256, dtype=np.uint16) * 257)[None, :].repeat(4, axis=0)
    monkeypatch.setitem(sys.modules, "folder_paths", fake_folder_paths(tmp_path))
    Image.fromarray(raw).save(tmp_path / "deep.png")
    preview_cache.CACHE.clear()

    loaded = server._load_source_image({"filename": "deep.png", "type": "input"})
    assert np.array_equal(loaded.frames[0, :, 0], np.arange(256, dtype=np.uint8))
    preview_cache.CACHE.clear()


def test_an_eight_bit_source_is_untouched_by_the_depth_handling(tmp_path, monkeypatch):
    frame = _busy_frame(64, 48)
    source = _write_source(tmp_path, monkeypatch, frame)
    assert np.array_equal(server._load_source_image(source).frames, frame)
    preview_cache.CACHE.clear()




def test_the_interlace_comb_is_a_line_pitch_not_a_pixel_pitch():
    settings = dict(ntsc_core.parse_settings_json(None))
    for key, value in list(settings.items()):
        if isinstance(value, (int, float)) and key not in _NEUTRAL_AT_THEIR_OWN_VALUE:
            settings[key] = type(value)(0)
    settings["vhs_enabled"] = False
    settings["head_switching"] = False
    settings["interlace"] = 1.0

    def strength(height):
        frame = _busy_frame(int(height * 1.5), height)
        processed = ntsc_core.process_rgb_uint8(frame, settings, frame_index=1)
        difference = processed.astype(np.float32) - frame.astype(np.float32)
        viewed = cv2.resize(difference, (720, 480), interpolation=cv2.INTER_AREA)
        return float(np.mean(np.abs(viewed)))

    reference = strength(480)
    assert reference > 0.5, "the control should do something at the reference size"
    for height in (960, 1920):
        assert strength(height) > reference * 0.5, "interlace faded away at %d lines" % height


@pytest.mark.parametrize("side", [720, 1440])
def test_a_native_preview_matches_the_render_at_every_size(tmp_path, monkeypatch, side):
    frame = _busy_frame(side, side // 2)
    source = _write_source(tmp_path, monkeypatch, frame, "s%d.png" % side)
    rendered = _preview(SETTINGS, "full", 360, None, None, source)
    expected = cv2.resize(_node_output(frame), (360, 180), interpolation=cv2.INTER_AREA)
    assert np.array_equal(_decode(rendered), expected)


def _fake_video(monkeypatch, tmp_path, count, width, height):
    """A VideoCapture stand-in, so the test does not depend on a codec."""
    monkeypatch.setitem(sys.modules, "folder_paths", fake_folder_paths(tmp_path))
    monkeypatch.delenv(server.EXTRA_ROOTS_ENV, raising=False)
    path = tmp_path / "clip.avi"
    path.write_bytes(b"stub")
    frames = [np.full((height, width, 3), (20 + n * 30) % 256, np.uint8) for n in range(count)]

    class FakeCapture:
        def __init__(self, _path):
            self.position = 0

        def isOpened(self):
            return True

        def get(self, prop):
            if prop == cv2.CAP_PROP_FRAME_COUNT:
                return count
            if prop == cv2.CAP_PROP_POS_FRAMES:
                return self.position
            return 0

        def set(self, prop, value):
            if prop == cv2.CAP_PROP_POS_FRAMES:
                self.position = int(value)

        def read(self):
            if self.position >= count:
                return False, None
            frame = frames[self.position]
            self.position += 1
            return True, frame

        def release(self):
            pass

    monkeypatch.setattr(server.cv2, "VideoCapture", FakeCapture)
    preview_cache.CACHE.clear()
    return {"filename": str(path), "path": True}


def test_a_short_clip_previews_at_its_own_resolution(tmp_path, monkeypatch):
    """The decode budget used to be shared out over the frames the client was
    allowed to ask for, so a four-frame clip was decoded to a twenty-fourth of
    what it could afford and previewed far below the output."""
    monkeypatch.setattr(server, "FULL_PREVIEW_PIXEL_BUDGET", 4096)
    monkeypatch.setattr(server, "ANIMATED_PREVIEW_TOTAL_BUDGET", 4096 * 4)
    monkeypatch.setattr(server, "ANIMATED_PREVIEW_MIN_BUDGET", 1)
    source = _fake_video(monkeypatch, tmp_path, count=4, width=64, height=64)

    rendered = _preview(SETTINGS, "full", 64, None, source, None)

    assert rendered.frame_count == 4
    assert (rendered.render_width, rendered.render_height) == (64, 64)
    assert (rendered.source_width, rendered.source_height) == (64, 64)
    preview_cache.CACHE.clear()


def test_a_long_clip_still_shares_the_budget_out(tmp_path, monkeypatch):
    monkeypatch.setattr(server, "FULL_PREVIEW_PIXEL_BUDGET", 4096)
    monkeypatch.setattr(server, "ANIMATED_PREVIEW_TOTAL_BUDGET", 4096 * 4)
    monkeypatch.setattr(server, "ANIMATED_PREVIEW_MIN_BUDGET", 1)
    source = _fake_video(monkeypatch, tmp_path, count=64, width=64, height=64)

    rendered = _preview(SETTINGS, "full", 64, None, source, None)

    assert rendered.render_width < 64
    preview_cache.CACHE.clear()
