"""The in-node preview route: load a source, render it, encode it for the panel."""

from __future__ import annotations

import asyncio
import io
import logging
import math
import os
import sys
from collections import OrderedDict
from concurrent.futures import ThreadPoolExecutor
from threading import RLock
from typing import NamedTuple

import cv2
import numpy as np
from PIL import Image, ImageOps

from .device import DEFAULT_DEVICE, DEVICE_CHOICES, gpu_fallback, gpu_lock, ntsc_gpu, resolve_device
from .ntsc_core import preset_catalog, process_rgb_frames
from .preview_cache import CACHE, MAX_CACHED_FRAMES, SOURCE_KIND, CachedFrames, get_preview_entry

try:
    from aiohttp import web
except Exception:
    web = None

logger = logging.getLogger(__name__)

ROUTE_PATH = "/retrotape/preview"
CLEAR_CACHE_ROUTE_PATH = "/retrotape/cache/clear"
PRESETS_ROUTE_PATH = "/retrotape/presets"

QUICK_PREVIEW_MAX_SIDE = 640
FULL_PREVIEW_MAX_SIDE = 900
MIN_PREVIEW_SIDE = 240
MAX_PREVIEW_SIDE = 1280
MAX_PREVIEW_FRAMES = MAX_CACHED_FRAMES

# Grain is not scale invariant, so the render runs at source size; this caps the cost.
FULL_PREVIEW_PIXEL_BUDGET = 3_000_000
ANIMATED_PREVIEW_TOTAL_BUDGET = 14_000_000
ANIMATED_PREVIEW_MIN_BUDGET = 400_000
MAX_RENDER_SIDE = 4096

# Denied and absent report the same thing, so the route cannot probe the filesystem.
_SOURCE_UNAVAILABLE = (
    "Preview source is unavailable. It must sit inside ComfyUI's input, output or temp "
    "folder, or in a folder listed in RETROTAPE_EXTRA_PREVIEW_ROOTS."
)
_SOURCE_MISSING = "Connect Load Image/Video or run the node once before requesting a preview."
_RENDER_FAILED = "Preview rendering failed."

MAX_SOURCE_PIXELS = 50_000_000

MAX_SELECT_EVERY_NTH = 64
MAX_FRAME_LOAD_CAP = 100_000
MAX_SEQUENTIAL_VIDEO_READS = 4096
MAX_VIDEO_FRAME_COUNT = 21_600_000

MAX_NODE_ID_LENGTH = 128
MAX_FRAME_OFFSET = 1_000_000

EXTRA_ROOTS_ENV = "RETROTAPE_EXTRA_PREVIEW_ROOTS"

_PREVIEW_EXECUTOR = ThreadPoolExecutor(max_workers=1, thread_name_prefix="retrotape-preview")
_PREVIEW_SEMAPHORE = None
_SEMAPHORE_LOOP = None

_REQUEST_SEQUENCE: OrderedDict[str, int] = OrderedDict()
_SEQUENCE_LOCK = RLock()


class PreviewSourceDenied(Exception):
    """A named source could not be read: outside the allowed folders, absent, or
    undecodable, and the three are deliberately indistinguishable to the client."""


class PreviewSourceMissing(Exception):
    """No source was named and no frames are cached for this node."""


def _coerce_int(value, default: int, low: int, high: int) -> int:
    """Clamp a client-supplied number; anything unusable falls back to default."""
    if value is None:
        return default
    try:
        number = float(value)
    except (TypeError, ValueError, OverflowError):
        return default
    if not math.isfinite(number):
        return default
    return int(max(low, min(high, number)))


def _coerce_float(value, default: float, low: float, high: float) -> float:
    """Clamp a client-supplied number; anything unusable falls back to default."""
    if value is None:
        return default
    try:
        number = float(value)
    except (TypeError, ValueError, OverflowError):
        return default
    if not math.isfinite(number):
        return default
    return float(max(low, min(high, number)))


def _get_prompt_server_class():
    server_module = sys.modules.get("server")
    prompt_server = getattr(server_module, "PromptServer", None)
    if prompt_server is not None:
        return prompt_server

    try:
        from server import PromptServer
    except Exception:
        return None
    return PromptServer


