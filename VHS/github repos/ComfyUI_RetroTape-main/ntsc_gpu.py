"""CUDA port of ntsc_core's NTSC/VHS pipeline, stage for stage and constant for constant."""

# A port of ntsc_core, which is adapted from ntscQT by JargeZ (Apache-2.0),
# so it carries the same attribution.

from __future__ import annotations

import math
from typing import Any

import numpy as np
import torch
import torch.nn.functional as F

from . import ntsc_core
from .ntsc_core import (  # noqa: F401
    BLACK_LIFT_PEDESTAL,
    CHROMA_LOSS_DIVISOR,
    DEFAULT_PRESET,
    DEFAULT_SETTINGS,
    DEFAULT_SETTINGS_JSON,
    NTSC_RATE,
    REFERENCE_HEIGHT,
    REFERENCE_WIDTH,
    TEMPORAL_EPOCH_FRAMES,
    _compact_from_preset,
    parse_settings_json,
)

# Full-frame noise is drawn from Philox rather than PCG64, so the grain differs from ntsc_core's.

_RGB_TO_YIQ = ntsc_core._RGB_TO_YIQ.tolist()
_YIQ_TO_RGB = ntsc_core._YIQ_TO_RGB.tolist()
_LUMA_ROW = ntsc_core._LUMA_ROW[0].tolist()

_ARANGE_CACHE: dict[tuple[int, str, str], torch.Tensor] = {}
_ARANGE_CACHE_LIMIT = 64

_SCAN_EPSILON = 5e-8


class RetroTapeGpuError(RuntimeError):
    """The CUDA path could not render this batch; the caller should re-run on ntsc_core."""


def is_available() -> bool:
    """True when this module imported cleanly and a CUDA device is present."""
    try:
        return bool(torch.cuda.is_available() and torch.cuda.device_count() > 0)
    except Exception:
        return False


def _oom_error_types() -> tuple[type[BaseException], ...]:
    error = getattr(torch, "OutOfMemoryError", None) or getattr(torch.cuda, "OutOfMemoryError", None)
    return (error,) if error is not None else (RuntimeError,)


def _passthrough_error_types() -> tuple[type[BaseException], ...]:
    try:
        from comfy.model_management import InterruptProcessingException
    except Exception:
        return (RetroTapeGpuError,)
    return (RetroTapeGpuError, InterruptProcessingException)


def _resolve_device(device: Any) -> torch.device:
    if not is_available():
        raise RetroTapeGpuError("No CUDA device is available for the RetroTape GPU path.")
    try:
        resolved = torch.device(device if device is not None else "cuda")
    except (RuntimeError, TypeError, ValueError) as error:
        raise RetroTapeGpuError(f"Not a usable device for the RetroTape GPU path: {device!r}") from error
    if resolved.type != "cuda":
        raise RetroTapeGpuError(f"The RetroTape GPU path needs a CUDA device, got {resolved}.")
    if resolved.index is not None and resolved.index >= torch.cuda.device_count():
        raise RetroTapeGpuError(f"CUDA device {resolved.index} does not exist.")
    return resolved


def _arange(size: int, device: torch.device, dtype: torch.dtype = torch.int64) -> torch.Tensor:
    key = (int(size), str(device), str(dtype))
    cached = _ARANGE_CACHE.get(key)
    if cached is None:
        if len(_ARANGE_CACHE) >= _ARANGE_CACHE_LIMIT:
            _ARANGE_CACHE.clear()
        cached = torch.arange(size, device=device, dtype=dtype)
        _ARANGE_CACHE[key] = cached
    return cached


def _reflect101_positions(positions: torch.Tensor, size: int) -> torch.Tensor:
    # A one-wide picture folds everything to 0, as cv2.borderInterpolate does.
    if size <= 1:
        return torch.zeros_like(positions)
    period = 2 * size - 2
    folded = positions.abs() % period
    return torch.where(folded >= size, period - folded, folded)


def _pad_reflect101_w(plane: torch.Tensor, left: int, right: int) -> torch.Tensor:
    if left <= 0 and right <= 0:
        return plane
    width = plane.shape[-1]
    positions = _arange(width + left + right, plane.device) - left
    return plane.index_select(-1, _reflect101_positions(positions, width))


def _pad_reflect101_h(plane: torch.Tensor, top: int, bottom: int) -> torch.Tensor:
    if top <= 0 and bottom <= 0:
        return plane
    height = plane.shape[-2]
    positions = _arange(height + top + bottom, plane.device) - top
    return plane.index_select(-2, _reflect101_positions(positions, height))


def _shift_plane(plane: torch.Tensor, x_shift: int, y_shift: int = 0) -> torch.Tensor:
    x_shift = int(x_shift)
    y_shift = int(y_shift)
    if x_shift == 0 and y_shift == 0:
        return plane
    out = plane
    if y_shift:
        height = out.shape[-2]
        rows = (_arange(height, out.device) - y_shift).clamp(0, height - 1)
        out = out.index_select(-2, rows)
    if x_shift:
        width = out.shape[-1]
        cols = (_arange(width, out.device) - x_shift).clamp(0, width - 1)
        out = out.index_select(-1, cols)
    return out


def _row_columns(shifts: torch.Tensor, width: int) -> torch.Tensor:
    columns = _arange(width, shifts.device).unsqueeze(0) - shifts.unsqueeze(1)
    return columns.clamp(0, width - 1)


