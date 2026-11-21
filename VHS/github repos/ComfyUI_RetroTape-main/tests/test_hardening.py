"""The preview endpoint needs no authentication, so these pin its limits."""

from __future__ import annotations

import json
import sys
from pathlib import Path

import cv2
import numpy as np
import pytest
from PIL import Image

from ComfyUI_RetroTape import ntsc_core, preview_cache, server
from ComfyUI_RetroTape.nodes import NODE_CLASS_MAPPINGS, NODE_DISPLAY_NAME_MAPPINGS
from conftest import fake_folder_paths

# The controls for which zero is not "off", skipped when a test nulls them all.
_NEUTRAL_AT_THEIR_OWN_VALUE = ("blend", "seed", "saturation")




@pytest.mark.parametrize(
    "value",
    [float("nan"), float("inf"), float("-inf"), "abc", [1], {"a": 1}, None, 10**40],
)
def test_unusable_numbers_fall_back_instead_of_raising(value):
    assert server._coerce_int(value, 7, 1, 24) in range(1, 25)
    assert 0.0 <= server._coerce_float(value, 0.5, 0.0, 1.0) <= 1.0


def test_numbers_are_clamped_to_their_ceiling():
    assert server._coerce_int(10**9, 1, 1, server.MAX_SELECT_EVERY_NTH) == server.MAX_SELECT_EVERY_NTH
    assert server._coerce_int(-5, 1, 0, 10) == 0




def test_oversized_source_image_is_refused_before_it_is_decoded(tmp_path, monkeypatch):
    monkeypatch.setitem(sys.modules, "folder_paths", fake_folder_paths(tmp_path))
    monkeypatch.setattr(server, "MAX_SOURCE_PIXELS", 64)
    path = tmp_path / "big.png"
    Image.fromarray(np.zeros((16, 16, 3), np.uint8)).save(path)

    with pytest.raises(ValueError):
        server._load_source_image({"filename": "big.png"})

    monkeypatch.setattr(server, "MAX_SOURCE_PIXELS", 50_000_000)
    assert server._load_source_image({"filename": "big.png"}).frames.shape == (16, 16, 3)




def test_source_cache_is_bounded_by_bytes_not_only_by_item_count(monkeypatch):
    preview_cache.CACHE.clear()
    monkeypatch.setattr(preview_cache.CACHE, "max_bytes", 20_000)
    entry_bytes = 5 * 20 * 20 * 3
    for index in range(8):
        preview_cache.CACHE.put(("video", index), np.zeros((5, 20, 20, 3), np.uint8))
    items, total = len(preview_cache.CACHE), preview_cache.CACHE.total_bytes
    assert items < 8
    assert total <= 20_000
    assert items <= 20_000 // entry_bytes + 1
    preview_cache.CACHE.clear()


def test_node_frames_outrank_decoded_sources_in_the_shared_cache(monkeypatch):
    """A decoded file can be read again; node frames need a graph run, so they stay."""
    preview_cache.CACHE.clear()
    monkeypatch.setattr(preview_cache.CACHE, "max_bytes", 40_000)
    node_frames = np.zeros((4, 40, 40, 3), np.uint8)
    preview_cache.CACHE.put("node-a", node_frames, kind=preview_cache.NODE_KIND)
    preview_cache.CACHE.put("node-b", node_frames, kind=preview_cache.NODE_KIND)
    held = preview_cache.CACHE.total_bytes
    preview_cache.CACHE.put(("video", 1), node_frames, kind=preview_cache.SOURCE_KIND)
    assert preview_cache.CACHE.get("node-a") is not None
    assert preview_cache.CACHE.get("node-b") is not None
    assert preview_cache.CACHE.get(("video", 1)) is None
    assert preview_cache.CACHE.total_bytes == held

    preview_cache.CACHE.clear()
    preview_cache.CACHE.put(("video", 1), node_frames, kind=preview_cache.SOURCE_KIND)
    preview_cache.CACHE.put(("video", 2), node_frames, kind=preview_cache.SOURCE_KIND)
    preview_cache.CACHE.put("node-a", node_frames, kind=preview_cache.NODE_KIND)
    assert preview_cache.CACHE.get("node-a") is not None
    assert preview_cache.CACHE.get(("video", 1)) is None
    preview_cache.CACHE.clear()