def _get_preview_semaphore():
    global _PREVIEW_SEMAPHORE, _SEMAPHORE_LOOP
    loop = asyncio.get_running_loop()
    if _PREVIEW_SEMAPHORE is None or _SEMAPHORE_LOOP is not loop:
        _PREVIEW_SEMAPHORE = asyncio.Semaphore(1)
        _SEMAPHORE_LOOP = loop
    return _PREVIEW_SEMAPHORE


_MAX_SEQUENCE_KEYS = 256


def _next_sequence(key: str) -> int:
    with _SEQUENCE_LOCK:
        value = _REQUEST_SEQUENCE.pop(key, 0) + 1
        _REQUEST_SEQUENCE[key] = value
        while len(_REQUEST_SEQUENCE) > _MAX_SEQUENCE_KEYS:
            _REQUEST_SEQUENCE.pop(next(iter(_REQUEST_SEQUENCE)))
        return value


def _is_latest_sequence(key: str, token: int) -> bool:
    with _SEQUENCE_LOCK:
        return _REQUEST_SEQUENCE.get(key, 0) == token


def _allowed_roots() -> list[str]:
    """Folders a preview request is permitted to read from."""
    roots: list[str] = []
    try:
        import folder_paths
    except Exception:
        folder_paths = None

    if folder_paths is not None:
        for getter in ("get_input_directory", "get_output_directory", "get_temp_directory"):
            func = getattr(folder_paths, getter, None)
            if func is None:
                continue
            try:
                directory = func()
            except Exception:
                continue
            if directory:
                roots.append(os.path.realpath(str(directory)))

    extra = os.environ.get(EXTRA_ROOTS_ENV, "")
    for entry in extra.split(os.pathsep):
        entry = entry.strip()
        if entry:
            roots.append(os.path.realpath(entry))
    return roots


def _within_allowed_roots(resolved: str) -> bool:
    for root in _allowed_roots():
        try:
            if os.path.commonpath([root, resolved]) == root:
                return True
        except ValueError:
            # Different drives on Windows cannot be compared; treat as outside.
            continue
    return False


def _resize_rgb_frame(frame: np.ndarray, max_side: int | None) -> np.ndarray:
    if not max_side:
        return frame
    height, width = frame.shape[:2]
    longest = max(height, width)
    if longest <= 0:
        raise ValueError("Source frame is empty.")
    scale = min(1.0, float(max_side) / float(longest))
    if scale >= 1.0:
        return frame
    new_size = (max(1, int(round(width * scale))), max(1, int(round(height * scale))))
    return cv2.resize(frame, new_size, interpolation=cv2.INTER_AREA)


def _fit_to_pixel_budget(frame: np.ndarray, budget: int | None) -> np.ndarray:
    """Reduce a frame until it fits a pixel count, keeping its aspect ratio."""
    if not budget:
        return frame
    height, width = frame.shape[:2]
    pixels = float(height) * float(width)
    if pixels <= 0:
        raise ValueError("Source frame is empty.")
    scale = min(1.0, math.sqrt(float(budget) / pixels), MAX_RENDER_SIDE / float(max(height, width)))
    if scale >= 1.0:
        return frame
    new_size = (max(1, int(round(width * scale))), max(1, int(round(height * scale))))
    return cv2.resize(frame, new_size, interpolation=cv2.INTER_AREA)


def _source_stat_key(path: str) -> tuple[str, int, int]:
    stat = os.stat(path)
    return os.path.abspath(path), int(stat.st_mtime_ns), int(stat.st_size)


# PIL keeps the low byte of these, which is 127 levels from what LoadImage gives.
_SIXTEEN_BIT_MODES = ("I;16", "I;16B", "I;16L", "I;16N")


def _to_eight_bit_rgb(image: Image.Image) -> Image.Image:
    # Mode "I" says nothing about the depth it holds, so check the values.
    if image.mode not in _SIXTEEN_BIT_MODES and image.mode != "I":
        return image.convert("RGB")
    raw = np.asarray(image)
    if raw.dtype.kind not in "iu":
        return image.convert("RGB")
    if image.mode == "I" and int(raw.max(initial=0)) <= 255:
        return image.convert("RGB")
    scaled = np.rint(raw.astype(np.float32) / 257.0).clip(0, 255).astype(np.uint8)
    return Image.fromarray(scaled).convert("RGB")


