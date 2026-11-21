"""What the preview renders from: a node's input batch, and decoded source files."""

from __future__ import annotations

import math
from collections import OrderedDict
from threading import RLock
from typing import Any, NamedTuple

import cv2
import numpy as np

# The still-image render budget in server.py.
MAX_CACHED_SIDE = 2560
MAX_CACHED_FRAMES = 24

MAX_CACHE_BYTES = 320 * 1024 * 1024
MAX_CACHE_ITEMS = 32
MAX_NODE_ENTRY_BYTES = MAX_CACHE_BYTES // 2

NODE_KIND = "node"
SOURCE_KIND = "source"


class CachedFrames(NamedTuple):
    frames: np.ndarray
    indices: np.ndarray | None
    source_size: tuple[int, int] | None


class ByteBoundedCache:
    """An LRU of frame batches bounded by the bytes they hold, not by item count alone.

    Node frames outrank decoded sources: a source can be re-read from disk, node
    frames cannot be had again without a graph run."""

    def __init__(self, max_bytes: int, max_items: int) -> None:
        self.max_bytes = max_bytes
        self.max_items = max_items
        self._entries: OrderedDict[Any, tuple[str, CachedFrames]] = OrderedDict()
        self._bytes = 0
        self._lock = RLock()

    def __len__(self) -> int:
        with self._lock:
            return len(self._entries)

    @property
    def total_bytes(self) -> int:
        with self._lock:
            return self._bytes

    def get(self, key) -> CachedFrames | None:
        """Frames, frame numbers and source size, read in one acquisition."""
        with self._lock:
            entry = self._entries.get(key)
            if entry is None:
                return None
            self._entries.move_to_end(key)
            return entry[1]

    def put(self, key, frames: np.ndarray, indices=None, source_size=None, *, kind=NODE_KIND) -> np.ndarray:
        stored = np.ascontiguousarray(frames).view()
        stored.flags.writeable = False
        if isinstance(indices, np.ndarray):
            indices = indices.view()
            indices.flags.writeable = False
        entry = CachedFrames(stored, indices, source_size)
        with self._lock:
            self._drop(key)
            self._entries[key] = (kind, entry)
            self._bytes += int(stored.nbytes)
            self._evict(key, kind)
        return stored

    def _drop(self, key) -> None:
        previous = self._entries.pop(key, None)
        if previous is not None:
            self._bytes -= int(previous[1].frames.nbytes)

    def _evict(self, new_key, new_kind: str) -> None:
        while len(self._entries) > self.max_items or self._bytes > self.max_bytes:
            victim = next(
                (k for k, (kind, _) in self._entries.items() if kind == SOURCE_KIND and k != new_key),
                None,
            )
            if victim is None and new_kind == NODE_KIND:
                victim = next((k for k in self._entries if k != new_key), None)
            if victim is None:
                if new_kind == SOURCE_KIND and len(self._entries) > 1:
                    self._drop(new_key)
                return
            self._drop(victim)

    def clear(self, kind: str | None = None) -> dict[str, int]:
        """Drop everything, or one kind, and report what went."""
        with self._lock:
            freed = 0
            dropped = {NODE_KIND: 0, SOURCE_KIND: 0}
            doomed = [k for k, (k_kind, _) in self._entries.items() if kind is None or k_kind == kind]
            for key in doomed:
                entry_kind, entry = self._entries.pop(key)
                freed += int(entry.frames.nbytes)
                dropped[entry_kind] = dropped.get(entry_kind, 0) + 1
            self._bytes -= freed
            return {
                "freed_bytes": freed,
                "node_entries": dropped[NODE_KIND],
                "source_entries": dropped[SOURCE_KIND],
            }


CACHE = ByteBoundedCache(MAX_CACHE_BYTES, MAX_CACHE_ITEMS)


def _sample_indices(count: int) -> np.ndarray:
    return np.linspace(0, count - 1, MAX_CACHED_FRAMES).round().astype(np.int64)


def _entry_side(count: int, height: int, width: int) -> int:
    longest = max(1, max(height, width))
    side = min(MAX_CACHED_SIDE, longest)
    pixels = float(height) * float(width) / float(longest * longest)
    budget = MAX_NODE_ENTRY_BYTES / max(1, count) / 3.0
    if pixels * side * side > budget:
        side = int(max(16, math.sqrt(budget / max(1e-9, pixels))))
    return side


def _downscale(frame: np.ndarray, side: int) -> np.ndarray:
    height, width = frame.shape[:2]
    longest = max(height, width)
    if longest <= side or longest == 0:
        return frame
    scale = side / float(longest)
    size = (max(1, int(round(width * scale))), max(1, int(round(height * scale))))
    return cv2.resize(frame, size, interpolation=cv2.INTER_AREA)


def _to_rgb_uint8_batch(images) -> tuple[np.ndarray, np.ndarray, tuple[int, int]]:
    array = images
    if hasattr(array, "detach"):
        import torch

        array = array.detach().cpu()
        if array.ndim == 3:
            array = array[None, ...]
        # Subsample before converting dtype, or a long batch copies to discard.
        indices = np.arange(int(array.shape[0]), dtype=np.int64)
        if array.shape[0] > MAX_CACHED_FRAMES:
            indices = _sample_indices(int(array.shape[0]))
            array = array[torch.as_tensor(indices)]
        array = array.numpy() if array.dtype == torch.uint8 else array.float().numpy()
    else:
        array = np.asarray(array)
        if array.ndim == 3:
            array = array[None, ...]
        indices = np.arange(int(array.shape[0]), dtype=np.int64)
        if array.shape[0] > MAX_CACHED_FRAMES:
            indices = _sample_indices(int(array.shape[0]))
            array = array[indices]

    if array.ndim != 4 or array.shape[-1] < 1:
        raise ValueError("Preview frames must be NHWC images.")
    if array.shape[-1] == 1:
        array = np.repeat(array, 3, axis=-1)
    elif array.shape[-1] < 3:
        raise ValueError("Preview frames must carry 1, 3 or 4 channels.")
    array = array[..., :3]

    if array.dtype != np.uint8:
        array = np.clip(array, 0.0, 1.0)
        array = (array * 255.0).round().astype(np.uint8)
    else:
        # Never alias a buffer the caller still owns and may write to later.
        array = array.copy()

    source_size = (int(array.shape[2]), int(array.shape[1]))
    if array.shape[0] == 0:
        return np.ascontiguousarray(array), indices, source_size
    side = _entry_side(int(array.shape[0]), int(array.shape[1]), int(array.shape[2]))
    frames = [_downscale(np.ascontiguousarray(frame), side) for frame in array]
    return np.ascontiguousarray(np.stack(frames, axis=0)), indices, source_size


def set_preview_frames(node_id, images) -> None:
    """Cache a node's input batch for the preview route."""
    frames, indices, source_size = _to_rgb_uint8_batch(images)
    CACHE.put(str(node_id), frames, indices, source_size, kind=NODE_KIND)


def get_preview_entry(node_id):
    """Frames, frame numbers and true source size, read in one acquisition."""
    return CACHE.get(str(node_id))