def test_the_cache_hands_back_a_view_that_cannot_be_written_through():
    preview_cache.CACHE.clear()
    source = np.zeros((2, 8, 8, 3), np.uint8)
    stored = preview_cache.CACHE.put("view", source)
    with pytest.raises(ValueError):
        stored[0, 0, 0, 0] = 7
    assert preview_cache.CACHE.get("view").frames.flags.writeable is False
    preview_cache.CACHE.clear()


def test_clearing_only_the_sources_keeps_what_a_run_produced():
    preview_cache.CACHE.clear()
    preview_cache.set_preview_frames("held", np.zeros((2, 16, 16, 3), np.uint8))
    preview_cache.CACHE.put(("video", 1), np.zeros((2, 16, 16, 3), np.uint8), kind=preview_cache.SOURCE_KIND)

    report = preview_cache.CACHE.clear(preview_cache.SOURCE_KIND)

    assert report == {"freed_bytes": 2 * 16 * 16 * 3, "node_entries": 0, "source_entries": 1}
    assert preview_cache.get_preview_entry("held") is not None
    preview_cache.CACHE.clear()


def test_request_sequence_keys_cannot_grow_without_bound(monkeypatch):
    monkeypatch.setattr(server, "_MAX_SEQUENCE_KEYS", 4)
    server._REQUEST_SEQUENCE.clear()
    for index in range(50):
        server._next_sequence("node-%d" % index)
    assert len(server._REQUEST_SEQUENCE) <= 4
    server._REQUEST_SEQUENCE.clear()




def _write_video(path, frames=40, size=(64, 48)):
    writer = cv2.VideoWriter(str(path), cv2.VideoWriter_fourcc(*"mp4v"), 12, size)
    if not writer.isOpened():
        pytest.skip("No OpenCV video writer available.")
    for index in range(frames):
        frame = np.full((size[1], size[0], 3), index * 5 % 255, np.uint8)
        writer.write(frame)
    writer.release()
    return path


def test_a_huge_select_every_nth_cannot_walk_the_whole_file(tmp_path, monkeypatch):
    monkeypatch.setitem(sys.modules, "folder_paths", fake_folder_paths(tmp_path))
    _write_video(tmp_path / "clip.mp4", frames=40)
    reads = {"count": 0}

    real_capture = cv2.VideoCapture

    class CountingCapture:
        def __init__(self, path):
            self._capture = real_capture(path)

        def read(self):
            reads["count"] += 1
            return self._capture.read()

        def __getattr__(self, name):
            return getattr(self._capture, name)

    monkeypatch.setattr(server.cv2, "VideoCapture", CountingCapture)
    preview_cache.CACHE.clear()
    loaded = server._load_source_video(
        {"filename": "clip.mp4", "frame_load_cap": 24, "select_every_nth": 10**9}
    )
    assert loaded.frames.shape[0] >= 1
    assert reads["count"] <= server.MAX_SEQUENTIAL_VIDEO_READS
    preview_cache.CACHE.clear()


def test_a_lying_frame_count_cannot_be_turned_into_a_huge_allocation(tmp_path, monkeypatch):
    """CAP_PROP_FRAME_COUNT is whatever the container claims, including 2e9 or 9e15."""
    monkeypatch.setitem(sys.modules, "folder_paths", fake_folder_paths(tmp_path))
    _write_video(tmp_path / "clip.mp4", frames=8)
    real_capture = cv2.VideoCapture

    for claimed in (2e9, 9e15, float("inf"), float("nan")):

        class LyingCapture:
            def __init__(self, path):
                self._capture = real_capture(path)

            def get(self, prop):
                if prop == cv2.CAP_PROP_FRAME_COUNT:
                    return claimed
                return self._capture.get(prop)

            def __getattr__(self, name):
                return getattr(self._capture, name)

        monkeypatch.setattr(server.cv2, "VideoCapture", LyingCapture)
        preview_cache.CACHE.clear()
        loaded = server._load_source_video({"filename": "clip.mp4"})
        assert 1 <= loaded.frames.shape[0] <= server.MAX_PREVIEW_FRAMES
    preview_cache.CACHE.clear()