def _load_source_image(source_image):
    if not isinstance(source_image, dict):
        return None

    filename = source_image.get("filename")
    if not filename or not isinstance(filename, str):
        return None

    try:
        import folder_paths
    except Exception:
        return None

    image_type = source_image.get("type") or "input"
    default_dir = folder_paths.get_directory_by_type(image_type) or folder_paths.get_input_directory()
    image_path = folder_paths.get_annotated_filepath(filename, default_dir=default_dir)

    resolved = os.path.realpath(os.path.abspath(image_path))
    allowed_root = os.path.realpath(os.path.abspath(default_dir))
    try:
        inside_default = os.path.commonpath([allowed_root, resolved]) == allowed_root
    except ValueError:
        inside_default = False
    if not inside_default and not _within_allowed_roots(resolved):
        raise PreviewSourceDenied(_SOURCE_UNAVAILABLE)
    if not os.path.isfile(resolved):
        raise FileNotFoundError(_SOURCE_UNAVAILABLE)

    decode_max_side = _coerce_int(source_image.get("decode_max_side") or None, 0, 0, MAX_RENDER_SIDE)
    decode_max_pixels = _coerce_int(
        source_image.get("decode_max_pixels") or None, 0, 0, MAX_SOURCE_PIXELS
    )
    cache_key = ("image", *_source_stat_key(resolved), decode_max_side, decode_max_pixels)
    cached = CACHE.get(cache_key)
    if cached is not None:
        return cached

    with Image.open(resolved) as image:
        width, height = image.size
        if width * height > MAX_SOURCE_PIXELS:
            raise ValueError(f"Source image is too large to preview ({width}x{height}).")
        try:
            image.seek(0)
        except EOFError:
            pass
        rgb = _to_eight_bit_rgb(ImageOps.exif_transpose(image))
        frame = np.asarray(rgb.copy(), dtype=np.uint8)
    # After the transpose, so an EXIF-rotated picture reports what the graph sees.
    source_size = (int(frame.shape[1]), int(frame.shape[0]))
    frame = _fit_to_pixel_budget(_resize_rgb_frame(frame, decode_max_side), decode_max_pixels)
    stored = CACHE.put(cache_key, frame, None, source_size, kind=SOURCE_KIND)
    return CachedFrames(stored, None, source_size)


def _resolve_source_path(source, *, default_type="input"):
    if not isinstance(source, dict):
        return None

    filename = source.get("filename")
    if not filename or not isinstance(filename, str):
        return None

    if bool(source.get("path")):
        resolved = os.path.realpath(os.path.abspath(filename))
        if not _within_allowed_roots(resolved):
            raise PreviewSourceDenied(_SOURCE_UNAVAILABLE)
        if not os.path.isfile(resolved):
            raise FileNotFoundError(_SOURCE_UNAVAILABLE)
        return resolved

    try:
        import folder_paths
    except Exception:
        return None

    source_type = source.get("type") or default_type
    default_dir = folder_paths.get_directory_by_type(source_type) or folder_paths.get_input_directory()
    source_path = folder_paths.get_annotated_filepath(filename, default_dir=default_dir)

    resolved = os.path.realpath(os.path.abspath(source_path))
    allowed_root = os.path.realpath(os.path.abspath(default_dir))
    try:
        inside_default = os.path.commonpath([allowed_root, resolved]) == allowed_root
    except ValueError:
        inside_default = False
    if not inside_default and not _within_allowed_roots(resolved):
        raise PreviewSourceDenied(_SOURCE_UNAVAILABLE)
    if not os.path.isfile(resolved):
        raise FileNotFoundError(_SOURCE_UNAVAILABLE)
    return resolved


