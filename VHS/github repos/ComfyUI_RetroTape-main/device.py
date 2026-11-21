"""Device choice and the CUDA fallback shared by the node and the preview route."""

from __future__ import annotations

import logging
from threading import RLock

logger = logging.getLogger(__name__)

try:
    from . import ntsc_gpu
except Exception:  # pragma: no cover - only on a torch build that cannot import
    ntsc_gpu = None
    logger.info("[RetroTape] CUDA path unavailable; CPU only.", exc_info=True)

DEVICE_CHOICES = ("auto", "cpu", "gpu")
DEFAULT_DEVICE = "auto"

# The node's execution thread and the preview worker would otherwise overlap.
gpu_lock = RLock()

# A CUDA context that hit one of these is unusable for the rest of the process.
_POISON_MARKERS = ("device-side assert", "illegal memory access", "unspecified launch failure")

_WARNED_NO_CUDA = False
_WARNED_FALLBACK = False
_GPU_UNUSABLE = False


def resolve_device(choice) -> str:
    """Turn the user's choice into "cpu" or "cuda"."""
    global _WARNED_NO_CUDA

    normalized = str(choice or DEFAULT_DEVICE).strip().lower()
    if normalized == "cpu":
        return "cpu"

    available = not _GPU_UNUSABLE and bool(ntsc_gpu is not None and ntsc_gpu.is_available())
    if normalized in ("gpu", "cuda"):
        if available:
            return "cuda"
        if not _WARNED_NO_CUDA:
            _WARNED_NO_CUDA = True
            logger.warning(
                "[RetroTape] GPU was asked for but no CUDA device is available; rendering on the CPU."
            )
        return "cpu"
    return "cuda" if available else "cpu"


def _cuda_error_types() -> tuple[type[BaseException], ...]:
    try:
        import torch
    except Exception:
        return ()
    error = getattr(torch, "OutOfMemoryError", None) or getattr(
        getattr(torch, "cuda", None), "OutOfMemoryError", None
    )
    return (error,) if isinstance(error, type) else ()


def gpu_fallback(error: BaseException, what: str) -> bool:
    """True when this error is a CUDA failure the caller should re-run on the CPU.

    ntsc_gpu wraps most faults in RetroTapeGpuError, but its outer allocations
    sit outside that guard, so the raw torch errors have to be caught here too.
    """
    global _WARNED_FALLBACK, _GPU_UNUSABLE

    message = str(error)
    recognised = (
        (ntsc_gpu is not None and isinstance(error, ntsc_gpu.RetroTapeGpuError))
        or isinstance(error, _cuda_error_types())
        or (isinstance(error, RuntimeError) and "cuda" in message.lower())
    )
    if not recognised:
        return False

    if any(marker in message.lower() for marker in _POISON_MARKERS):
        _GPU_UNUSABLE = True

    if _WARNED_FALLBACK:
        logger.debug("[RetroTape] CUDA fallback for the %s.", what, exc_info=True)
    else:
        _WARNED_FALLBACK = True
        logger.warning(
            "[RetroTape] The CUDA path could not render this %s; falling back to the CPU.",
            what,
            exc_info=True,
        )
    return True