def _shift_rows(plane: torch.Tensor, columns: torch.Tensor) -> torch.Tensor:
    if plane.dim() == columns.dim():
        return plane.gather(-1, columns)
    return plane.gather(-1, columns.expand(plane.shape[:-2] + columns.shape))


def _correlate_w(planes: torch.Tensor, taps: torch.Tensor, left: int, right: int, replicate: bool) -> torch.Tensor:
    channels, height, width = planes.shape
    if replicate:
        columns = (_arange(width + left + right, planes.device) - left).clamp(0, width - 1)
        padded = planes.index_select(-1, columns)
    else:
        padded = _pad_reflect101_w(planes, left, right)
    weight = taps.view(1, 1, 1, -1).expand(channels, 1, 1, taps.numel()).contiguous()
    out = F.conv2d(padded.unsqueeze(0).contiguous(), weight, groups=channels)
    return out.view(channels, height, width)


def _correlate_h(planes: torch.Tensor, taps: torch.Tensor, top: int, bottom: int) -> torch.Tensor:
    channels, height, width = planes.shape
    padded = _pad_reflect101_h(planes, top, bottom)
    weight = taps.view(1, 1, -1, 1).expand(channels, 1, taps.numel(), 1).contiguous()
    out = F.conv2d(padded.unsqueeze(0).contiguous(), weight, groups=channels)
    return out.view(channels, height, width)


def _gaussian_taps(sigma: float) -> np.ndarray:
    # cv2.GaussianBlur sizes a float32 ksize (0, 0) as cvRound(sigma * 4 * 2 + 1) | 1.
    size = int(np.rint(sigma * 4.0 * 2.0 + 1.0)) | 1
    size = max(1, size)
    x = np.arange(size, dtype=np.float64) - (size - 1) * 0.5
    taps = np.exp(-0.5 / (sigma * sigma) * x * x)
    taps /= taps.sum()
    return taps.astype(np.float32)


_TAP_CACHE: dict[tuple, torch.Tensor] = {}


def _cached_constant(key: tuple, build, device: torch.device) -> torch.Tensor:
    full = key + (str(device),)
    cached = _TAP_CACHE.get(full)
    if cached is None:
        if len(_TAP_CACHE) > 64:
            _TAP_CACHE.clear()
        cached = _upload(build(), device)
        _TAP_CACHE[full] = cached
    return cached


def _gaussian_blur(planes: torch.Tensor, sigma: float) -> torch.Tensor:
    if sigma <= 0.0:
        return planes
    sigma = float(sigma)
    if (int(np.rint(sigma * 4.0 * 2.0 + 1.0)) | 1) <= 1:
        return planes
    taps = _cached_constant(("gauss", round(sigma, 9)), lambda: _gaussian_taps(sigma), planes.device)
    radius = taps.numel() // 2
    # OpenCV's separable engine runs the row filter first, then the column one.
    out = _correlate_w(planes, taps, radius, radius, replicate=False)
    return _correlate_h(out, taps, radius, radius)


_SCAN_PLANS: dict[tuple, tuple[torch.Tensor, torch.Tensor]] = {}


def _scan_plan(
    cutoffs: tuple[float, ...], h_scale: float, width: int, device: torch.device
) -> tuple[torch.Tensor, torch.Tensor]:
    key = (cutoffs, round(float(h_scale), 9), int(width), str(device))
    cached = _SCAN_PLANS.get(key)
    if cached is not None:
        return cached
    interval = 1.0 / (NTSC_RATE * max(0.05, h_scale))
    alphas = np.array(
        [interval / (1.0 / (cutoff * 2.0 * math.pi) + interval) for cutoff in cutoffs], dtype=np.float64
    )
    poles = 1.0 - alphas
    steps = []
    step = 1
    while step < width and float(np.max(poles**step)) > _SCAN_EPSILON:
        steps.append(poles**step)
        step *= 2
    shape = (len(cutoffs), 1, 1)
    stacked = np.stack(steps) if steps else np.zeros((0, len(cutoffs)), np.float64)
    alpha_t = _upload(alphas.astype(np.float32).reshape(shape), device)
    coef_t = _upload(stacked.astype(np.float32).reshape((len(steps),) + shape), device)
    if len(_SCAN_PLANS) > 64:
        _SCAN_PLANS.clear()
    _SCAN_PLANS[key] = (alpha_t, coef_t)
    return alpha_t, coef_t


def _one_pole_scan(planes: torch.Tensor, alpha: torch.Tensor, coefficients: torch.Tensor) -> torch.Tensor:
    # Hillis-Steele doubling over y[n] = alpha * x[n] + pole * y[n-1], with pole**d precomputed.
    width = planes.shape[-1]
    out = planes * alpha
    step = 1
    for index in range(coefficients.shape[0]):
        nxt = torch.empty_like(out)
        nxt[..., :step].copy_(out[..., :step])
        torch.addcmul(out[..., step:], out[..., : width - step], coefficients[index], out=nxt[..., step:])
        out = nxt
        step *= 2
    return out


def _lowpass_rows(
    planes: torch.Tensor, cutoffs: tuple[float, ...], passes: int = 3, h_scale: float = 1.0
) -> torch.Tensor:
    # planes is (P, H, W), one cutoff per plane.
    if planes.numel() == 0:
        return planes
    alpha, coefficients = _scan_plan(cutoffs, h_scale, int(planes.shape[-1]), planes.device)
    out = planes
    for _ in range(passes):
        out = _one_pole_scan(out, alpha, coefficients)
    return out