def test_the_decode_budget_leaves_the_cache_key_matching_what_it_names(tmp_path, monkeypatch):
    """decode_max_pixels used to vary the key while the loader overwrote the value."""
    monkeypatch.setitem(sys.modules, "folder_paths", fake_folder_paths(tmp_path))
    _write_video(tmp_path / "clip.mp4", frames=8)
    preview_cache.CACHE.clear()
    for pixels in (0, 10_000, 250_000, 3_000_000):
        server.render_preview(
            json.dumps({"preset": "Rental Copy"}),
            "full",
            360,
            None,
            {"filename": "clip.mp4", "decode_max_pixels": pixels},
            None,
        )
    assert len(preview_cache.CACHE) == 1
    preview_cache.CACHE.clear()




def test_a_refused_path_and_a_missing_file_are_indistinguishable(tmp_path, monkeypatch):
    monkeypatch.setitem(sys.modules, "folder_paths", fake_folder_paths(tmp_path))
    outside = tmp_path.parent / "outside.mp4"
    outside.write_bytes(b"not a video")

    with pytest.raises(server.PreviewSourceDenied) as refused:
        server.render_preview(
            None, "fast", 320, None, {"filename": str(outside), "path": True}, None
        )
    with pytest.raises(server.PreviewSourceDenied) as absent:
        server.render_preview(None, "fast", 320, None, None, {"filename": "nope.png"})

    assert str(refused.value) == str(absent.value) == server._SOURCE_UNAVAILABLE




def test_one_unusable_number_costs_only_its_own_control():
    settings = ntsc_core.parse_settings_json(
        '{"preset":"Damaged Tape","blend":0.5,"video_noise":NaN}'
    )
    assert settings["preset"] == "Damaged Tape"
    assert settings["blend"] == 0.5
    assert settings["video_noise"] == ntsc_core._compact_from_preset("Damaged Tape")["video_noise"]


def test_deeply_nested_json_does_not_fail_the_render():
    deep = "[" * 100_000 + "]" * 100_000
    assert ntsc_core.parse_settings_json(deep)["preset"] == ntsc_core.DEFAULT_PRESET




def test_uint8_conversion_rounds_rather_than_truncating():
    """Three float-to-uint8 conversions run in series, so truncation compounds."""
    rng = np.random.default_rng(0)
    source = rng.integers(0, 255, (64, 64, 3), dtype=np.uint8)
    settings = dict(ntsc_core.parse_settings_json(None))
    for key, value in list(settings.items()):
        if isinstance(value, (int, float)) and key not in _NEUTRAL_AT_THEIR_OWN_VALUE:
            settings[key] = 0
    settings["vhs_enabled"] = False
    settings["head_switching"] = False
    bias = (ntsc_core.process_rgb_uint8(source, settings).astype(int) - source.astype(int)).mean()
    assert abs(bias) < 0.2


def test_blend_mixes_between_the_source_and_the_processed_image():
    rng = np.random.default_rng(1)
    source = rng.integers(0, 255, (48, 64, 3), dtype=np.uint8)
    base = dict(ntsc_core.parse_settings_json(None))
    distance = {}
    for blend in (0.0, 0.5, 1.0):
        processed = ntsc_core.process_rgb_uint8(source, {**base, "blend": blend})
        distance[blend] = np.abs(processed.astype(int) - source.astype(int)).mean()
    assert distance[0.0] < 1.0
    assert distance[0.0] < distance[0.5] < distance[1.0]




def test_every_registered_node_has_a_display_name():
    assert set(NODE_DISPLAY_NAME_MAPPINGS) == set(NODE_CLASS_MAPPINGS)


def test_web_directory_points_at_the_panel_script():
    import ComfyUI_RetroTape as package

    root = Path(package.__file__).resolve().parent
    assert (root / package.WEB_DIRECTORY / "js" / "retrotape.js").is_file()




def test_frame_offset_makes_a_split_render_match_a_single_one():
    import torch

    frames = torch.rand(8, 24, 32, 3, generator=torch.Generator().manual_seed(0))
    whole = ntsc_core.process_tensor_batch(frames, None)
    split = torch.cat(
        [
            ntsc_core.process_tensor_batch(frames[:4], None, 0),
            ntsc_core.process_tensor_batch(frames[4:], None, 4),
        ]
    )
    assert torch.equal(split, whole)
    without_offset = ntsc_core.process_tensor_batch(frames[4:], None, 0)
    assert not torch.equal(without_offset, whole[4:])


