from __future__ import annotations

import logging

from . import ntsc_core
from .device import DEVICE_CHOICES, gpu_fallback, gpu_lock, ntsc_gpu, resolve_device
from .preview_cache import set_preview_frames

logger = logging.getLogger(__name__)


def _rewind(progress) -> None:
    reset = getattr(progress, "update_absolute", None)
    if reset is not None:
        try:
            reset(0)
        except Exception:
            pass


class RetroTape:
    @classmethod
    def INPUT_TYPES(cls):
        return {
            "required": {
                "image": ("IMAGE",),
                "settings_json": (
                    "STRING",
                    {
                        "default": ntsc_core.DEFAULT_SETTINGS_JSON,
                        "multiline": True,
                        "dynamicPrompts": False,
                    },
                ),
            },
            "hidden": {
                "unique_id": "UNIQUE_ID",
            },
            "optional": {
                "frame_offset": (
                    "INT",
                    {
                        "default": 0,
                        "min": 0,
                        "max": 1_000_000,
                        "step": 1,
                        "tooltip": (
                            "Where this batch starts in a longer clip. Tape artefacts follow the "
                            "frame number, so a clip rendered in several runs needs each run's "
                            "start here or the artefacts restart and the seam shows."
                        ),
                    },
                ),
                "device": (
                    list(DEVICE_CHOICES),
                    {
                        "default": "auto",
                        "tooltip": (
                            "Where the effect runs. auto uses the GPU when CUDA is available and "
                            "the CPU otherwise. The two paths give the same look and put the tape "
                            "damage in the same places for a given seed, but the grain is drawn "
                            "from a different generator, so a seed does not reproduce the same "
                            "grain pattern across devices. The preview follows this setting."
                        ),
                    },
                ),
            },
        }

    RETURN_TYPES = ("IMAGE",)
    RETURN_NAMES = ("image",)
    FUNCTION = "apply"
    CATEGORY = "RetroTape"
    # An output node, so it runs and feeds the panel with nothing downstream.
    OUTPUT_NODE = True
    DESCRIPTION = (
        "NTSC / VHS analog tape degradation for images and frame batches, with a live preview "
        "inside the node. The preview renders immediately when the image input comes straight "
        "from a Load Image or Load Video node; any other graph needs one run first. A still "
        "inside the preview budget renders at its own resolution, so the preview is the output; "
        "above it the panel says which size it fell back to."
    )

    def apply(self, image, settings_json="", unique_id=None, frame_offset=0, device="auto"):
        shape = getattr(image, "shape", ())
        frame_count = 1 if len(shape) == 3 else int(shape[0]) if shape else 0
        if unique_id is not None and frame_count > 0:
            try:
                set_preview_frames(unique_id, image)
            except Exception:
                logger.exception("[RetroTape] Could not cache preview frames.")

        target = resolve_device(device)
        # One bar for the whole node, so a fallback mid-batch does not restart it.
        progress = ntsc_core._make_progress_bar(frame_count)
        rendered = None
        if target == "cuda":
            try:
                with gpu_lock:
                    rendered = ntsc_gpu.process_tensor_batch(
                        image, settings_json, frame_offset, progress=progress
                    )
            except Exception as error:
                if not gpu_fallback(error, "batch"):
                    raise
                _rewind(progress)
        if rendered is None:
            rendered = ntsc_core.process_tensor_batch(
                image, settings_json, frame_offset, progress=progress
            )

        return {
            "ui": {
                "retrotape_preview_ready": [str(unique_id or ""), str(frame_count)],
            },
            "result": (rendered,),
        }


NODE_CLASS_MAPPINGS = {
    "RetroTape": RetroTape,
}

NODE_DISPLAY_NAME_MAPPINGS = {
    "RetroTape": "Retro Tape (VHS / NTSC)",
}