def _upload(array: np.ndarray, device: torch.device) -> torch.Tensor:
    array = np.ascontiguousarray(array)
    if not array.flags.writeable:
        # torch.from_numpy warns on a read-only buffer, and PIL-backed frames arrive read-only.
        array = array.copy()
    return torch.from_numpy(array).to(device, non_blocking=False)


def _upload_pack(arrays: list[np.ndarray | None], device: torch.device) -> list[torch.Tensor | None]:
    out: list[torch.Tensor | None] = [None] * len(arrays)
    groups: dict[str, list[int]] = {}
    for index, array in enumerate(arrays):
        if array is None or array.size == 0:
            continue
        groups.setdefault(array.dtype.str, []).append(index)
    for indices in groups.values():
        flat = np.concatenate([np.ascontiguousarray(arrays[index]).ravel() for index in indices])
        buffer = _upload(flat, device)
        offset = 0
        for index in indices:
            size = int(arrays[index].size)
            out[index] = buffer[offset : offset + size].view(arrays[index].shape)
            offset += size
    return out


def _tracking_shifts(settings: dict[str, Any], frame_index: int, height: int, width: int) -> np.ndarray | None:
    jitter = settings["tracking_jitter"]
    warp = settings["tape_warp"]
    if jitter <= 0.0 and warp <= 0.0:
        return None

    h_scale = settings.get("_h_scale", 1.0)
    v_scale = settings.get("_v_scale", 1.0)
    rows = np.arange(height, dtype=np.float32) / v_scale
    shifts = np.zeros(height, dtype=np.float32)
    if warp > 0.0:
        phase = frame_index * 0.17 + settings["seed"] * 0.0003
        shifts += np.sin(rows * (0.020 + warp * 0.0015) + phase) * warp * 0.62
        shifts += np.sin(rows * 0.071 + phase * 1.9) * warp * 0.28

    if jitter > 0.0:
        span = max(1, TEMPORAL_EPOCH_FRAMES)
        epoch = frame_index // span
        mix = (frame_index % span) / float(span)
        current = ntsc_core._servo_profile(settings["seed"], epoch, height, jitter, v_scale)
        upcoming = ntsc_core._servo_profile(settings["seed"], epoch + 1, height, jitter, v_scale)
        shifts += current * (1.0 - mix) + upcoming * mix
        # These are the slow stream's first draws; _apply_vhs is handed it but never draws.
        rng = _slow_rng(settings, frame_index)
        for _ in range(int(jitter // 3)):
            start = int(rng.integers(0, max(1, height)))
            length = int(rng.integers(max(2, height // 80), max(3, height // 18)))
            end = min(height, start + length)
            if end <= start:
                continue
            amount = rng.normal(0.0, jitter * 1.65)
            shifts[start:end] += np.linspace(amount, 0.0, end - start, dtype=np.float32)

    limit = max(1, width // 3)
    return np.clip(np.round(shifts * h_scale), -limit, limit).astype(np.int64)


def _edge_wave_shifts(settings: dict[str, Any], frame_index: int, height: int) -> np.ndarray | None:
    if not settings["vhs_enabled"] or settings["vhs_edge_wave"] <= 0:
        return None
    span = max(1, TEMPORAL_EPOCH_FRAMES)
    epoch = frame_index // span
    mix = (frame_index % span) / float(span)
    amplitude = settings["vhs_edge_wave"] * 0.62 * settings.get("_h_scale", 1.0)
    v_scale = settings.get("_v_scale", 1.0)
    current = ntsc_core._edge_wave_profile(settings["seed"], epoch, height, amplitude, v_scale)
    upcoming = ntsc_core._edge_wave_profile(settings["seed"], epoch + 1, height, amplitude, v_scale)
    return (current * (1.0 - mix) + upcoming * mix).round().astype(np.int64)


def _dropout_bands(settings: dict[str, Any], frame_index: int, height: int, width: int) -> list[dict[str, Any]]:
    # The stripe noise draw sits between bands in the same stream; skipping it moves every later band.
    strength = settings["dropouts"]
    if strength <= 0.0:
        return []
    v_scale = settings.get("_v_scale", 1.0)
    rng = _dropout_rng(settings, frame_index)
    bands: list[dict[str, Any]] = []
    count = int(rng.poisson(max(0.0, strength) * max(1.0, height / (22.0 * v_scale))))
    for _ in range(count):
        row = int(rng.integers(0, height))
        band = int(rng.integers(1, max(2, int(round((2 + strength * 7) * v_scale)))))
        y1 = min(height, row + band)
        x0 = int(rng.integers(0, max(1, width)))
        length = int(rng.integers(max(2, width // 10), max(3, width)))
        x1 = min(width, x0 + length)
        if x1 <= x0:
            continue
        alpha = float(np.clip(0.24 + strength * rng.uniform(0.28, 0.72), 0.0, 0.96))
        target = 235.0 if rng.random() > 0.35 else 18.0
        noise = rng.normal(0.0, 18.0 + strength * 22.0, size=(y1 - row, x1 - x0)).astype(np.float32)
        bands.append({"row": row, "y1": y1, "x0": x0, "x1": x1, "alpha": alpha, "target": target, "noise": noise})
    return bands


def _frame_seed(settings: dict[str, Any], frame_index: int) -> int:
    return int((int(settings["seed"]) * 1_000_003 + int(frame_index) * 1009) % (2**32))


def _head_switch_noise(settings: dict[str, Any], frame_index: int, rows: int, width: int) -> np.ndarray:
    # numpy fills a 2-D normal in C order, so one draw equals the CPU loop's width-wide draws.
    rng = np.random.default_rng(_frame_seed(settings, frame_index))
    return rng.normal(0.0, 8.0, size=(rows, width)).astype(np.float32)


def _head_switch_plan(settings: dict[str, Any], frame_index: int, height: int, width: int) -> dict[str, Any]:
    band = max(2, int(height * 0.045))
    start = height - band
    base_shift = int(max(4, width * 0.035))
    rows = np.arange(start, height, dtype=np.int64)
    decay = 1.0 - (rows - start) / max(1, band)
    return {
        "start": start,
        # decay is non-negative, so the CPU path's int() truncation is a floor.
        "shifts": np.maximum(1, np.trunc(base_shift * decay).astype(np.int64)),
        "gains": (0.75 + 0.2 * decay).astype(np.float32),
        "noise": _head_switch_noise(settings, frame_index, rows.size, width),
    }


def _slow_rng(settings: dict[str, Any], frame_index: int) -> np.random.Generator:
    span = max(1, TEMPORAL_EPOCH_FRAMES)
    return np.random.default_rng(
        (int(settings["seed"]) * 2_654_435_761 + (int(frame_index) // span) * 40_503) % (2**32)
    )


def _dropout_rng(settings: dict[str, Any], frame_index: int) -> np.random.Generator:
    span = max(1, TEMPORAL_EPOCH_FRAMES)
    return np.random.default_rng(
        (int(settings["seed"]) * 2_246_822_519 + ((int(frame_index) + span // 2) // span) * 40_503) % (2**32)
    )


def _rgb_to_yiq(rgb: torch.Tensor) -> tuple[torch.Tensor, torch.Tensor, torch.Tensor]:
    (yr, yg, yb), (ir, ig, ib), (qr, qg, qb) = _RGB_TO_YIQ
    r = rgb[..., 0]
    g = rgb[..., 1]
    b = rgb[..., 2]
    return r * yr + g * yg + b * yb, r * ir + g * ig + b * ib, r * qr + g * qg + b * qb


def _yiq_to_rgb(y: torch.Tensor, i: torch.Tensor, q: torch.Tensor) -> torch.Tensor:
    # The matrix's luma column is 1, so it is dropped rather than multiplied.
    (_, ri, rq), (_, gi, gq), (_, bi, bq) = _YIQ_TO_RGB
    r = y + i * ri + q * rq
    g = y + i * gi + q * gq
    b = y + i * bi + q * bq
    # torch.round is round-half-to-even, which is what np.rint does.
    return torch.stack([r, g, b], dim=0).clamp(0.0, 255.0).round()


def _pad_width_to_multiple_of_four(rgb: torch.Tensor) -> tuple[torch.Tensor, int]:
    pad = (-rgb.shape[1]) % 4
    if pad == 0:
        return rgb, 0
    width = rgb.shape[1]
    positions = _arange(width + pad, rgb.device)
    return rgb.index_select(1, _reflect101_positions(positions, width)), pad


def _apply_color_bleed(
    i: torch.Tensor, q: torch.Tensor, h_shift: int, v_shift: int
) -> tuple[torch.Tensor, torch.Tensor]:
    if h_shift == 0 and v_shift == 0:
        return i, q
    return _shift_plane(i, h_shift, v_shift), _shift_plane(q, h_shift, v_shift)


def _apply_chroma_delay(i: torch.Tensor, q: torch.Tensor, delay: int) -> tuple[torch.Tensor, torch.Tensor]:
    if delay <= 0:
        return i, q
    i_delayed = _shift_plane(i, delay, 0)
    q_delayed = _shift_plane(q, max(1, delay // 2), 0)
    return i * 0.35 + i_delayed * 0.65, q * 0.45 + q_delayed * 0.55


def _apply_color_drift(
    i: torch.Tensor, q: torch.Tensor, settings: dict[str, Any], frame_index: int
) -> tuple[torch.Tensor, torch.Tensor]:
    amount = settings["color_drift"]
    if amount <= 0.0:
        return i, q
    rows = _arange(i.shape[0], i.device, torch.float32).unsqueeze(1)
    base = math.sin(frame_index * 0.19 + settings["seed"] * 0.00011) * amount * 0.36
    v_scale = settings.get("_v_scale", 1.0)
    row_angle = base + torch.sin(rows * ((0.013 + amount * 0.007) / v_scale) + frame_index * 0.11) * amount * 0.16
    sin = torch.sin(row_angle)
    cos = torch.cos(row_angle)
    chroma_gain = 1.0 + amount * 0.16
    return (i * cos - q * sin) * chroma_gain, (i * sin + q * cos) * chroma_gain


def _apply_composite_artifacts(
    y: torch.Tensor, i: torch.Tensor, q: torch.Tensor, settings: dict[str, Any]
) -> tuple[torch.Tensor, torch.Tensor, torch.Tensor]:
    height, width = y.shape
    x = _arange(width, y.device, torch.float32).unsqueeze(0)
    rows = _arange(height, y.device, torch.float32).unsqueeze(1)
    row_phase = math.radians(settings["scanline_phase"])
    h_scale = settings.get("_h_scale", 1.0)
    v_scale = settings.get("_v_scale", 1.0)
    carrier = torch.sin(((x / h_scale + settings["scanline_offset"]) * (math.pi / 2.0)) + (rows / v_scale) * row_phase)
    chroma = torch.hypot(i, q)
    y = y + carrier * chroma * (0.045 + settings["scanlines"] * 0.065)
    chroma_lp = _lowpass_rows(torch.stack((i, q)), (1_300_000.0, 600_000.0), h_scale=h_scale)
    return y, chroma_lp[0], chroma_lp[1]


def _apply_ringing(y: torch.Tensor, settings: dict[str, Any]) -> torch.Tensor:
    strength = settings["ringing"]
    if strength <= 0.0:
        return y
    power = settings["ringing_power"]
    h_scale = settings.get("_h_scale", 1.0)
    blur = _gaussian_blur(y.unsqueeze(0), (1.25 + power * 0.15) * h_scale)[0]
    edge = y - blur
    taps = _cached_constant(
        ("ring", round(h_scale, 9)),
        lambda: ntsc_core._resample_kernel(ntsc_core._RINGING_TAPS, h_scale),
        y.device,
    )
    radius = taps.numel() // 2
    ring = _correlate_w(edge.unsqueeze(0), taps, radius, radius, replicate=False)[0]
    forward = ntsc_core._scaled_shift(max(1, power), h_scale, 1)
    backward = ntsc_core._scaled_shift(max(1, power // 2), h_scale, 1)
    echo = _shift_plane(edge, forward, 0) - _shift_plane(edge, -backward, 0)
    return y + ring * (strength * (1.05 + power * 0.22)) + echo * strength * 0.22


def _apply_noise(
    y: torch.Tensor,
    i: torch.Tensor,
    q: torch.Tensor,
    settings: dict[str, Any],
    generator: torch.Generator,
) -> tuple[torch.Tensor, torch.Tensor, torch.Tensor]:
    device = y.device
    if settings["video_noise"] > 0:
        sigma = settings["video_noise"] / 9000.0 * 42.0
        noise = torch.randn(y.shape, generator=generator, device=device, dtype=torch.float32) * sigma
        noise = _lowpass_rows(
            noise.unsqueeze(0), (2_000_000.0,), passes=1, h_scale=settings.get("_h_scale", 1.0)
        )[0]
        fine = torch.randn(y.shape, generator=generator, device=device, dtype=torch.float32) * (sigma * 0.28)
        y = y + (noise + fine)

    if settings["chroma_noise"] > 0:
        sigma = settings["chroma_noise"] / 32768.0 * 82.0
        i = i + torch.randn(i.shape, generator=generator, device=device, dtype=torch.float32) * sigma
        q = q + torch.randn(q.shape, generator=generator, device=device, dtype=torch.float32) * sigma

    if settings["chroma_phase_noise"] > 0:
        row_noise = torch.randn(
            (i.shape[0], 1), generator=generator, device=device, dtype=torch.float32
        ) * (settings["chroma_phase_noise"] / 120.0 * 0.72)
        sin = torch.sin(row_noise)
        cos = torch.cos(row_noise)
        i, q = i * cos - q * sin, i * sin + q * cos

    if settings["chroma_loss"] > 0:
        chance = settings["chroma_loss"] / CHROMA_LOSS_DIVISOR
        mask = torch.rand((i.shape[0], 1), generator=generator, device=device, dtype=torch.float32) < chance
        i = torch.where(mask, i * 0.08, i)
        q = torch.where(mask, q * 0.08, q)

    return y, i, q


def _apply_ghosting(
    y: torch.Tensor, i: torch.Tensor, q: torch.Tensor, settings: dict[str, Any]
) -> tuple[torch.Tensor, torch.Tensor, torch.Tensor]:
    amount = settings["ghosting"]
    delay = settings["chroma_delay"]
    if amount <= 0.0 and delay <= 0:
        return y, i, q
    h_scale = settings.get("_h_scale", 1.0)
    width = y.shape[1]
    if amount > 0.0:
        echo_delay = max(1, int(round(delay * h_scale + width * amount * 0.035)))
        second_delay = max(echo_delay + 1, int(echo_delay * 2.35))
        y = y * (1.0 - amount * 0.18) + _shift_plane(y, echo_delay, 0) * (amount * 0.15)
        y = y + _shift_plane(y, second_delay, 0) * (amount * 0.06)
    if delay > 0:
        i, q = _apply_chroma_delay(i, q, ntsc_core._scaled_shift(delay, h_scale, 1))
    return y, i, q


def _apply_vhs(
    y: torch.Tensor,
    i: torch.Tensor,
    q: torch.Tensor,
    settings: dict[str, Any],
    edge_columns: torch.Tensor | None,
) -> tuple[torch.Tensor, torch.Tensor, torch.Tensor]:
    if not settings["vhs_enabled"]:
        return y, i, q

    h_scale = settings.get("_h_scale", 1.0)
    planes = _lowpass_rows(
        torch.stack((y, i, q)), (1_900_000.0, 300_000.0, 300_000.0), h_scale=h_scale
    )
    if edge_columns is not None:
        planes = _shift_rows(planes, edge_columns)
    y, i, q = planes[0], planes[1], planes[2]

    y_blur = _gaussian_blur(y.unsqueeze(0), 0.75 * h_scale)[0]
    y = y + (y - y_blur) * (0.65 + settings["ringing"] * 0.28)

    if y.shape[0] >= 2:
        # numpy builds the right-hand side whole, so this is not a running filter down the frame.
        i = torch.cat((i[:1], (i[1:] + i[:-1]) * 0.5), dim=0)
        q = torch.cat((q[:1], (q[1:] + q[:-1]) * 0.5), dim=0)

    return y, i, q


def _apply_tracking_warp(
    y: torch.Tensor, i: torch.Tensor, q: torch.Tensor, columns: torch.Tensor | None
) -> tuple[torch.Tensor, torch.Tensor, torch.Tensor]:
    if columns is None:
        return y, i, q
    planes = _shift_rows(torch.stack((y, i, q)), columns)
    return planes[0], planes[1], planes[2]


def _apply_interlace_roll(
    y: torch.Tensor,
    i: torch.Tensor,
    q: torch.Tensor,
    settings: dict[str, Any],
    frame_index: int,
    odd_mask: torch.Tensor,
) -> tuple[torch.Tensor, torch.Tensor, torch.Tensor]:
    roll_amount = settings["vertical_roll"]
    if roll_amount > 0.0:
        offset = int(
            round(math.sin(frame_index * 0.23 + settings["seed"] * 0.00017) * roll_amount * y.shape[0] * 0.045)
        )
        if offset:
            y = torch.roll(y, offset, dims=0)
            i = torch.roll(i, offset, dims=0)
            q = torch.roll(q, offset, dims=0)

    interlace = settings["interlace"]
    if interlace <= 0.0 or y.shape[0] < 2:
        return y, i, q

    step = ntsc_core._scaled_shift(1, settings.get("_h_scale", 1.0), 1)
    shifted_y = _shift_plane(y, step if frame_index % 2 == 0 else -step, 0)
    shifted_i = _shift_plane(i, step, 0)
    shifted_q = _shift_plane(q, step, 0)
    y = torch.where(odd_mask, y * (1.0 - interlace * 0.22) + shifted_y * (interlace * 0.08), y)
    i = torch.where(odd_mask, i * (1.0 - interlace * 0.16) + shifted_i * (interlace * 0.08), i)
    q = torch.where(odd_mask, q * (1.0 - interlace * 0.16) + shifted_q * (interlace * 0.08), q)
    return y, i, q


def _apply_luma_smear(y: torch.Tensor, settings: dict[str, Any]) -> torch.Tensor:
    amount = settings["luma_smear"]
    if amount <= 0.0:
        return y
    length = max(3, int(round((3 + amount * 23) * settings.get("_h_scale", 1.0))))
    length += 1 - length % 2

    def build() -> np.ndarray:
        decay = np.exp(-np.linspace(0.0, 3.2, length, dtype=np.float32))
        return (decay / max(1e-6, float(decay.sum())))[::-1].copy()

    taps = _cached_constant(("smear", length), build, y.device)
    # The kernel is anchored at its end, so the smear is causal: left padding only, replicated.
    both = torch.stack([y, torch.clamp(y - 154.0, min=0.0)], dim=0)
    filtered = _correlate_w(both, taps, length - 1, 0, replicate=True)
    smear = filtered[0]
    glow_trail = filtered[1]
    return y * (1.0 - amount * 0.18) + smear * (amount * 0.18) + glow_trail * (amount * 0.22)


def _apply_dropouts(
    y: torch.Tensor,
    i: torch.Tensor,
    q: torch.Tensor,
    settings: dict[str, Any],
    bands: list[dict[str, Any]],
    stripes: torch.Tensor | None,
    generator: torch.Generator,
) -> tuple[torch.Tensor, torch.Tensor, torch.Tensor]:
    strength = settings["dropouts"]
    if strength <= 0.0:
        return y, i, q

    if bands:
        offset = 0
        # Bands can overlap and the blend is order-dependent, so they go on one at a time.
        for band in bands:
            row, y1, x0, x1 = band["row"], band["y1"], band["x0"], band["x1"]
            size = (y1 - row) * (x1 - x0)
            noise = stripes[offset : offset + size].view(y1 - row, x1 - x0)
            offset += size
            alpha = band["alpha"]
            block = y[row:y1, x0:x1]
            y[row:y1, x0:x1] = block * (1.0 - alpha) + (band["target"] + noise) * alpha
            i[row:y1, x0:x1] *= 1.0 - alpha * 0.75
            q[row:y1, x0:x1] *= 1.0 - alpha * 0.75

    h_scale = settings.get("_h_scale", 1.0)
    v_scale = settings.get("_v_scale", 1.0)
    chance = strength * 0.0018 / max(0.05, h_scale * v_scale)
    speckle = torch.rand(y.shape, generator=generator, device=y.device, dtype=torch.float32) < chance
    level = torch.rand(y.shape, generator=generator, device=y.device, dtype=torch.float32) * 255.0
    return torch.where(speckle, level, y), i, q


def _apply_scanlines(y: torch.Tensor, settings: dict[str, Any], alternating: torch.Tensor) -> torch.Tensor:
    strength = settings["scanlines"]
    if strength <= 0.0:
        return y
    height, width = y.shape
    h_scale = settings.get("_h_scale", 1.0)
    v_scale = settings.get("_v_scale", 1.0)
    rows = (_arange(height, y.device, torch.float32) / v_scale).unsqueeze(1)
    x = _arange(width, y.device, torch.float32).unsqueeze(0)
    triad = 2.0 * math.pi / max(1.0, 3.0 * h_scale)
    fine = 0.5 + 0.5 * torch.sin(x * triad + rows * 0.03 + settings["scanline_offset"])
    darken = 1.0 - strength * (0.16 * alternating + 0.08 * fine)
    return y * darken


def _apply_head_switching(
    y: torch.Tensor,
    i: torch.Tensor,
    q: torch.Tensor,
    plan: dict[str, Any],
    shifts: torch.Tensor,
    gains: torch.Tensor,
    noise: torch.Tensor,
) -> tuple[torch.Tensor, torch.Tensor, torch.Tensor]:
    height, width = y.shape
    start = plan["start"]

    if start < 0:
        # A one-row picture: the CPU loop starts at -1 and writes row 0 twice, in order.
        for offset, row in enumerate(range(start, height)):
            columns = (_arange(width, y.device) - int(plan["shifts"][offset])).clamp(0, width - 1)
            y[row] = y[row].index_select(0, columns) * float(plan["gains"][offset]) + noise[offset]
            i[row] *= 0.45
            q[row] *= 0.45
        return y, i, q

    y[start:] = _shift_rows(y[start:], _row_columns(shifts, width)) * gains.unsqueeze(1) + noise
    i[start:] *= 0.45
    q[start:] *= 0.45
    return y, i, q


def _apply_analog_glow_rgb(rgb: torch.Tensor, settings: dict[str, Any]) -> torch.Tensor:
    amount = settings["analog_glow"]
    if amount <= 0.0:
        return rgb
    luma = rgb[0] * _LUMA_ROW[0] + rgb[1] * _LUMA_ROW[1] + rgb[2] * _LUMA_ROW[2]
    mask = ((luma - 135.0) / 120.0).clamp(0.0, 1.0)
    bloom = _gaussian_blur(rgb * mask, (1.8 + amount * 5.0) * settings.get("_h_scale", 1.0))
    gain = _cached_constant(
        ("glow", round(float(amount), 9)),
        lambda: ntsc_core._GLOW_CHANNEL_GAIN * np.float32(amount),
        rgb.device,
    )
    return (rgb + bloom * gain.view(3, 1, 1)).clamp(0.0, 255.0).round()


def _process_frame(source: torch.Tensor, settings: dict[str, Any], frame_index: int) -> torch.Tensor:
    # source is (H, W, 3) float32 holding 0..255 integer levels, on the device.
    if source.shape[0] == 0 or source.shape[1] == 0:
        # An empty axis would clamp(0, -1) and trip a device assert, poisoning the CUDA context.
        return source
    device = source.device
    padded, pad = _pad_width_to_multiple_of_four(source)
    h_scale, v_scale = ntsc_core._frame_scale(padded.shape)
    settings = dict(settings)
    settings["_h_scale"] = h_scale
    settings["_v_scale"] = v_scale
    height, width = padded.shape[0], padded.shape[1]

    generator = torch.Generator(device=device)
    generator.manual_seed(_frame_seed(settings, frame_index))

    edge_np = _edge_wave_shifts(settings, frame_index, height)
    tracking_np = _tracking_shifts(settings, frame_index, height, width)
    bands = _dropout_bands(settings, frame_index, height, width)
    stripes_np = np.concatenate([band["noise"].ravel() for band in bands]) if bands else None
    head = _head_switch_plan(settings, frame_index, height, width) if settings["head_switching"] else None
    reference = ntsc_core._reference_lines(height, settings)

    edge_t, tracking_t, odd_t, alternating_t, stripes_t, head_shifts, head_gains, head_noise = _upload_pack(
        [
            edge_np,
            tracking_np,
            ((reference + frame_index) % 2 == 1),
            (reference % 2).astype(np.float32),
            stripes_np,
            head["shifts"] if head else None,
            head["gains"] if head else None,
            head["noise"] if head else None,
        ],
        device,
    )
    edge_columns = _row_columns(edge_t, width) if edge_t is not None else None
    tracking_columns = _row_columns(tracking_t, width) if tracking_t is not None else None
    odd_mask = odd_t.view(height, 1)
    alternating = alternating_t.view(height, 1)

    y, i, q = _rgb_to_yiq(padded)
    i, q = ntsc_core._apply_saturation(i, q, settings)
    y = ntsc_core._apply_black_lift(y, settings)
    i, q = _apply_color_bleed(
        i,
        q,
        ntsc_core._scaled_shift(settings["color_bleed_h"], h_scale),
        ntsc_core._scaled_shift(settings["color_bleed_v"], v_scale),
    )
    i, q = _apply_color_drift(i, q, settings, frame_index)
    y, i, q = _apply_composite_artifacts(y, i, q, settings)
    y = _apply_ringing(y, settings)
    y, i, q = _apply_noise(y, i, q, settings, generator)
    y, i, q = _apply_ghosting(y, i, q, settings)
    y, i, q = _apply_vhs(y, i, q, settings, edge_columns)
    y, i, q = _apply_tracking_warp(y, i, q, tracking_columns)
    y, i, q = _apply_interlace_roll(y, i, q, settings, frame_index, odd_mask)
    y = _apply_luma_smear(y, settings)
    # _apply_dropouts writes into its planes, and the stages above may hold views onto them.
    y, i, q = _apply_dropouts(
        y.contiguous(), i.contiguous(), q.contiguous(), settings, bands, stripes_t, generator
    )
    y = _apply_scanlines(y, settings, alternating)
    if head is not None:
        y, i, q = _apply_head_switching(
            y.contiguous(), i.contiguous(), q.contiguous(), head, head_shifts, head_gains, head_noise
        )

    processed = _yiq_to_rgb(y, i, q)
    if pad:
        processed = processed[:, :, :-pad]
    processed = _apply_analog_glow_rgb(processed.contiguous(), settings)
    processed = processed.permute(1, 2, 0)

    blend = settings["blend"]
    if blend < 1.0:
        processed = (processed * blend + source * (1.0 - blend)).clamp(0.0, 255.0).round()
    return processed


def process_rgb_uint8(
    image_rgb: np.ndarray,
    settings_json: str | dict[str, Any] | None = None,
    *,
    frame_index: int = 0,
    device: Any = "cuda",
) -> np.ndarray:
    """The GPU twin of ntsc_core.process_rgb_uint8: HxWx3 uint8 in and out."""
    target = _resolve_device(device)
    settings = parse_settings_json(settings_json)
    source = np.ascontiguousarray(image_rgb[..., :3])
    if source.dtype != np.uint8:
        source = np.clip(source, 0, 255).astype(np.uint8)
    try:
        with torch.no_grad():
            uploaded = _upload(source, target).to(torch.float32)
            processed = _process_frame(uploaded, settings, int(frame_index))
            return processed.to(torch.uint8).contiguous().cpu().numpy()
    except _passthrough_error_types():
        raise
    except _oom_error_types() as error:
        _empty_cache()
        raise RetroTapeGpuError(
            f"CUDA ran out of memory rendering a {source.shape[1]}x{source.shape[0]} frame."
        ) from error
    except Exception as error:
        _empty_cache()
        raise RetroTapeGpuError(f"The RetroTape CUDA path failed: {error}") from error


def _empty_cache() -> None:
    try:
        torch.cuda.empty_cache()
    except Exception:
        pass


class _FrameProgress:
    def __init__(self, total: int, bar: Any = None) -> None:
        self._bar = ntsc_core._make_progress_bar(total) if bar is None else bar
        self._done = 0

    def reached(self, index: int) -> None:
        # An OOM retry re-renders frames already finished, which must not be counted twice.
        if self._bar is not None and index >= self._done:
            self._done = index + 1
            self._bar.update(1)


def _chunk_frames(count: int, height: int, width: int, device: torch.device) -> int:
    try:
        free, _total = torch.cuda.mem_get_info(device)
    except Exception:
        free = 2 * 1024**3
    frame_bytes = float(height) * float(width) * 3.0 * 4.0
    if frame_bytes <= 0:
        return max(1, count)
    budget = min(free * 0.4 - frame_bytes * 11.0, 128.0 * 1024**2)
    if budget <= frame_bytes * 2.0:
        return 1
    return int(max(1, min(count, int(budget // (frame_bytes * 2.0)), 64)))


def process_tensor_batch(
    images: torch.Tensor,
    settings_json: str | dict[str, Any] | None = None,
    frame_offset: int = 0,
    *,
    device: Any = "cuda",
    progress: Any = None,
) -> torch.Tensor:
    """The GPU twin of ntsc_core.process_tensor_batch, with the same frame_offset contract."""
    if images.ndim == 3:
        images = images.unsqueeze(0)
    if images.ndim != 4:
        raise ValueError("IMAGE input must be an NHWC batch.")
    if images.shape[0] == 0 or images.shape[1] == 0 or images.shape[2] == 0:
        return images

    channels = int(images.shape[-1])
    if channels < 1 or channels == 2:
        raise ValueError("IMAGE tensors must carry 1, 3 or 4 channels.")

    target = _resolve_device(device)
    settings = parse_settings_json(settings_json)
    count = int(images.shape[0])
    height = int(images.shape[1])
    width = int(images.shape[2])
    ticker = _FrameProgress(count, progress)

    try:
        result = torch.empty((count, height, width, 3), dtype=torch.float32, device=images.device)
        chunk = _chunk_frames(count, height, width, target)
        with torch.no_grad():
            start = 0
            while start < count:
                size = min(chunk, count - start)
                while True:
                    try:
                        _process_chunk(images, result, start, size, settings, int(frame_offset), target, ticker)
                        break
                    except _oom_error_types():
                        _empty_cache()
                        if size == 1:
                            raise RetroTapeGpuError(
                                f"CUDA ran out of memory on a single {width}x{height} frame."
                            ) from None
                        size = max(1, size // 2)
                        chunk = size
                start += size
        if channels == 4:
            result = torch.cat([result, images[..., 3:4].to(dtype=torch.float32)], dim=-1)
    except _passthrough_error_types():
        raise
    except Exception as error:
        _empty_cache()
        raise RetroTapeGpuError(f"The RetroTape CUDA path failed: {error}") from error
    return result


def _process_chunk(
    images: torch.Tensor,
    result: torch.Tensor,
    start: int,
    size: int,
    settings: dict[str, Any],
    frame_offset: int,
    target: torch.device,
    progress: _FrameProgress,
) -> None:
    source = images[start : start + size]
    if source.shape[-1] == 1:
        source = source.expand(-1, -1, -1, 3)
    uploaded = source[..., :3].to(device=target, dtype=torch.float32)
    # The pipeline is authored on 0..255 integer levels, not on the float range.
    uploaded = (uploaded.clamp(0.0, 1.0) * 255.0).round()

    rendered = torch.empty((size, images.shape[1], images.shape[2], 3), dtype=torch.float32, device=target)
    for offset in range(size):
        ntsc_core._raise_if_interrupted()
        rendered[offset] = _process_frame(uploaded[offset], settings, start + offset + frame_offset)
        progress.reached(start + offset)
    result[start : start + size] = (rendered / 255.0).to(result.device)