def _planned_video_frames(total: int, start_index: int, every: int, max_frames: int) -> int:
    """How many frames this clip will actually yield, so the decode budget is
    sized from that and not from the frames the client asked to be allowed."""
    if total <= 0:
        return max_frames
    remaining = max(0, total - start_index)
    return min(max_frames, max(1, -(-remaining // max(1, every))))


def _load_source_video(source_video):
    """The sampled frames, with each one's position in the stream the Load Video
    node will hand the graph."""
    path = _resolve_source_path(source_video)
    if path is None:
        return None

    max_frames = _coerce_int(source_video.get("max_frames") or None, MAX_PREVIEW_FRAMES, 1, MAX_PREVIEW_FRAMES)
    frame_load_cap = _coerce_int(source_video.get("frame_load_cap") or None, 0, 0, MAX_FRAME_LOAD_CAP)
    if frame_load_cap > 0:
        max_frames = min(max_frames, frame_load_cap)
    skip = _coerce_int(source_video.get("skip_first_frames") or None, 0, 0, MAX_FRAME_LOAD_CAP)
    every = _coerce_int(source_video.get("select_every_nth") or None, 1, 1, MAX_SELECT_EVERY_NTH)
    start_time = _coerce_float(source_video.get("start_time") or None, 0.0, 0.0, 86_400.0)
    decode_max_side = _coerce_int(source_video.get("decode_max_side") or None, 0, 0, MAX_RENDER_SIDE)
    decode_max_pixels = _coerce_int(
        source_video.get("decode_max_pixels") or None, 0, 0, MAX_SOURCE_PIXELS
    )
    budget_mode = source_video.get("decode_budget_mode")
    if budget_mode not in ("fast", "full"):
        budget_mode = None
    cache_key = (
        "video",
        *_source_stat_key(path),
        max_frames,
        frame_load_cap,
        skip,
        every,
        round(start_time, 3),
        decode_max_side,
        decode_max_pixels,
        budget_mode,
    )
    cached = CACHE.get(cache_key)
    if cached is not None:
        return cached

    source_size: list[int] = [0, 0]

    def keep(frame_bgr):
        frame_rgb = cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2RGB)
        source_size[0] = int(frame_rgb.shape[1])
        source_size[1] = int(frame_rgb.shape[0])
        return _fit_to_pixel_budget(_resize_rgb_frame(frame_rgb, decode_max_side), decode_max_pixels)

    capture = cv2.VideoCapture(path)
    if not capture.isOpened():
        raise ValueError("Could not open source video.")
    try:
        total = _coerce_int(capture.get(cv2.CAP_PROP_FRAME_COUNT), 0, 0, MAX_VIDEO_FRAME_COUNT)
        if start_time > 0.0:
            capture.set(cv2.CAP_PROP_POS_MSEC, start_time * 1000.0)
            start_index = int(capture.get(cv2.CAP_PROP_POS_FRAMES) or 0)
        elif skip:
            capture.set(cv2.CAP_PROP_POS_FRAMES, skip)
            start_index = skip
        else:
            start_index = 0
        if budget_mode is not None:
            decode_max_pixels = _pixel_budget(
                budget_mode, _planned_video_frames(total, start_index, every, max_frames)
            )
        frames = []
        stream_indices: list[int] = []
        if total > 0 and frame_load_cap <= 0:
            candidates = -(-max(0, total - start_index) // every)
            if candidates == 0:
                raise ValueError("Source video has no readable frames.")
            sample_indices = np.arange(candidates, dtype=np.int64)
            if candidates > max_frames:
                sample_indices = np.linspace(0, candidates - 1, max_frames).round().astype(np.int64)
            candidate_positions = start_index + sample_indices * every
            for rank, position in zip(sample_indices, candidate_positions):
                capture.set(cv2.CAP_PROP_POS_FRAMES, int(position))
                ok, frame_bgr = capture.read()
                if ok:
                    frames.append(keep(frame_bgr))
                    stream_indices.append(int(rank))
            if not frames:
                raise ValueError("Source video has no readable frames.")
            indices = np.asarray(stream_indices, dtype=np.int64)
            size = (source_size[0], source_size[1])
            return CachedFrames(
                CACHE.put(cache_key, np.stack(frames, axis=0), indices, size, kind=SOURCE_KIND),
                indices,
                size,
            )

        index = start_index
        # Bounded by reads too, or a large select_every_nth walks the whole file.
        reads = 0
        while len(frames) < max_frames and reads < MAX_SEQUENTIAL_VIDEO_READS:
            ok, frame_bgr = capture.read()
            if not ok:
                break
            reads += 1
            if (index - start_index) % every == 0:
                frames.append(keep(frame_bgr))
                stream_indices.append(len(stream_indices))
            index += 1
        if not frames:
            raise ValueError("Source video has no readable frames.")
        indices = np.asarray(stream_indices, dtype=np.int64)
        size = (source_size[0], source_size[1])
        return CachedFrames(
            CACHE.put(cache_key, np.stack(frames, axis=0), indices, size, kind=SOURCE_KIND),
            indices,
            size,
        )
    finally:
        capture.release()


def _as_frame_batch(frames):
    array = np.asarray(frames)
    if array.ndim == 3:
        array = array[None, ...]
    if array.ndim != 4 or array.shape[-1] < 3:
        raise ValueError("Preview source must be an RGB frame or frame batch.")
    if array.shape[0] == 0:
        raise ValueError("Preview source has no frames.")
    return np.ascontiguousarray(array[..., :3])


class PreviewResult(NamedTuple):
    body: bytes
    content_type: str
    frame_count: int
    render_width: int
    render_height: int
    source_width: int
    source_height: int
    # "cpu" or "cuda": the two paths do not draw the same grain.
    device: str = "cpu"


def _pixel_budget(preview_mode: str, frame_count: int) -> int:
    if preview_mode == "fast":
        return QUICK_PREVIEW_MAX_SIDE * QUICK_PREVIEW_MAX_SIDE
    if frame_count <= 1:
        return FULL_PREVIEW_PIXEL_BUDGET
    per_frame = ANIMATED_PREVIEW_TOTAL_BUDGET // max(1, frame_count)
    return int(max(ANIMATED_PREVIEW_MIN_BUDGET, min(FULL_PREVIEW_PIXEL_BUDGET, per_frame)))


def _encode_preview(
    frames,
    settings_json,
    *,
    preview_mode="full",
    max_side=FULL_PREVIEW_MAX_SIDE,
    frame_indices=None,
    frame_offset=0,
    source_size=None,
    device=DEFAULT_DEVICE,
):
    batch = _as_frame_batch(frames)
    indices = (
        np.arange(batch.shape[0], dtype=np.int64)
        if frame_indices is None
        else np.asarray(frame_indices, dtype=np.int64)[: batch.shape[0]]
    )
    if indices.shape[0] != batch.shape[0]:
        indices = np.arange(batch.shape[0], dtype=np.int64)

    if preview_mode == "fast":
        batch = batch[:1]
        indices = indices[:1]
        max_side = min(int(max_side), QUICK_PREVIEW_MAX_SIDE)
    else:
        max_side = int(max(MIN_PREVIEW_SIDE, min(MAX_PREVIEW_SIDE, max_side)))
        if batch.shape[0] > MAX_PREVIEW_FRAMES:
            keep = np.linspace(0, batch.shape[0] - 1, MAX_PREVIEW_FRAMES).round().astype(np.int64)
            batch = batch[keep]
            indices = indices[keep]

    source_width, source_height = (
        (int(batch.shape[2]), int(batch.shape[1])) if not source_size else (int(source_size[0]), int(source_size[1]))
    )
    budget = _pixel_budget(preview_mode, int(batch.shape[0]))
    render_frames = [_fit_to_pixel_budget(frame, budget) for frame in batch]
    render_height, render_width = render_frames[0].shape[:2]
    target = resolve_device(device)
    render_indices = [int(index) + int(frame_offset) for index in indices]
    rendered = None
    if target == "cuda" and ntsc_gpu is not None:
        try:
            with gpu_lock:
                rendered = [
                    ntsc_gpu.process_rgb_uint8(frame, settings_json, frame_index=number)
                    for frame, number in zip(render_frames, render_indices)
                ]
        except Exception as error:
            if not gpu_fallback(error, "preview"):
                raise
            rendered = None
            target = "cpu"
    if rendered is None:
        target = "cpu"
        rendered = process_rgb_frames(render_frames, settings_json, render_indices)
    processed = [_resize_rgb_frame(frame, max_side) for frame in rendered]

    def result(body: bytes, content_type: str, count: int) -> PreviewResult:
        return PreviewResult(
            body,
            content_type,
            count,
            int(render_width),
            int(render_height),
            int(source_width),
            int(source_height),
            target,
        )

    if len(processed) <= 1:
        buffer = io.BytesIO()
        Image.fromarray(processed[0]).save(buffer, format="PNG", optimize=False)
        return result(buffer.getvalue(), "image/png", 1)

    pil_frames = [Image.fromarray(frame) for frame in processed]
    duration = 83
    buffer = io.BytesIO()
    try:
        pil_frames[0].save(
            buffer,
            format="WEBP",
            save_all=True,
            append_images=pil_frames[1:],
            duration=duration,
            loop=0,
            # Lossy WebP subsamples chroma, which is where this effect lives.
            lossless=True,
            quality=20,
            method=0,
        )
        return result(buffer.getvalue(), "image/webp", len(pil_frames))
    except Exception:
        logger.info("[RetroTape] WebP preview encoding failed, falling back to GIF.", exc_info=True)
        buffer = io.BytesIO()
        pil_frames[0].save(
            buffer,
            format="GIF",
            save_all=True,
            append_images=pil_frames[1:],
            duration=duration,
            loop=0,
        )
        return result(buffer.getvalue(), "image/gif", len(pil_frames))


def render_preview(
    settings_json,
    preview_mode,
    max_side,
    node_id,
    source_video,
    source_image,
    frame_offset=0,
    device=DEFAULT_DEVICE,
):
    """Load the source frames and render them. Runs on a worker thread."""
    frames = None
    frame_indices = None
    source_size = None
    try:
        if source_video is not None:
            if isinstance(source_video, dict):
                source_video = {
                    **source_video,
                    "decode_max_side": MAX_RENDER_SIDE,
                    "decode_max_pixels": 0,
                    "decode_budget_mode": preview_mode,
                }
            loaded = _load_source_video(source_video)
            if loaded is not None:
                frames, frame_indices, source_size = loaded
        if frames is None and source_image is not None:
            if isinstance(source_image, dict):
                source_image = {
                    **source_image,
                    "decode_max_side": MAX_RENDER_SIDE,
                    "decode_max_pixels": _pixel_budget(preview_mode, 1),
                }
            loaded = _load_source_image(source_image)
            if loaded is not None:
                frames, frame_indices, source_size = loaded
    except PreviewSourceDenied:
        raise
    except (FileNotFoundError, ValueError, OSError, Image.DecompressionBombError) as exc:
        logger.info("[RetroTape] Preview source could not be read: %s", exc)
        raise PreviewSourceDenied(_SOURCE_UNAVAILABLE) from None

    if frames is None and node_id is not None:
        cached = get_preview_entry(node_id)
        if cached is not None:
            frames, frame_indices, source_size = cached
    if frames is None:
        raise PreviewSourceMissing(_SOURCE_MISSING)

    return _encode_preview(
        frames,
        settings_json,
        preview_mode=preview_mode,
        max_side=max_side,
        frame_indices=frame_indices,
        frame_offset=frame_offset,
        source_size=source_size,
        device=device,
    )


def register_routes() -> bool:
    """Attach the preview route to the running PromptServer."""
    if web is None:
        logger.warning("[RetroTape] aiohttp unavailable; preview route not registered.")
        return False

    prompt_server = _get_prompt_server_class()
    if prompt_server is None:
        logger.warning("[RetroTape] PromptServer unavailable; preview route not registered.")
        return False

    instance = getattr(prompt_server, "instance", None)
    if instance is None or not hasattr(instance, "routes"):
        logger.warning("[RetroTape] PromptServer.instance unavailable; preview route not registered.")
        return False

    if getattr(instance, "_retrotape_routes_registered", False):
        return True

    router = getattr(getattr(instance, "app", None), "router", None)
    if router is not None and getattr(router, "frozen", False):
        logger.warning("[RetroTape] ComfyUI is already serving; restart it to enable the preview route.")
        return False

    def fail(message, status, **extra):
        return web.json_response(
            {"error": message, **extra}, status=status, headers={"Cache-Control": "no-store"}
        )

    async def preview(request):
        try:
            payload = await request.json()
        except Exception:
            return fail("Invalid JSON payload.", 400)
        if not isinstance(payload, dict):
            return fail("Invalid JSON payload.", 400)

        preview_mode = "fast" if payload.get("preview_mode") == "fast" else "full"
        default_side = QUICK_PREVIEW_MAX_SIDE if preview_mode == "fast" else FULL_PREVIEW_MAX_SIDE
        try:
            max_side = int(payload.get("max_side") or default_side)
        except (TypeError, ValueError, OverflowError):
            return fail("max_side must be a number.", 400)
        max_side = int(max(MIN_PREVIEW_SIDE, min(MAX_PREVIEW_SIDE, max_side)))

        frame_offset = payload.get("frame_offset")
        if frame_offset is None:
            frame_offset = 0
        elif isinstance(frame_offset, bool) or not isinstance(frame_offset, (int, float)):
            return fail("frame_offset must be a number.", 400)
        elif not math.isfinite(float(frame_offset)):
            return fail("frame_offset must be a number.", 400)
        else:
            frame_offset = int(max(0, min(MAX_FRAME_OFFSET, float(frame_offset))))

        requested_device = payload.get("device")
        if requested_device is None:
            requested_device = DEFAULT_DEVICE
        elif not isinstance(requested_device, str):
            return fail("device must be a string.", 400)
        elif requested_device not in DEVICE_CHOICES:
            return fail("device must be one of %s." % ", ".join(DEVICE_CHOICES), 400)

        node_id = payload.get("node_id")
        if node_id is not None and len(str(node_id)) > MAX_NODE_ID_LENGTH:
            return fail("node_id is too long.", 400)
        source_video = payload.get("source_video")
        source_image = payload.get("source_image")
        if node_id is None and source_video is None and source_image is None:
            return fail("node_id, source_image, or source_video is required.", 400)

        sequence_key = str(node_id) if node_id is not None else "__anonymous__"
        token = _next_sequence(sequence_key)

        semaphore = _get_preview_semaphore()
        async with semaphore:
            if not _is_latest_sequence(sequence_key, token):
                return fail("Superseded by a newer request.", 409)
            transport = request.transport
            if transport is None or transport.is_closing():
                return fail("Client went away.", 409)

            loop = asyncio.get_running_loop()
            try:
                rendered = await loop.run_in_executor(
                    _PREVIEW_EXECUTOR,
                    render_preview,
                    payload.get("settings_json"),
                    preview_mode,
                    max_side,
                    node_id,
                    source_video,
                    source_image,
                    frame_offset,
                    requested_device,
                )
            except PreviewSourceDenied:
                return fail(_SOURCE_UNAVAILABLE, 404, reason="source_unavailable")
            except PreviewSourceMissing:
                return fail(_SOURCE_MISSING, 404, reason="no_frames")
            except Exception:
                logger.exception("[RetroTape] Preview rendering failed.")
                return fail(_RENDER_FAILED, 500)

        return web.Response(
            body=rendered.body,
            content_type=rendered.content_type,
            headers={
                "Cache-Control": "no-store",
                "X-RetroTape-Frames": str(rendered.frame_count),
                "X-RetroTape-Render-Size": "%dx%d" % (rendered.render_width, rendered.render_height),
                "X-RetroTape-Source-Size": "%dx%d" % (rendered.source_width, rendered.source_height),
                "X-RetroTape-Device": str(rendered.device),
            },
        )

    async def clear_cache(request):
        payload = {}
        if request.can_read_body:
            try:
                payload = await request.json()
            except Exception:
                return fail("Invalid JSON payload.", 400)
            if payload is None:
                payload = {}
        if not isinstance(payload, dict):
            return fail("Invalid JSON payload.", 400)
        sources_only = payload.get("sources_only", False)
        if not isinstance(sources_only, bool):
            return fail("sources_only must be true or false.", 400)
        dropped = CACHE.clear(SOURCE_KIND if sources_only else None)
        return web.json_response(dropped, headers={"Cache-Control": "no-store"})

    async def presets(request):
        return web.json_response(preset_catalog(), headers={"Cache-Control": "no-store"})

    instance.routes.get(PRESETS_ROUTE_PATH)(presets)
    instance.routes.post(ROUTE_PATH)(preview)
    instance.routes.post(CLEAR_CACHE_ROUTE_PATH)(clear_cache)
    instance._retrotape_routes_registered = True

    logger.info("[RetroTape] Preview route registered: %s", ROUTE_PATH)
    return True