def test_luma_smear_trails_to_the_right_and_only_to_the_right():
    frame = np.zeros((5, 201, 3), np.uint8)
    frame[:, 100] = 255
    settings = dict(ntsc_core.parse_settings_json(None))
    for key, value in list(settings.items()):
        if isinstance(value, (int, float)) and key not in _NEUTRAL_AT_THEIR_OWN_VALUE:
            settings[key] = 0
    settings["vhs_enabled"] = False
    settings["head_switching"] = False
    settings["luma_smear"] = 1.0
    row = ntsc_core.process_rgb_uint8(frame, settings)[2, :, 0].astype(int)
    assert row[90:100].max() == 0, "the smear must not reach backwards"
    assert row[101] > row[104] > 0, "the trail must decay away from the source"


@pytest.mark.parametrize("profile", ["_servo_profile", "_edge_wave_profile"])
def test_the_vertical_wobble_keeps_its_shape_at_any_resolution(profile):
    generator = getattr(ntsc_core, profile)

    def correlation_length(height):
        lags = []
        for seed in range(60):
            values = generator(seed, 0, height, 1.0, v_scale=height / 480.0)
            values = values - values.mean()
            correlation = np.correlate(values, values, "full")[len(values) - 1 :]
            correlation /= correlation[0]
            lags.append(int(np.argmax(correlation < 1 / np.e)))
        return float(np.mean(lags)) / (height / 480.0)

    reference = correlation_length(480)
    for height in (1080, 2160):
        assert abs(correlation_length(height) - reference) / reference < 0.25


def _panel_set(name):
    import re

    script = Path(__file__).resolve().parents[1] / "web" / "js" / "retrotape.js"
    match = re.search(r"const %s = new Set\(\[([^\]]*)\]\)" % name, script.read_text(encoding="utf-8"))
    assert match, "%s not found in the panel script" % name
    return set(re.findall(r'"(\w+)"', match.group(1)))


def _detailed_frame(width=96, height=80):
    """Not 72 rows: there the carrier advances whole cycles per row and
    scanline_phase 0 and 270 draw the same pattern."""
    rng = np.random.default_rng(3)
    return rng.integers(0, 255, (height, width, 3), dtype=np.uint8)


def test_every_slider_actually_changes_the_picture():
    """Every slider must move the picture, or be flagged frame-dependent in the panel."""
    ranges = ntsc_core.preset_catalog()["ranges"]
    assert len(ranges) > 20

    frame = _detailed_frame()
    base = dict(ntsc_core.parse_settings_json(None))
    sequence_only = _panel_set("SEQUENCE_ONLY")

    inert_on_a_still = []
    for key, (low, high) in ranges.items():
        if key == "seed":
            continue
        caster = type(base[key])
        quiet = ntsc_core.process_rgb_uint8(frame, {**base, key: caster(float(low))}, frame_index=0)
        loud = ntsc_core.process_rgb_uint8(frame, {**base, key: caster(float(high))}, frame_index=0)
        if np.array_equal(quiet, loud):
            inert_on_a_still.append(key)

    assert set(inert_on_a_still) == sequence_only, (
        "controls with no effect on a single image: %s; the panel flags: %s"
        % (sorted(inert_on_a_still), sorted(sequence_only))
    )

    for key in sequence_only:
        caster = type(base[key])
        low, high = ranges[key]
        quiet = ntsc_core.process_rgb_uint8(frame, {**base, key: caster(low)}, frame_index=1)
        loud = ntsc_core.process_rgb_uint8(frame, {**base, key: caster(high)}, frame_index=1)
        assert not np.array_equal(quiet, loud), "%s does nothing even across frames" % key


def test_the_panel_flags_every_control_the_vhs_deck_switches_off():
    frame = _detailed_frame()
    base = dict(ntsc_core.parse_settings_json(None), vhs_enabled=False)

    inert = []
    for key, (low, high) in ntsc_core.preset_catalog()["ranges"].items():
        if key == "seed":
            continue
        caster = type(base[key])
        quiet = ntsc_core.process_rgb_uint8(frame, {**base, key: caster(low)}, frame_index=1)
        loud = ntsc_core.process_rgb_uint8(frame, {**base, key: caster(high)}, frame_index=1)
        if np.array_equal(quiet, loud):
            inert.append(key)

    assert set(inert) == _panel_set("VHS_ONLY"), (
        "controls with no effect while the deck is off: %s; the panel flags: %s"
        % (sorted(inert), sorted(_panel_set("VHS_ONLY")))
    )
