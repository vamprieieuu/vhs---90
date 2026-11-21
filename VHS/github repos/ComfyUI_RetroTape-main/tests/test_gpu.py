"""The GPU path is held to the picture ntsc_core would have produced, stage by
stage where the stage is deterministic and statistically where it cannot be."""

from __future__ import annotations

import json

import cv2
import numpy as np
import pytest
import torch

from ComfyUI_RetroTape import ntsc_core, ntsc_gpu

pytestmark = pytest.mark.skipif(not ntsc_gpu.is_available(), reason="no CUDA device")

ALL_PRESETS = list(ntsc_core.PRESETS) + list(ntsc_core.CUSTOM_PRESETS)

RANDOM_STAGES = ("video_noise", "chroma_noise", "chroma_phase_noise", "chroma_loss", "dropouts")


def _detail_image(width, height, seed=7):
    """Detail at every spatial frequency, so a stage differing only in the fine structure shows up."""
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
    for row in range(0, height, max(1, height // 10)):
        frame[row : row + max(1, height // 80), :] = 12
    return np.clip(frame, 0, 255).astype(np.uint8)


def _deterministic(preset):
    """The only configuration in which the two paths can be compared pixel for pixel."""
    settings = ntsc_core._compact_from_preset(preset)
    for key in RANDOM_STAGES:
        settings[key] = 0
    return settings


def _delta(first, second):
    difference = np.abs(first.astype(np.int32) - second.astype(np.int32))
    return float(difference.mean()), int(difference.max())




def test_the_gpu_render_is_bit_identical_run_to_run():
    frame = _detail_image(384, 288)
    settings = {"preset": "Damaged Tape", "seed": 12345}
    first = ntsc_gpu.process_rgb_uint8(frame, settings, frame_index=5)
    second = ntsc_gpu.process_rgb_uint8(frame, settings, frame_index=5)
    assert np.array_equal(first, second)


def test_a_batch_renders_the_same_bits_as_the_single_frame_call():
    frame = _detail_image(192, 128)
    settings = {"preset": "Rental Copy", "seed": 99}
    tensor = torch.from_numpy(frame.astype(np.float32) / 255.0)[None]
    batched = ntsc_gpu.process_tensor_batch(tensor, settings, 4)
    single = ntsc_gpu.process_rgb_uint8(frame, settings, frame_index=4)
    assert np.array_equal((batched[0].numpy() * 255.0).round().astype(np.uint8), single)


def test_the_seed_still_changes_the_picture():
    frame = _detail_image(160, 120)
    one = ntsc_gpu.process_rgb_uint8(frame, {"preset": "Rental Copy", "seed": 1})
    two = ntsc_gpu.process_rgb_uint8(frame, {"preset": "Rental Copy", "seed": 2})
    assert not np.array_equal(one, two)




@pytest.mark.parametrize("preset", ALL_PRESETS)
@pytest.mark.parametrize("side", [512, 1600])
def test_the_deterministic_stages_match_the_cpu_path(preset, side):
    frame = _detail_image(side, side)
    settings = _deterministic(preset)
    cpu = ntsc_core.process_rgb_uint8(frame, dict(settings), frame_index=3)
    gpu = ntsc_gpu.process_rgb_uint8(frame, dict(settings), frame_index=3)
    mean, largest = _delta(cpu, gpu)
    assert mean < 0.01, f"{preset} @{side}: mean |delta| {mean:.4f}"
    assert largest < 4, f"{preset} @{side}: max |delta| {largest}"


def test_the_deterministic_match_holds_on_an_odd_width():
    """A width that is not a multiple of four goes through the reflected pad."""
    frame = _detail_image(501, 331)
    settings = _deterministic("Home Movie 1987")
    cpu = ntsc_core.process_rgb_uint8(frame, dict(settings), frame_index=1)
    gpu = ntsc_gpu.process_rgb_uint8(frame, dict(settings), frame_index=1)
    assert gpu.shape == frame.shape
    mean, largest = _delta(cpu, gpu)
    assert mean < 0.01 and largest < 4, f"mean {mean:.4f} max {largest}"


def test_the_deterministic_match_holds_under_a_partial_blend():
    frame = _detail_image(256, 256)
    settings = _deterministic("Bootleg Anime")
    settings["blend"] = 0.37
    cpu = ntsc_core.process_rgb_uint8(frame, dict(settings), frame_index=2)
    gpu = ntsc_gpu.process_rgb_uint8(frame, dict(settings), frame_index=2)
    mean, largest = _delta(cpu, gpu)
    assert mean < 0.01 and largest < 4, f"mean {mean:.4f} max {largest}"


def test_the_edge_wave_is_not_drawn_when_the_deck_is_off():
    settings = _deterministic("Rental Copy")
    settings["_h_scale"] = settings["_v_scale"] = 1.0
    assert settings["vhs_edge_wave"] > 0
    settings["vhs_enabled"] = False
    assert ntsc_gpu._edge_wave_shifts(settings, 3, 480) is None

    frame = _detail_image(320, 240)
    cpu = ntsc_core.process_rgb_uint8(frame, dict(settings), frame_index=3)
    gpu = ntsc_gpu.process_rgb_uint8(frame, dict(settings), frame_index=3)
    mean, largest = _delta(cpu, gpu)
    assert mean < 0.01 and largest < 4, f"deck off: mean {mean:.4f} max {largest}"


def test_the_head_switch_band_matches_the_cpu_path_exactly():
    # The band's noise is the Generator's last draw, so both paths match with noise off.
    frame = _detail_image(320, 240)
    settings = _deterministic("RGM")
    settings["head_switching"] = True
    cpu = ntsc_core.process_rgb_uint8(frame, dict(settings), frame_index=6)
    gpu = ntsc_gpu.process_rgb_uint8(frame, dict(settings), frame_index=6)
    band = max(2, int(240 * 0.045))
    mean, largest = _delta(cpu[-band:], gpu[-band:])
    assert mean < 0.01 and largest < 4, f"head switch band: mean {mean:.4f} max {largest}"


def test_the_tape_damage_lands_in_the_same_places_as_on_the_cpu():
    """Dropout bands and tracking tears are geometry, not grain, so they must sit in the same rows."""
    frame = np.full((240, 320, 3), 128, np.uint8)
    settings = ntsc_core._compact_from_preset("Damaged Tape")
    settings["video_noise"] = 0
    settings["chroma_noise"] = 0
    settings["chroma_phase_noise"] = 0
    settings["chroma_loss"] = 0
    cpu = ntsc_core.process_rgb_uint8(frame, dict(settings), frame_index=2).astype(np.float32)
    gpu = ntsc_gpu.process_rgb_uint8(frame, dict(settings), frame_index=2).astype(np.float32)
    cpu_rows = np.abs(cpu - 128.0).mean(axis=(1, 2)) > 12.0
    gpu_rows = np.abs(gpu - 128.0).mean(axis=(1, 2)) > 12.0
    assert cpu_rows.any(), "the test settings should produce visible damage"
    agreement = float((cpu_rows == gpu_rows).mean())
    assert agreement > 0.97, f"damaged rows disagree: {agreement:.3f}"




@pytest.mark.parametrize("preset", ["RGM", "Broadcast Dirty", "Damaged Tape", "Home Movie 1987"])
def test_noise_statistics_match_the_cpu_path(preset):
    frame = _detail_image(768, 768)
    settings = ntsc_core._compact_from_preset(preset)
    clean = ntsc_core.process_rgb_uint8(frame, _deterministic(preset), frame_index=1).astype(np.float32)
    cpu = ntsc_core.process_rgb_uint8(frame, dict(settings), frame_index=1).astype(np.float32) - clean
    gpu = ntsc_gpu.process_rgb_uint8(frame, dict(settings), frame_index=1).astype(np.float32) - clean

    assert cpu.std() > 1.0, "the test preset should add measurable grain"
    ratio = gpu.std() / cpu.std()
    assert 0.9 < ratio < 1.1, f"{preset}: grain std ratio {ratio:.4f}"
    assert abs(gpu.mean() - cpu.mean()) < max(0.5, 0.1 * abs(cpu.mean())), (
        f"{preset}: grain mean {gpu.mean():.3f} vs {cpu.mean():.3f}"
    )


@pytest.mark.parametrize("control,sigma", [("video_noise", 42.0 / 9000.0), ("chroma_noise", 82.0 / 32768.0)])
def test_a_single_noise_control_adds_the_amplitude_the_cpu_path_adds(control, sigma):
    frame = np.full((512, 512, 3), 120, np.uint8)
    base = {key: 0 for key in RANDOM_STAGES}
    base.update(
        {
            "preset": "First Gen",
            "blend": 1.0,
            "seed": 3,
            "ringing": 0.0,
            "ghosting": 0.0,
            "scanlines": 0.0,
            "analog_glow": 0.0,
            "luma_smear": 0.0,
            "interlace": 0.0,
            "tracking_jitter": 0.0,
            "tape_warp": 0.0,
            "vhs_edge_wave": 0,
            "head_switching": False,
            "vhs_enabled": False,
        }
    )
    quiet = ntsc_core.parse_settings_json(base)
    loud = dict(quiet)
    loud[control] = 4000
    flat = ntsc_core.process_rgb_uint8(frame, dict(quiet)).astype(np.float32)
    cpu = ntsc_core.process_rgb_uint8(frame, dict(loud)).astype(np.float32) - flat
    gpu = ntsc_gpu.process_rgb_uint8(frame, dict(loud)).astype(np.float32) - flat
    ratio = gpu.std() / cpu.std()
    assert 0.9 < ratio < 1.1, f"{control}: std ratio {ratio:.4f} (cpu {cpu.std():.3f} gpu {gpu.std():.3f})"




def test_the_gpu_effect_is_resolution_independent():
    """A downscaled render must look like a downscale of the full render."""
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

    full = ntsc_gpu.process_rgb_uint8(big, settings)
    full_down = cv2.resize(full, (720, 480), interpolation=cv2.INTER_AREA).astype(np.float32)
    preview = ntsc_gpu.process_rgb_uint8(small, settings).astype(np.float32)

    effect = float(np.mean(np.abs(preview - small.astype(np.float32))))
    mismatch = float(np.mean(np.abs(full_down - preview)))
    assert effect > 5.0, "the test settings should still produce a visible effect"
    assert mismatch < effect * 0.25, f"preview drifted from the full render: {mismatch:.2f}"


def test_the_interlace_comb_is_a_line_pitch_not_a_pixel_pitch():
    """The comb belongs to a 480-line picture, so it must survive a resolution change."""
    settings = {
        **ntsc_core.DEFAULT_SETTINGS,
        "video_noise": 0,
        "chroma_noise": 0,
        "chroma_phase_noise": 0,
        "chroma_loss": 0,
        "dropouts": 0.0,
        "interlace": 1.0,
        "scanlines": 0.0,
        "tracking_jitter": 0.0,
        "vhs_edge_wave": 0,
        "head_switching": False,
    }

    def strength(height):
        frame = np.full((height, 320, 3), 200, np.uint8)
        out = ntsc_gpu.process_rgb_uint8(frame, settings).astype(np.float32).mean(axis=(1, 2))
        lines = ntsc_core._reference_lines(height, {"_v_scale": max(0.05, height / 480.0)})
        return abs(float(out[lines % 2 == 1].mean() - out[lines % 2 == 0].mean()))

    at_480 = strength(480)
    at_1440 = strength(1440)
    assert at_480 > 1.0
    assert at_1440 > at_480 * 0.5, f"the comb faded with resolution: {at_480:.2f} -> {at_1440:.2f}"




@pytest.mark.parametrize("width,height", [(1, 1), (7, 5), (720, 480), (1920, 1080), (3840, 2160)])
def test_every_preset_renders_at_every_shape(width, height):
    frame = _detail_image(width, height)
    for preset in ALL_PRESETS:
        out = ntsc_gpu.process_rgb_uint8(frame, {"preset": preset}, frame_index=2)
        assert out.shape == (height, width, 3)
        assert out.dtype == np.uint8


@pytest.mark.parametrize("width,height", [(1, 1), (7, 5), (13, 480)])
def test_a_tiny_frame_still_matches_the_cpu_path(width, height):
    frame = _detail_image(width, height)
    for preset in ("RGM", "Damaged Tape", "Kinescope"):
        settings = _deterministic(preset)
        cpu = ntsc_core.process_rgb_uint8(frame, dict(settings), frame_index=1)
        gpu = ntsc_gpu.process_rgb_uint8(frame, dict(settings), frame_index=1)
        mean, largest = _delta(cpu, gpu)
        assert mean < 0.01 and largest < 4, f"{preset} {width}x{height}: mean {mean:.3f} max {largest}"


def test_an_empty_batch_round_trips():
    empty = torch.zeros((0, 8, 8, 3))
    assert ntsc_gpu.process_tensor_batch(empty, ntsc_core.DEFAULT_SETTINGS_JSON).shape == (0, 8, 8, 3)


@pytest.mark.parametrize("shape", [(1, 8, 0, 3), (1, 0, 8, 3)])
def test_a_zero_sized_frame_leaves_the_cuda_context_usable(shape):
    # An empty axis used to trip a device-side assert, poisoning the CUDA context.
    assert ntsc_gpu.process_tensor_batch(torch.zeros(shape), ntsc_core.DEFAULT_SETTINGS_JSON).shape == shape
    assert ntsc_gpu.process_rgb_uint8(np.zeros(shape[1:], np.uint8), ntsc_core.DEFAULT_SETTINGS_JSON).shape == shape[1:]
    assert float((torch.ones(8, device="cuda") * 2.0).sum()) == 16.0




def test_a_split_render_equals_a_whole_one():
    """A clip rendered in several runs has to be the clip rendered in one."""
    frames = torch.from_numpy(
        np.stack([_detail_image(160, 128, seed=index) for index in range(9)]).astype(np.float32) / 255.0
    )
    settings = {"preset": "Tracking Hell", "seed": 21}
    whole = ntsc_gpu.process_tensor_batch(frames, settings, 0)
    split = torch.cat(
        [
            ntsc_gpu.process_tensor_batch(frames[:4], settings, 0),
            ntsc_gpu.process_tensor_batch(frames[4:], settings, 4),
        ],
        dim=0,
    )
    assert torch.equal(whole, split)


def test_slow_artefacts_persist_across_an_epoch_and_change_between_them():
    settings = {
        **ntsc_core.DEFAULT_SETTINGS,
        "video_noise": 0,
        "chroma_noise": 0,
        "chroma_phase_noise": 0,
        "chroma_loss": 0,
        "dropouts": 0.3,
        "tracking_jitter": 5.0,
        "tape_warp": 0.0,
        "seed": 5,
    }
    frame = _detail_image(240, 160)
    span = ntsc_core.TEMPORAL_EPOCH_FRAMES
    rendered = [
        ntsc_gpu.process_rgb_uint8(frame, settings, frame_index=index).astype(np.float32)
        for index in range(span * 2)
    ]
    within = [float(np.mean(np.abs(rendered[k] - rendered[k - 1]))) for k in range(1, span)]
    assert max(within) < 12.0, f"frames flicker within one epoch: {within}"
    across = float(np.mean(np.abs(rendered[span] - rendered[span - 1])))
    assert across > 0.0


def test_the_epoch_geometry_tracks_the_cpu_path_frame_for_frame():
    """The interpolated servo profile and edge wave must give the same per-row integers."""
    settings = ntsc_core.parse_settings_json({"preset": "Tracking Hell", "seed": 8})
    settings["_h_scale"], settings["_v_scale"] = ntsc_core._frame_scale((480, 720))
    for frame_index in range(0, 10):
        gpu_edge = ntsc_gpu._edge_wave_shifts(settings, frame_index, 480)
        span = ntsc_core.TEMPORAL_EPOCH_FRAMES
        epoch, mix = frame_index // span, (frame_index % span) / float(span)
        amplitude = settings["vhs_edge_wave"] * 0.62 * settings["_h_scale"]
        current = ntsc_core._edge_wave_profile(settings["seed"], epoch, 480, amplitude, settings["_v_scale"])
        upcoming = ntsc_core._edge_wave_profile(settings["seed"], epoch + 1, 480, amplitude, settings["_v_scale"])
        expected = (current * (1.0 - mix) + upcoming * mix).round().astype(np.int64)
        assert np.array_equal(gpu_edge, expected)




def test_alpha_is_preserved_and_single_channel_is_expanded():
    rgba = torch.rand(2, 16, 12, 4)
    out = ntsc_gpu.process_tensor_batch(rgba, ntsc_core.DEFAULT_SETTINGS_JSON)
    assert out.shape == (2, 16, 12, 4)
    assert torch.equal(out[..., 3], rgba[..., 3])

    grey = torch.rand(1, 16, 12, 1)
    assert ntsc_gpu.process_tensor_batch(grey, ntsc_core.DEFAULT_SETTINGS_JSON).shape == (1, 16, 12, 3)


def test_a_three_dimensional_image_is_treated_as_a_one_frame_batch():
    single = torch.rand(16, 12, 3)
    assert ntsc_gpu.process_tensor_batch(single, ntsc_core.DEFAULT_SETTINGS_JSON).shape == (1, 16, 12, 3)


def test_the_output_comes_back_on_the_device_it_arrived_on():
    on_gpu = torch.rand(2, 32, 32, 3, device="cuda")
    assert ntsc_gpu.process_tensor_batch(on_gpu, ntsc_core.DEFAULT_SETTINGS_JSON).device.type == "cuda"
    on_cpu = torch.rand(2, 32, 32, 3)
    assert ntsc_gpu.process_tensor_batch(on_cpu, ntsc_core.DEFAULT_SETTINGS_JSON).device.type == "cpu"


def _recording_bar(steps):
    return lambda total: type("Bar", (), {"update": lambda _self, n: steps.append(n)})()


def test_the_progress_bar_is_stepped_once_per_frame(monkeypatch):
    steps = []
    monkeypatch.setattr(ntsc_core, "_make_progress_bar", _recording_bar(steps))
    ntsc_gpu.process_tensor_batch(torch.rand(5, 16, 16, 3), ntsc_core.DEFAULT_SETTINGS_JSON)
    assert steps == [1, 1, 1, 1, 1]


def test_the_progress_bar_does_not_re_count_frames_an_oom_retry_redid(monkeypatch):
    steps = []
    monkeypatch.setattr(ntsc_core, "_make_progress_bar", _recording_bar(steps))
    original = ntsc_gpu._process_chunk
    failed = []

    def flaky(images, result, start, size, *rest):
        out = original(images, result, start, size, *rest)
        if size > 1 and not failed:
            failed.append(size)
            raise torch.cuda.OutOfMemoryError("out of memory")
        return out

    monkeypatch.setattr(ntsc_gpu, "_process_chunk", flaky)
    ntsc_gpu.process_tensor_batch(torch.rand(4, 16, 16, 3), ntsc_core.DEFAULT_SETTINGS_JSON)
    assert failed, "the test needs the retry path to be taken"
    assert steps == [1, 1, 1, 1]


def test_a_callers_progress_bar_is_used_instead_of_a_new_one(monkeypatch):
    mine, made = [], []
    monkeypatch.setattr(ntsc_core, "_make_progress_bar", _recording_bar(made))
    bar = type("Bar", (), {"update": lambda _self, n: mine.append(n)})()
    ntsc_gpu.process_tensor_batch(torch.rand(3, 16, 16, 3), ntsc_core.DEFAULT_SETTINGS_JSON, progress=bar)
    assert mine == [1, 1, 1] and made == []


def test_an_interrupt_stops_the_batch(monkeypatch):
    seen = []

    def interrupt():
        seen.append(1)
        if len(seen) > 2:
            raise KeyboardInterrupt("cancelled")

    monkeypatch.setattr(ntsc_core, "_raise_if_interrupted", interrupt)
    with pytest.raises(KeyboardInterrupt):
        ntsc_gpu.process_tensor_batch(torch.rand(8, 16, 16, 3), ntsc_core.DEFAULT_SETTINGS_JSON)
    assert len(seen) == 3


def test_a_malformed_batch_is_rejected():
    with pytest.raises(ValueError):
        ntsc_gpu.process_tensor_batch(torch.rand(2, 2, 8, 8, 3), ntsc_core.DEFAULT_SETTINGS_JSON)
    with pytest.raises(ValueError):
        ntsc_gpu.process_tensor_batch(torch.rand(1, 8, 8, 2), ntsc_core.DEFAULT_SETTINGS_JSON)


def test_settings_are_parsed_the_same_way_as_the_cpu_path():
    for payload in ("", "{", "null", '{"preset": "nope", "ringing": "x"}', '{"blend": NaN}'):
        frame = _detail_image(96, 64)
        cpu = ntsc_core.parse_settings_json(payload)
        gpu_out = ntsc_gpu.process_rgb_uint8(frame, payload)
        cpu_out = ntsc_gpu.process_rgb_uint8(frame, cpu)
        assert np.array_equal(gpu_out, cpu_out)




def test_the_chunk_is_bounded_and_shrinks_as_the_frame_grows():
    device = torch.device("cuda")
    sizes = [ntsc_gpu._chunk_frames(500, h, w, device) for (w, h) in [(512, 512), (1920, 1080), (3840, 2160)]]
    assert all(size >= 1 for size in sizes)
    assert sizes == sorted(sizes, reverse=True), f"chunk did not shrink with frame size: {sizes}"
    assert ntsc_gpu._chunk_frames(3, 512, 512, device) <= 3


def test_a_four_k_batch_stays_inside_a_sane_amount_of_vram():
    torch.cuda.empty_cache()
    torch.cuda.reset_peak_memory_stats()
    ntsc_gpu.process_tensor_batch(torch.rand(6, 2160, 3840, 3), {"preset": "First Gen"})
    peak = torch.cuda.max_memory_allocated() / 1024**3
    assert peak < 4.0, f"a 4K batch took {peak:.2f} GiB"


def test_a_chunk_that_will_not_fit_is_retried_smaller(monkeypatch):
    """An allocation failure mid-batch must be recoverable by chunking down."""
    calls = []
    original = ntsc_gpu._process_chunk

    def flaky(images, result, start, size, *rest):
        calls.append(size)
        if size > 1:
            raise torch.cuda.OutOfMemoryError("out of memory")
        return original(images, result, start, size, *rest)

    monkeypatch.setattr(ntsc_gpu, "_process_chunk", flaky)
    out = ntsc_gpu.process_tensor_batch(torch.rand(4, 16, 16, 3), ntsc_core.DEFAULT_SETTINGS_JSON)
    assert out.shape == (4, 16, 16, 3)
    assert max(calls) > 1 and min(calls) == 1


def test_an_unusable_device_raises_something_the_caller_can_fall_back_on():
    frame = _detail_image(32, 32)
    with pytest.raises(ntsc_gpu.RetroTapeGpuError):
        ntsc_gpu.process_rgb_uint8(frame, ntsc_core.DEFAULT_SETTINGS_JSON, device="cpu")
    with pytest.raises(ntsc_gpu.RetroTapeGpuError):
        ntsc_gpu.process_tensor_batch(torch.rand(1, 8, 8, 3), ntsc_core.DEFAULT_SETTINGS_JSON, device="cuda:99")
    assert issubclass(ntsc_gpu.RetroTapeGpuError, RuntimeError)


@pytest.mark.parametrize(
    "error",
    [
        RuntimeError("CUDA error: device-side assert triggered"),
        AttributeError("'NoneType' object has no attribute 'view'"),
        IndexError("index out of range"),
    ],
)
def test_any_frame_path_failure_is_reported_as_a_gpu_error(monkeypatch, error):
    # The caller catches RetroTapeGpuError, so nothing may reach it under another type.
    def explode(*_args, **_kwargs):
        raise error

    monkeypatch.setattr(ntsc_gpu, "_process_frame", explode)
    with pytest.raises(ntsc_gpu.RetroTapeGpuError):
        ntsc_gpu.process_tensor_batch(torch.rand(1, 8, 8, 3), ntsc_core.DEFAULT_SETTINGS_JSON)
    with pytest.raises(ntsc_gpu.RetroTapeGpuError):
        ntsc_gpu.process_rgb_uint8(_detail_image(8, 8), ntsc_core.DEFAULT_SETTINGS_JSON)


def test_the_public_surface_mirrors_the_cpu_module():
    for name in ("is_available", "process_rgb_uint8", "process_tensor_batch", "RetroTapeGpuError"):
        assert hasattr(ntsc_gpu, name)
    assert ntsc_gpu.parse_settings_json is ntsc_core.parse_settings_json
    assert ntsc_gpu._compact_from_preset is ntsc_core._compact_from_preset
    assert ntsc_gpu.DEFAULT_SETTINGS_JSON == ntsc_core.DEFAULT_SETTINGS_JSON


def test_the_shared_constants_come_from_the_cpu_module():
    assert ntsc_gpu._RGB_TO_YIQ == ntsc_core._RGB_TO_YIQ.tolist()
    assert ntsc_gpu._YIQ_TO_RGB == ntsc_core._YIQ_TO_RGB.tolist()
    assert ntsc_gpu._LUMA_ROW == ntsc_core._LUMA_ROW[0].tolist()
    for name in ("_apply_saturation", "_apply_black_lift"):
        assert not hasattr(ntsc_gpu, name), f"{name} is ntsc_core's and runs unchanged on CUDA tensors"


def test_is_available_agrees_with_torch():
    assert ntsc_gpu.is_available() == bool(torch.cuda.is_available())
