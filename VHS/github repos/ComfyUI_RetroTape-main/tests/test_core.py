from __future__ import annotations

import json
import sys

import cv2
import numpy as np
import pytest
import torch
from PIL import Image

from ComfyUI_RetroTape import ntsc_core, preview_cache, server
from ComfyUI_RetroTape.nodes import RetroTape
from ComfyUI_RetroTape.preview_cache import CACHE, get_preview_entry
from conftest import fake_folder_paths


def _sample_batch(width=8, height=8, batch=1):
    x = torch.linspace(0, 1, width).view(1, 1, width, 1).repeat(batch, height, 1, 3)
    y = torch.linspace(0, 1, height).view(1, height, 1, 1).repeat(batch, 1, width, 3)
    return torch.clamp((x + y) * 0.5, 0, 1)


def _detail_image(width, height):
    x = np.linspace(0, 255, width, dtype=np.float32)[None, :]
    y = np.linspace(0, 255, height, dtype=np.float32)[:, None]
    frame = np.zeros((height, width, 3), dtype=np.uint8)
    frame[..., 0] = np.clip(x * 0.6 + y * 0.4, 0, 255)
    frame[..., 1] = np.clip(255 - y * 0.8, 0, 255)
    frame[..., 2] = np.clip(x * 0.3 + 60, 0, 255)
    for column in range(0, width, max(1, width // 12)):
        frame[:, column : column + max(1, width // 48)] = 245
    for row in range(0, height, max(1, height // 8)):
        frame[row : row + max(1, height // 60), :] = 15
    return frame


def test_processing_is_deterministic_for_same_seed():
    image = _sample_batch()
    settings = json.dumps({**ntsc_core.DEFAULT_SETTINGS, "seed": 123})
    first = ntsc_core.process_tensor_batch(image, settings)
    second = ntsc_core.process_tensor_batch(image, settings)
    assert torch.equal(first, second)


def test_node_processes_full_batch():
    image = _sample_batch(width=8, height=8, batch=3)
    node = RetroTape()
    output = node.apply(image, ntsc_core.DEFAULT_SETTINGS_JSON, unique_id="batch")
    (result,) = output["result"]
    assert result.shape == image.shape
    assert output["ui"]["retrotape_preview_ready"] == ["batch", "3"]
    assert get_preview_entry("batch").frames.shape == (3, 8, 8, 3)


def test_node_is_output_node_for_standalone_preview_runs():
    assert RetroTape.OUTPUT_NODE is True


def test_non_multiple_of_four_width_preserves_dimensions():
    image = _sample_batch(width=7, height=9, batch=1)
    result = ntsc_core.process_tensor_batch(image, ntsc_core.DEFAULT_SETTINGS_JSON)
    assert result.shape == image.shape


def test_invalid_settings_fall_back_to_default_preset():
    settings = ntsc_core.parse_settings_json("{")
    assert settings["preset"] == ntsc_core.DEFAULT_PRESET


def test_custom_preset_and_expanded_controls_are_accepted():
    settings = ntsc_core.parse_settings_json({"preset": "Damaged Tape", "video_noise": 9000})
    assert settings["preset"] == "Damaged Tape"
    assert settings["video_noise"] == 9000
    assert settings["tracking_jitter"] > 0
    assert settings["dropouts"] > 0
    assert settings["color_drift"] > 0
    assert settings["analog_glow"] > 0


@pytest.mark.parametrize(
    "payload",
    [
        '{"ringing":NaN}',
        '{"video_noise":Infinity}',
        '{"video_noise":-Infinity}',
        '{"luma_smear":NaN}',
        '{"analog_glow":NaN}',
        '{"preset":{"a":1}}',
        '{"preset":[1]}',
        '{"preset":123}',
        '{"video_noise":1e400}',
        '{"seed":Infinity}',
        "[1,2,3]",
        "null",
    ],
)
def test_hostile_settings_never_raise_and_stay_in_range(payload):
    settings = ntsc_core.parse_settings_json(payload)
    assert settings["preset"] in ntsc_core.PRESETS or settings["preset"] in ntsc_core.CUSTOM_PRESETS
    for key, (low, high) in ntsc_core._FLOAT_RANGES.items():
        value = settings[key]
        assert np.isfinite(value) and low <= value <= high, key
    for key, (low, high) in ntsc_core._INT_RANGES.items():
        value = settings[key]
        assert isinstance(value, int) and low <= value <= high, key

    frame = _detail_image(24, 16)
    out = ntsc_core.process_rgb_uint8(frame, payload)
    assert out.shape == frame.shape
    # A rejected value must not collapse the picture to black.
    assert int(out.max()) > 0


def test_saturation_and_black_lift_default_to_doing_nothing():
    settings = ntsc_core.parse_settings_json("{}")
    assert settings["saturation"] == 1.0
    assert settings["black_lift"] == 0.0


def test_a_settings_json_written_before_these_controls_existed_renders_the_same():
    legacy = {key: value for key, value in ntsc_core.DEFAULT_SETTINGS.items()
              if key not in ("saturation", "black_lift")}
    parsed = ntsc_core.parse_settings_json(json.dumps(legacy))
    assert parsed["saturation"] == 1.0
    assert parsed["black_lift"] == 0.0

    frame = _detail_image(96, 72)
    without = ntsc_core.process_rgb_uint8(frame, json.dumps(legacy))
    withkeys = ntsc_core.process_rgb_uint8(
        frame, json.dumps({**legacy, "saturation": 1.0, "black_lift": 0.0})
    )
    assert np.array_equal(without, withkeys)


@pytest.mark.parametrize("name", list(ntsc_core.PRESETS) + list(ntsc_core.CUSTOM_PRESETS))
def test_the_neutral_values_are_a_no_op_for_every_preset(name, monkeypatch):
    """Removing the stages outright has to change nothing at their neutral value."""
    frame = _detail_image(64, 48)
    settings = {**ntsc_core._compact_from_preset(name), "saturation": 1.0, "black_lift": 0.0}
    reference = ntsc_core.process_rgb_uint8(frame, settings)
    monkeypatch.setattr(ntsc_core, "_apply_saturation", lambda i, q, _settings: (i, q))
    monkeypatch.setattr(ntsc_core, "_apply_black_lift", lambda y, _settings: y)
    assert np.array_equal(ntsc_core.process_rgb_uint8(frame, settings), reference)


def _null_settings():
    """Every stage off, so one control at a time can be read on its own."""
    settings = dict(ntsc_core.parse_settings_json(None))
    for key, value in list(settings.items()):
        if isinstance(value, (int, float)) and key not in ("blend", "seed", "saturation"):
            settings[key] = 0
    settings["vhs_enabled"] = False
    settings["head_switching"] = False
    return settings


def _colour_spread(image):
    array = image.astype(np.float32)
    return float((array.max(axis=-1) - array.min(axis=-1)).mean())


def test_saturation_scales_the_colour_in_the_picture():
    frame = _detail_image(96, 72)
    base = _null_settings()

    def colourfulness(gain):
        return _colour_spread(ntsc_core.process_rgb_uint8(frame, {**base, "saturation": gain}))

    assert colourfulness(0.0) < 1.0, "at zero the picture has to be monochrome"
    assert colourfulness(0.0) < colourfulness(0.5) < colourfulness(1.0) < colourfulness(2.0)


def test_the_deck_still_adds_colour_noise_to_a_monochrome_picture():
    frame = _detail_image(96, 72)
    base = {**_null_settings(), "saturation": 0.0}
    quiet = ntsc_core.process_rgb_uint8(frame, {**base, "chroma_noise": 0})
    snowy = ntsc_core.process_rgb_uint8(frame, {**base, "chroma_noise": 20000})
    assert _colour_spread(quiet) < 1.0
    assert _colour_spread(snowy) > 8.0


def test_black_lift_raises_the_floor_and_leaves_white_where_it_is():
    frame = np.zeros((32, 64, 3), np.uint8)
    frame[:, 32:] = 255
    base = _null_settings()

    floor = []
    for amount in (0.0, 0.5, 1.0):
        out = ntsc_core.process_rgb_uint8(frame, {**base, "black_lift": amount}).astype(int)
        floor.append(out[:, :24].mean())
        assert out[:, 40:].mean() > 250, "white must not move"
    assert floor[0] < 1.0
    assert floor[0] < floor[1] < floor[2]
    assert abs(floor[2] - ntsc_core.BLACK_LIFT_PEDESTAL) < 1.5


def test_unusable_value_falls_back_to_the_preset_value_not_the_minimum():
    settings = ntsc_core.parse_settings_json('{"preset":"RGM","video_noise":null}')
    assert settings["video_noise"] == ntsc_core._compact_from_preset("RGM")["video_noise"]
    assert settings["video_noise"] > 0


def test_effect_is_resolution_independent():
    settings = json.dumps(
        {
            **ntsc_core.DEFAULT_SETTINGS,
            "video_noise": 0,
            "chroma_noise": 0,
            "chroma_phase_noise": 0,
            "chroma_loss": 0,
            "dropouts": 0.0,
            "tracking_jitter": 0.0,
            "vhs_edge_wave": 0,
            "head_switching": False,
            "seed": 5,
        }
    )
    big = _detail_image(1440, 960)
    small = cv2.resize(big, (720, 480), interpolation=cv2.INTER_AREA)

    full = ntsc_core.process_rgb_uint8(big, settings)
    full_down = cv2.resize(full, (720, 480), interpolation=cv2.INTER_AREA).astype(np.float32)
    preview = ntsc_core.process_rgb_uint8(small, settings).astype(np.float32)

    effect = float(np.mean(np.abs(preview - small.astype(np.float32))))
    mismatch = float(np.mean(np.abs(full_down - preview)))
    assert effect > 5.0, "the test settings should still produce a visible effect"
    assert mismatch < effect * 0.25, f"preview drifted from the full render: {mismatch:.2f}"


def test_slow_artefacts_persist_across_frames():
    settings = json.dumps(
        {
            **ntsc_core.DEFAULT_SETTINGS,
            "video_noise": 0,
            "chroma_noise": 0,
            "chroma_phase_noise": 0,
            "chroma_loss": 0,
            "dropouts": 0.3,
            "tracking_jitter": 5.0,
            "seed": 5,
        }
    )
    frame = _detail_image(240, 160)
    rendered = [
        ntsc_core.process_rgb_uint8(frame, settings, frame_index=index).astype(np.float32)
        for index in range(ntsc_core.TEMPORAL_EPOCH_FRAMES)
    ]
    within_epoch = [
        float(np.mean(np.abs(rendered[index] - rendered[index - 1])))
        for index in range(1, len(rendered))
    ]
    assert max(within_epoch) < 12.0, f"frames flicker within one epoch: {within_epoch}"


def test_alpha_is_preserved_and_single_channel_is_expanded():
    rgba = torch.rand(2, 8, 8, 4)
    out = ntsc_core.process_tensor_batch(rgba, ntsc_core.DEFAULT_SETTINGS_JSON)
    assert out.shape == (2, 8, 8, 4)
    assert torch.allclose(out[..., 3], rgba[..., 3])

    grey = torch.rand(1, 8, 8, 1)
    assert ntsc_core.process_tensor_batch(grey, ntsc_core.DEFAULT_SETTINGS_JSON).shape == (1, 8, 8, 3)


@pytest.mark.parametrize("dtype", [torch.float16, torch.bfloat16, torch.float64])
def test_unusual_dtypes_are_accepted(dtype):
    image = torch.rand(1, 8, 8, 3).to(dtype)
    out = ntsc_core.process_tensor_batch(image, ntsc_core.DEFAULT_SETTINGS_JSON)
    assert out.dtype == torch.float32
    assert out.shape == (1, 8, 8, 3)


@pytest.mark.parametrize("shape", [(1, 0, 16, 3), (1, 16, 0, 3), (1, 0, 0, 3), (2, 0, 8, 4)])
def test_a_zero_sized_image_comes_back_untouched_on_both_backends(shape):
    """The CPU path raised while the GPU path returned it, so the fallback changed behaviour."""
    from ComfyUI_RetroTape import ntsc_gpu

    images = torch.zeros(shape)
    assert torch.equal(ntsc_core.process_tensor_batch(images, ntsc_core.DEFAULT_SETTINGS_JSON), images)
    frame = np.zeros(shape[1:], np.uint8)[..., :3]
    assert ntsc_core.process_rgb_uint8(frame, ntsc_core.DEFAULT_SETTINGS_JSON).shape == frame.shape
    if not ntsc_gpu.is_available():
        return
    on_gpu = ntsc_gpu.process_tensor_batch(images.cuda(), ntsc_core.DEFAULT_SETTINGS_JSON)
    assert torch.equal(on_gpu.cpu(), images)
    assert ntsc_gpu.process_rgb_uint8(frame, ntsc_core.DEFAULT_SETTINGS_JSON).shape == frame.shape


def test_empty_batch_round_trips():
    empty = torch.rand(0, 8, 8, 3)
    assert ntsc_core.process_tensor_batch(empty, ntsc_core.DEFAULT_SETTINGS_JSON).shape == (0, 8, 8, 3)


def test_preview_cache_refuses_unknown_node():
    CACHE.clear()
    assert get_preview_entry("missing") is None
    preview_cache.set_preview_frames("known", np.zeros((4, 4, 3), dtype=np.uint8))
    assert get_preview_entry("known").frames.shape == (1, 4, 4, 3)


def test_preview_cache_copies_uint8_input():
    CACHE.clear()
    frames = np.zeros((2, 4, 4, 3), dtype=np.uint8)
    preview_cache.set_preview_frames("owned", frames)
    frames[:] = 99
    assert int(get_preview_entry("owned").frames.max()) == 0


def test_preview_cache_is_bounded_by_bytes(monkeypatch):
    CACHE.clear()
    monkeypatch.setattr(preview_cache.CACHE, "max_bytes", 8 * 1024)
    for index in range(12):
        preview_cache.set_preview_frames(f"node{index}", np.zeros((4, 32, 32, 3), dtype=np.uint8))
    items, total = len(preview_cache.CACHE), preview_cache.CACHE.total_bytes
    assert total <= preview_cache.CACHE.max_bytes or items == 1
    assert items < 12


def test_preview_cache_downscales_before_storing():
    CACHE.clear()
    huge = np.zeros((1, 2000, 3000, 3), dtype=np.uint8)
    preview_cache.set_preview_frames("huge", huge)
    stored = get_preview_entry("huge").frames
    assert max(stored.shape[1:3]) == preview_cache.MAX_CACHED_SIDE


def test_animated_preview_is_returned_for_frame_batches():
    frames = np.zeros((3, 6, 6, 3), dtype=np.uint8)
    frames[1, :, :, 1] = 128
    frames[2, :, :, 2] = 255
    rendered = server._encode_preview(frames, ntsc_core.DEFAULT_SETTINGS_JSON)
    assert rendered.frame_count == 3
    assert rendered.content_type == "image/webp"
    assert len(rendered.body) > 0


def test_fast_preview_returns_single_downscaled_frame():
    frames = np.zeros((4, 32, 64, 3), dtype=np.uint8)
    rendered = server._encode_preview(
        frames,
        ntsc_core.DEFAULT_SETTINGS_JSON,
        preview_mode="fast",
        max_side=16,
    )
    assert rendered.frame_count == 1
    assert rendered.content_type == "image/png"
    assert len(rendered.body) > 0


def test_empty_frame_batch_is_rejected():
    with pytest.raises(ValueError):
        server._encode_preview(np.zeros((0, 4, 4, 3), dtype=np.uint8), ntsc_core.DEFAULT_SETTINGS_JSON)


def test_absolute_path_outside_allowed_roots_is_refused(tmp_path, monkeypatch):
    allowed = tmp_path / "input"
    allowed.mkdir()
    outside = tmp_path / "secret.avi"
    outside.write_bytes(b"stub")
    monkeypatch.setitem(sys.modules, "folder_paths", fake_folder_paths(allowed))
    monkeypatch.delenv(server.EXTRA_ROOTS_ENV, raising=False)

    with pytest.raises(server.PreviewSourceDenied):
        server._resolve_source_path({"filename": str(outside), "path": True})


def test_extra_roots_environment_variable_opts_a_folder_in(tmp_path, monkeypatch):
    allowed = tmp_path / "input"
    allowed.mkdir()
    elsewhere = tmp_path / "media"
    elsewhere.mkdir()
    clip = elsewhere / "clip.avi"
    clip.write_bytes(b"stub")
    monkeypatch.setitem(sys.modules, "folder_paths", fake_folder_paths(allowed))
    monkeypatch.setenv(server.EXTRA_ROOTS_ENV, str(elsewhere))

    resolved = server._resolve_source_path({"filename": str(clip), "path": True})
    assert resolved == str(clip.resolve())


def test_traversal_out_of_the_input_folder_is_refused(tmp_path, monkeypatch):
    allowed = tmp_path / "input"
    allowed.mkdir()
    (tmp_path / "outside.png").write_bytes(b"stub")
    monkeypatch.setitem(sys.modules, "folder_paths", fake_folder_paths(allowed))
    monkeypatch.delenv(server.EXTRA_ROOTS_ENV, raising=False)

    with pytest.raises(server.PreviewSourceDenied):
        server._load_source_image({"filename": "../outside.png", "type": "input"})


def test_source_video_can_load_multiple_preview_frames(tmp_path, monkeypatch):
    preview_cache.CACHE.clear()

    allowed = tmp_path / "input"
    allowed.mkdir()
    monkeypatch.setitem(sys.modules, "folder_paths", fake_folder_paths(allowed))
    monkeypatch.delenv(server.EXTRA_ROOTS_ENV, raising=False)

    video_path = allowed / "source.avi"
    video_path.write_bytes(b"stub")
    source_frames = []
    for index in range(5):
        frame = np.zeros((4, 4, 3), dtype=np.uint8)
        frame[:, :, 2] = index * 40
        source_frames.append(frame)

    open_count = 0

    class FakeCapture:
        def __init__(self, path):
            nonlocal open_count
            open_count += 1
            self.path = path
            self.position = 0

        def isOpened(self):
            return True

        def get(self, prop):
            if prop == server.cv2.CAP_PROP_FRAME_COUNT:
                return len(source_frames)
            if prop == server.cv2.CAP_PROP_POS_FRAMES:
                return self.position
            return 0

        def set(self, prop, value):
            if prop == server.cv2.CAP_PROP_POS_FRAMES:
                self.position = int(value)

        def read(self):
            if self.position >= len(source_frames):
                return False, None
            frame = source_frames[self.position]
            self.position += 1
            return True, frame

        def release(self):
            pass

    monkeypatch.setattr(server.cv2, "VideoCapture", FakeCapture)

    request = {"filename": str(video_path), "path": True, "frame_load_cap": 3, "max_frames": 3}
    loaded = server._load_source_video(dict(request))
    cached = server._load_source_video(dict(request))

    assert loaded.frames.shape == (3, 4, 4, 3)
    assert cached.frames.shape == (3, 4, 4, 3)
    assert open_count == 1
    # A cached preview must not render another part of the clip's artefacts.
    assert loaded.indices.tolist() == [0, 1, 2]
    assert cached.indices.tolist() == [0, 1, 2]
    assert loaded.source_size == cached.source_size == (4, 4)


def test_source_image_can_load_from_comfyui_input_folder(tmp_path, monkeypatch):
    image_path = tmp_path / "source.png"
    Image.new("RGB", (3, 2), (20, 40, 60)).save(image_path)

    monkeypatch.setitem(sys.modules, "folder_paths", fake_folder_paths(tmp_path))

    loaded = server._load_source_image({"filename": "source.png", "type": "input"})
    assert loaded.frames.shape == (2, 3, 3)
    assert loaded.frames[0, 0].tolist() == [20, 40, 60]
    assert loaded.source_size == (3, 2)


def test_render_preview_reports_a_generic_message_for_a_missing_source(tmp_path, monkeypatch):
    CACHE.clear()
    allowed = tmp_path / "input"
    allowed.mkdir()
    monkeypatch.setitem(sys.modules, "folder_paths", fake_folder_paths(allowed))
    monkeypatch.delenv(server.EXTRA_ROOTS_ENV, raising=False)

    with pytest.raises(server.PreviewSourceDenied) as excinfo:
        server.render_preview(
            ntsc_core.DEFAULT_SETTINGS_JSON,
            "fast",
            320,
            "no-such-node",
            {"filename": "missing.avi", "type": "input"},
            None,
        )
    assert str(tmp_path) not in str(excinfo.value)
