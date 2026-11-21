from __future__ import annotations

import json
import math
import os
from concurrent.futures import FIRST_COMPLETED, ThreadPoolExecutor, wait
from functools import lru_cache
from threading import Lock
from typing import Any

import cv2
import numpy as np
import torch
from scipy.signal import lfilter

# Adapted for ComfyUI from ntscQT by JargeZ (Apache-2.0).
#   https://github.com/JargeZ/ntscqt
# Carried over: the NTSC sample rate, the one-pole per-line lowpass and its
# cutoffs, the YIQ conversion, the parameter names, and the four presets in
# PRESETS, which are its builtin_templates.json verbatim. Everything else,
# including CUSTOM_PRESETS, is this project's.

NTSC_RATE = (315000000.0 / 88.0) * 4.0
DEFAULT_PRESET = "RGM"

# Every spatial constant below is authored against this and scaled to the frame.
REFERENCE_WIDTH = 720.0
REFERENCE_HEIGHT = 480.0

# Slow tape mechanics are held this long, so they drift rather than flicker.
TEMPORAL_EPOCH_FRAMES = 4

# chroma_loss is clamped to 5000, so only the first 7.7% of this is reachable.
CHROMA_LOSS_DIVISOR = 65000.0

# Black level a fully lifted picture sits on, out of 255: about 18% grey.
BLACK_LIFT_PEDESTAL = 46.0

_RINGING_TAPS = np.array([0.16, -0.3, 0.42, 1.0, -0.42, 0.3, -0.16], dtype=np.float32)

_LUMA_ROW = np.array([[0.299, 0.587, 0.114]], dtype=np.float32)

_GLOW_CHANNEL_GAIN = np.array(
    [1.12 * 0.55 + 0.08, 0.86 * 0.55, 0.62 * 0.55 - 0.04], dtype=np.float32
)

# Copied verbatim from ntscQT's builtin_templates.json, authors' handles and all.
PRESETS: dict[str, dict[str, Any]] = {
    "yobelkcip": {
        "_composite_preemphasis": 0.0,
        "_vhs_out_sharpen": 1,
        "_vhs_edge_wave": 2,
        "_ringing": 0.9375,
        "_ringing_power": 1,
        "_ringing_shift": 0.9375,
        "_freq_noise_size": 0.88806,
        "_freq_noise_amplitude": 1,
        "_color_bleed_horiz": 2,
        "_color_bleed_vert": 0,
        "_video_chroma_noise": 12335,
        "_video_chroma_phase_noise": 6,
        "_video_chroma_loss": 0,
        "_video_noise": 3907,
        "_video_scanline_phase_shift": 43,
        "_video_scanline_phase_shift_offset": 2,
        "_head_switching_speed": 37,
        "_vhs_head_switching": False,
        "_color_bleed_before": False,
        "_enable_ringing2": True,
        "_composite_in_chroma_lowpass": True,
        "_composite_out_chroma_lowpass": True,
        "_composite_out_chroma_lowpass_lite": True,
        "_emulating_vhs": True,
        "_nocolor_subcarrier": False,
        "_vhs_chroma_vert_blend": True,
        "_vhs_svideo_out": True,
        "_output_ntsc": True,
    },
    "Master Nama": {
        "_composite_preemphasis": 0.0492,
        "_vhs_out_sharpen": 2,
        "_vhs_edge_wave": 1,
        "_ringing": 0.05523,
        "_ringing_power": 1,
        "_ringing_shift": 0.37425,
        "_freq_noise_size": 0.7906,
        "_freq_noise_amplitude": 0,
        "_color_bleed_horiz": 0,
        "_color_bleed_vert": 0,
        "_video_chroma_noise": 949,
        "_video_chroma_phase_noise": 6,
        "_video_chroma_loss": 0,
        "_video_noise": 1114,
        "_video_scanline_phase_shift": 90,
        "_video_scanline_phase_shift_offset": 1,
        "_head_switching_speed": 0,
        "_vhs_head_switching": False,
        "_color_bleed_before": True,
        "_enable_ringing2": True,
        "_composite_in_chroma_lowpass": True,
        "_composite_out_chroma_lowpass": True,
        "_composite_out_chroma_lowpass_lite": True,
        "_emulating_vhs": True,
        "_nocolor_subcarrier": False,
        "_vhs_chroma_vert_blend": True,
        "_vhs_svideo_out": True,
        "_output_ntsc": True,
    },
    "RGM": {
        "_composite_preemphasis": 0.9434,
        "_vhs_out_sharpen": 1,
        "_vhs_edge_wave": 0,
        "_ringing": 0.40541,
        "_ringing_power": 1,
        "_ringing_shift": 0.0,
        "_freq_noise_size": 0.0,
        "_freq_noise_amplitude": 2,
        "_color_bleed_horiz": 0,
        "_color_bleed_vert": 0,
        "_video_chroma_noise": 6892,
        "_video_chroma_phase_noise": 4,
        "_video_chroma_loss": 65,
        "_video_noise": 2743,
        "_video_scanline_phase_shift": 90,
        "_video_scanline_phase_shift_offset": 0,
        "_head_switching_speed": 0,
        "_vhs_head_switching": True,
        "_color_bleed_before": True,
        "_enable_ringing2": True,
        "_composite_in_chroma_lowpass": True,
        "_composite_out_chroma_lowpass": True,
        "_composite_out_chroma_lowpass_lite": False,
        "_emulating_vhs": True,
        "_nocolor_subcarrier": False,
        "_vhs_chroma_vert_blend": True,
        "_vhs_svideo_out": True,
        "_output_ntsc": True,
    },
    "JaneLon": {
        "_composite_preemphasis": 0.5344,
        "_vhs_out_sharpen": 2,
        "_vhs_edge_wave": 1,
        "_ringing": 0.51042,
        "_ringing_power": 0,
        "_ringing_shift": 0.0,
        "_freq_noise_size": 0.7906,
        "_freq_noise_amplitude": 0,
        "_color_bleed_horiz": 1,
        "_color_bleed_vert": 2,
        "_video_chroma_noise": 15591,
        "_video_chroma_phase_noise": 2,
        "_video_chroma_loss": 0,
        "_video_noise": 4163,
        "_video_scanline_phase_shift": 90,
        "_video_scanline_phase_shift_offset": 1,
        "_head_switching_speed": 0,
        "_vhs_head_switching": False,
        "_color_bleed_before": True,
        "_enable_ringing2": True,
        "_composite_in_chroma_lowpass": True,
        "_composite_out_chroma_lowpass": True,
        "_composite_out_chroma_lowpass_lite": True,
        "_emulating_vhs": True,
        "_nocolor_subcarrier": False,
        "_vhs_chroma_vert_blend": True,
        "_vhs_svideo_out": False,
        "_output_ntsc": True,
    },
}

CUSTOM_PRESETS: dict[str, dict[str, Any]] = {
    "Sun Bleached": {
        "preset": 'Sun Bleached',
        "blend": 1.0,
        "seed": 0,
        "video_noise": 1800,
        "chroma_noise": 5200,
        "chroma_phase_noise": 16,
        "chroma_loss": 700,
        "color_bleed_h": 5,
        "color_bleed_v": 2,
        "ringing": 0.45,
        "ringing_power": 3,
        "vhs_enabled": True,
        "vhs_edge_wave": 2,
        "head_switching": False,
        "scanline_phase": 90,
        "scanline_offset": 1,
        "chroma_delay": 6,
        "tracking_jitter": 1.1,
        "tape_warp": 2.0,
        "dropouts": 0.05,
        "ghosting": 0.12,
        "scanlines": 0.26,
        "color_drift": 0.14,
        "luma_smear": 0.34,
        "interlace": 0.14,
        "vertical_roll": 0.05,
        "analog_glow": 0.44,
        "saturation": 0.38,
        "black_lift": 0.58,
    },
    "First Gen": {
        "preset": "First Gen",
        "blend": 1.0,
        "seed": 0,
        "video_noise": 100,
        "chroma_noise": 300,
        "chroma_phase_noise": 1,
        "chroma_loss": 0,
        "color_bleed_h": 0,
        "color_bleed_v": 0,
        "ringing": 0.14,
        "ringing_power": 2,
        "vhs_enabled": True,
        "vhs_edge_wave": 0,
        "head_switching": False,
        "scanline_phase": 90,
        "scanline_offset": 0,
        "chroma_delay": 0,
        "tracking_jitter": 0.0,
        "tape_warp": 0.1,
        "dropouts": 0.0,
        "ghosting": 0.02,
        "scanlines": 0.03,
        "color_drift": 0.0,
        "luma_smear": 0.02,
        "interlace": 0.02,
        "vertical_roll": 0.0,
        "analog_glow": 0.02,
        "saturation": 0.96,
        "black_lift": 0.07,
    },
    "Home Movie 1987": {
        "preset": "Home Movie 1987",
        "blend": 1.0,
        "seed": 0,
        "video_noise": 2200,
        "chroma_noise": 4200,
        "chroma_phase_noise": 8,
        "chroma_loss": 40,
        "color_bleed_h": 4,
        "color_bleed_v": 2,
        "ringing": 0.42,
        "ringing_power": 3,
        "vhs_enabled": True,
        "vhs_edge_wave": 3,
        "head_switching": True,
        "scanline_phase": 90,
        "scanline_offset": 1,
        "chroma_delay": 5,
        "tracking_jitter": 2.6,
        "tape_warp": 3.55,
        "dropouts": 0.14,
        "ghosting": 0.1,
        "scanlines": 0.16,
        "color_drift": 0.2,
        "luma_smear": 0.52,
        "interlace": 0.12,
        "vertical_roll": 0.06,
        "analog_glow": 0.55,
        "saturation": 1.18,
        "black_lift": 0.2,
    },
    "Rental Copy": {
        "preset": "Rental Copy",
        "blend": 1.0,
        "seed": 0,
        "video_noise": 4000,
        "chroma_noise": 9000,
        "chroma_phase_noise": 40,
        "chroma_loss": 2400,
        "color_bleed_h": 8,
        "color_bleed_v": 3,
        "ringing": 0.74,
        "ringing_power": 5,
        "vhs_enabled": True,
        "vhs_edge_wave": 5,
        "head_switching": True,
        "scanline_phase": 90,
        "scanline_offset": 2,
        "chroma_delay": 11,
        "tracking_jitter": 4.6,
        "tape_warp": 6.45,
        "dropouts": 0.52,
        "ghosting": 0.4,
        "scanlines": 0.36,
        "color_drift": 0.44,
        "luma_smear": 0.3,
        "interlace": 0.46,
        "vertical_roll": 0.2,
        "analog_glow": 0.2,
        "saturation": 0.72,
        "black_lift": 0.38,
    },
    "Film Transfer": {
        "preset": "Film Transfer",
        "blend": 1.0,
        "seed": 0,
        "video_noise": 2000,
        "chroma_noise": 4200,
        "chroma_phase_noise": 6,
        "chroma_loss": 60,
        "color_bleed_h": 2,
        "color_bleed_v": 0,
        "ringing": 0.7,
        "ringing_power": 4,
        "vhs_enabled": False,
        "vhs_edge_wave": 0,
        "head_switching": False,
        "scanline_phase": 90,
        "scanline_offset": 0,
        "chroma_delay": 2,
        "tracking_jitter": 0.2,
        "tape_warp": 0.2,
        "dropouts": 0.03,
        "ghosting": 0.16,
        "scanlines": 0.12,
        "color_drift": 0.08,
        "luma_smear": 0.16,
        "interlace": 0.06,
        "vertical_roll": 0.0,
        "analog_glow": 0.5,
        "saturation": 0.82,
        "black_lift": 0.24,
    },
    "Late Night Broadcast": {
        "preset": "Late Night Broadcast",
        "blend": 1.0,
        "seed": 0,
        "video_noise": 4200,
        "chroma_noise": 11000,
        "chroma_phase_noise": 26,
        "chroma_loss": 420,
        "color_bleed_h": 5,
        "color_bleed_v": 1,
        "ringing": 0.86,
        "ringing_power": 6,
        "vhs_enabled": False,
        "vhs_edge_wave": 0,
        "head_switching": False,
        "scanline_phase": 180,
        "scanline_offset": 0,
        "chroma_delay": 7,
        "tracking_jitter": 0.6,
        "tape_warp": 0.3,
        "dropouts": 0.05,
        "ghosting": 0.68,
        "scanlines": 0.46,
        "color_drift": 0.22,
        "luma_smear": 0.3,
        "interlace": 0.34,
        "vertical_roll": 0.1,
        "analog_glow": 0.42,
        "saturation": 0.94,
        "black_lift": 0.14,
    },
    "Broadcast Dirty": {
        "preset": "Broadcast Dirty",
        "blend": 1.0,
        "seed": 0,
        "video_noise": 5200,
        "chroma_noise": 13000,
        "chroma_phase_noise": 64,
        "chroma_loss": 1600,
        "color_bleed_h": 3,
        "color_bleed_v": 1,
        "ringing": 0.95,
        "ringing_power": 8,
        "vhs_enabled": False,
        "vhs_edge_wave": 0,
        "head_switching": False,
        "scanline_phase": 90,
        "scanline_offset": 0,
        "chroma_delay": 4,
        "tracking_jitter": 0.4,
        "tape_warp": 0.2,
        "dropouts": 0.04,
        "ghosting": 0.12,
        "scanlines": 0.42,
        "color_drift": 0.3,
        "luma_smear": 0.08,
        "interlace": 0.6,
        "vertical_roll": 0.0,
        "analog_glow": 0.0,
        "saturation": 1.0,
        "black_lift": 0.1,
    },
    "Weather Channel": {
        "preset": "Weather Channel",
        "blend": 1.0,
        "seed": 0,
        "video_noise": 900,
        "chroma_noise": 1400,
        "chroma_phase_noise": 2,
        "chroma_loss": 0,
        "color_bleed_h": 1,
        "color_bleed_v": 0,
        "ringing": 0.62,
        "ringing_power": 2,
        "vhs_enabled": False,
        "vhs_edge_wave": 0,
        "head_switching": False,
        "scanline_phase": 90,
        "scanline_offset": 0,
        "chroma_delay": 1,
        "tracking_jitter": 0.1,
        "tape_warp": 0.1,
        "dropouts": 0.0,
        "ghosting": 0.05,
        "scanlines": 0.38,
        "color_drift": 0.02,
        "luma_smear": 0.04,
        "interlace": 0.26,
        "vertical_roll": 0.0,
        "analog_glow": 0.12,
        "saturation": 1.14,
        "black_lift": 0.06,
    },
    "Security Cam": {
        "preset": "Security Cam",
        "blend": 1.0,
        "seed": 0,
        "video_noise": 6400,
        "chroma_noise": 4000,
        "chroma_phase_noise": 8,
        "chroma_loss": 1200,
        "color_bleed_h": 1,
        "color_bleed_v": 0,
        "ringing": 0.9,
        "ringing_power": 7,
        "vhs_enabled": True,
        "vhs_edge_wave": 1,
        "head_switching": False,
        "scanline_phase": 90,
        "scanline_offset": 0,
        "chroma_delay": 2,
        "tracking_jitter": 0.8,
        "tape_warp": 0.95,
        "dropouts": 0.22,
        "ghosting": 0.1,
        "scanlines": 0.72,
        "color_drift": 0.05,
        "luma_smear": 0.12,
        "interlace": 0.55,
        "vertical_roll": 0.02,
        "analog_glow": 0.06,
        "saturation": 0.12,
        "black_lift": 0.3,
    },
    "Kinescope": {
        "preset": "Kinescope",
        "blend": 1.0,
        "seed": 0,
        "video_noise": 3400,
        "chroma_noise": 600,
        "chroma_phase_noise": 0,
        "chroma_loss": 0,
        "color_bleed_h": 0,
        "color_bleed_v": 0,
        "ringing": 0.3,
        "ringing_power": 3,
        "vhs_enabled": False,
        "vhs_edge_wave": 0,
        "head_switching": False,
        "scanline_phase": 90,
        "scanline_offset": 0,
        "chroma_delay": 0,
        "tracking_jitter": 0.3,
        "tape_warp": 0.4,
        "dropouts": 0.08,
        "ghosting": 0.2,
        "scanlines": 0.18,
        "color_drift": 0.0,
        "luma_smear": 0.24,
        "interlace": 0.08,
        "vertical_roll": 0.0,
        "analog_glow": 0.6,
        "saturation": 0.0,
        "black_lift": 0.48,
    },
    "AE Punch": {
        "preset": "AE Punch",
        "blend": 1.0,
        "seed": 0,
        "video_noise": 3000,
        "chroma_noise": 28000,
        "chroma_phase_noise": 18,
        "chroma_loss": 0,
        "color_bleed_h": 12,
        "color_bleed_v": 4,
        "ringing": 0.98,
        "ringing_power": 6,
        "vhs_enabled": True,
        "vhs_edge_wave": 1,
        "head_switching": False,
        "scanline_phase": 90,
        "scanline_offset": 1,
        "chroma_delay": 14,
        "tracking_jitter": 0.5,
        "tape_warp": 0.75,
        "dropouts": 0.05,
        "ghosting": 0.5,
        "scanlines": 0.24,
        "color_drift": 0.05,
        "luma_smear": 0.0,
        "interlace": 0.1,
        "vertical_roll": 0.0,
        "analog_glow": 0.0,
        "saturation": 1.35,
        "black_lift": 0.0,
    },
    "Chrome Dream": {
        "preset": "Chrome Dream",
        "blend": 1.0,
        "seed": 0,
        "video_noise": 1400,
        "chroma_noise": 20000,
        "chroma_phase_noise": 44,
        "chroma_loss": 0,
        "color_bleed_h": 12,
        "color_bleed_v": 4,
        "ringing": 0.6,
        "ringing_power": 3,
        "vhs_enabled": True,
        "vhs_edge_wave": 1,
        "head_switching": False,
        "scanline_phase": 45,
        "scanline_offset": 1,
        "chroma_delay": 16,
        "tracking_jitter": 0.5,
        "tape_warp": 1.25,
        "dropouts": 0.02,
        "ghosting": 0.3,
        "scanlines": 0.2,
        "color_drift": 0.55,
        "luma_smear": 0.4,
        "interlace": 0.12,
        "vertical_roll": 0.0,
        "analog_glow": 0.72,
        "saturation": 1.7,
        "black_lift": 0.0,
    },
    "Bootleg Anime": {
        "preset": "Bootleg Anime",
        "blend": 1.0,
        "seed": 0,
        "video_noise": 1500,
        "chroma_noise": 6000,
        "chroma_phase_noise": 10,
        "chroma_loss": 300,
        "color_bleed_h": 6,
        "color_bleed_v": 1,
        "ringing": 1.0,
        "ringing_power": 2,
        "vhs_enabled": True,
        "vhs_edge_wave": 2,
        "head_switching": True,
        "scanline_phase": 90,
        "scanline_offset": 0,
        "chroma_delay": 9,
        "tracking_jitter": 1.6,
        "tape_warp": 2.1,
        "dropouts": 0.1,
        "ghosting": 0.16,
        "scanlines": 0.36,
        "color_drift": 0.12,
        "luma_smear": 0.1,
        "interlace": 0.34,
        "vertical_roll": 0.04,
        "analog_glow": 0.2,
        "saturation": 1.42,
        "black_lift": 0.32,
    },
    "Damaged Tape": {
        "preset": "Damaged Tape",
        "blend": 1.0,
        "seed": 0,
        "video_noise": 7600,
        "chroma_noise": 26000,
        "chroma_phase_noise": 58,
        "chroma_loss": 1800,
        "color_bleed_h": 10,
        "color_bleed_v": 3,
        "ringing": 0.92,
        "ringing_power": 8,
        "vhs_enabled": True,
        "vhs_edge_wave": 7,
        "head_switching": True,
        "scanline_phase": 132,
        "scanline_offset": 2,
        "chroma_delay": 13,
        "tracking_jitter": 7.0,
        "tape_warp": 9.65,
        "dropouts": 0.58,
        "ghosting": 0.5,
        "scanlines": 0.62,
        "color_drift": 0.62,
        "luma_smear": 0.58,
        "interlace": 0.72,
        "vertical_roll": 0.42,
        "analog_glow": 0.34,
        "saturation": 0.68,
        "black_lift": 0.3,
    },
    "Tracking Hell": {
        "preset": "Tracking Hell",
        "blend": 1.0,
        "seed": 0,
        "video_noise": 6200,
        "chroma_noise": 22000,
        "chroma_phase_noise": 42,
        "chroma_loss": 1000,
        "color_bleed_h": 8,
        "color_bleed_v": 2,
        "ringing": 0.84,
        "ringing_power": 7,
        "vhs_enabled": True,
        "vhs_edge_wave": 10,
        "head_switching": True,
        "scanline_phase": 43,
        "scanline_offset": 2,
        "chroma_delay": 10,
        "tracking_jitter": 10.0,
        "tape_warp": 13.5,
        "dropouts": 0.4,
        "ghosting": 0.4,
        "scanlines": 0.55,
        "color_drift": 0.48,
        "luma_smear": 0.44,
        "interlace": 0.62,
        "vertical_roll": 0.7,
        "analog_glow": 0.24,
        "saturation": 0.86,
        "black_lift": 0.18,
    },
}

EXTRA_SETTINGS_DEFAULTS: dict[str, Any] = {
    "color_drift": 0.0,
    "luma_smear": 0.0,
    "interlace": 0.0,
    "vertical_roll": 0.0,
    "analog_glow": 0.0,
    "saturation": 1.0,
    "black_lift": 0.0,
}

BUILTIN_PRESET_OVERRIDES: dict[str, dict[str, Any]] = {
    "RGM": {
        "video_noise": 3900,
        "chroma_noise": 9800,
        "chroma_phase_noise": 14,
        "chroma_loss": 180,
        "color_bleed_h": 2,
        "ringing": 0.72,
        "ringing_power": 4,
        "vhs_edge_wave": 2,
        "chroma_delay": 4,
        "tracking_jitter": 2.2,
        "tape_warp": 2.5,
        "dropouts": 0.14,
        "ghosting": 0.2,
        "scanlines": 0.34,
        "color_drift": 0.18,
        "luma_smear": 0.2,
        "interlace": 0.24,
        "vertical_roll": 0.05,
        "analog_glow": 0.16,
    },
    "Master Nama": {
        "video_noise": 1900,
        "chroma_noise": 2200,
        "chroma_phase_noise": 10,
        "ringing": 0.82,
        "ringing_power": 4,
        "chroma_delay": 1,
        "tracking_jitter": 0.7,
        "tape_warp": 1.35,
        "dropouts": 0.03,
        "ghosting": 0.12,
        "scanlines": 0.24,
        "color_drift": 0.06,
        "luma_smear": 0.1,
        "interlace": 0.16,
        "vertical_roll": 0.02,
        "analog_glow": 0.08,
    },
    "JaneLon": {
        "video_noise": 5400,
        "chroma_noise": 22000,
        "chroma_phase_noise": 18,
        "chroma_loss": 260,
        "color_bleed_h": 4,
        "ringing": 0.64,
        "ringing_power": 4,
        "vhs_edge_wave": 3,
        "chroma_delay": 7,
        "tracking_jitter": 2.8,
        "tape_warp": 3.95,
        "dropouts": 0.18,
        "ghosting": 0.34,
        "scanlines": 0.4,
        "color_drift": 0.28,
        "luma_smear": 0.3,
        "interlace": 0.32,
        "vertical_roll": 0.08,
        "analog_glow": 0.16,
    },
    "yobelkcip": {
        "video_noise": 5800,
        "chroma_noise": 18000,
        "chroma_phase_noise": 24,
        "chroma_loss": 380,
        "color_bleed_h": 5,
        "ringing": 0.38,
        "ringing_power": 5,
        "vhs_edge_wave": 5,
        "chroma_delay": 8,
        "tracking_jitter": 4.2,
        "tape_warp": 6.25,
        "dropouts": 0.22,
        "ghosting": 0.28,
        "scanlines": 0.46,
        "color_drift": 0.34,
        "luma_smear": 0.34,
        "interlace": 0.42,
        "vertical_roll": 0.12,
        "analog_glow": 0.18,
    },
}


def _compact_from_preset(name: str) -> dict[str, Any]:
    if name in CUSTOM_PRESETS:
        return {**EXTRA_SETTINGS_DEFAULTS, **CUSTOM_PRESETS[name]}

    preset = PRESETS.get(name, PRESETS[DEFAULT_PRESET])
    settings = {
        **EXTRA_SETTINGS_DEFAULTS,
        "preset": name if name in PRESETS else DEFAULT_PRESET,
        "blend": 1.0,
        "seed": 0,
        "video_noise": int(preset["_video_noise"]),
        "chroma_noise": int(preset["_video_chroma_noise"]),
        "chroma_phase_noise": int(preset["_video_chroma_phase_noise"]),
        "chroma_loss": int(preset["_video_chroma_loss"]),
        "color_bleed_h": int(preset["_color_bleed_horiz"]),
        "color_bleed_v": int(preset["_color_bleed_vert"]),
        "ringing": float(np.clip(1.0 - float(preset["_ringing"]), 0.0, 1.0)),
        "ringing_power": max(1, int(preset["_ringing_power"]) + 1),
        "vhs_enabled": bool(preset["_emulating_vhs"]),
        "vhs_edge_wave": int(preset["_vhs_edge_wave"]),
        "head_switching": bool(preset["_vhs_head_switching"]),
        "scanline_phase": int(preset["_video_scanline_phase_shift"]),
        "scanline_offset": int(preset["_video_scanline_phase_shift_offset"]),
        "chroma_delay": int(preset["_color_bleed_horiz"]) * 2,
        "tracking_jitter": float(preset["_vhs_edge_wave"]) * 1.25 + (1.2 if preset["_vhs_head_switching"] else 0.0),
        "tape_warp": float(preset["_vhs_edge_wave"]) * 1.60,
        "dropouts": float(np.clip(int(preset["_video_noise"]) / 9000.0 * 0.2, 0.0, 0.45)),
        "ghosting": float(np.clip((1.0 - float(preset["_ringing"])) * 0.28, 0.0, 0.45)),
        "scanlines": 0.28 if preset["_emulating_vhs"] else 0.12,
    }
    settings.update(BUILTIN_PRESET_OVERRIDES.get(name, {}))
    return settings


DEFAULT_SETTINGS = _compact_from_preset(DEFAULT_PRESET)
DEFAULT_SETTINGS_JSON = json.dumps(DEFAULT_SETTINGS, separators=(",", ":"))


def _clamp_float(value: Any, default: float, low: float, high: float) -> float:
    try:
        number = float(value)
    except (TypeError, ValueError, OverflowError):
        number = default
    if not math.isfinite(number):
        number = default
    return float(np.clip(number, low, high))


def _clamp_int(value: Any, default: int, low: int, high: int) -> int:
    try:
        number = float(value)
    except (TypeError, ValueError, OverflowError):
        number = default
    if not math.isfinite(number):
        number = default
    try:
        number = int(number)
    except (TypeError, ValueError, OverflowError):
        number = default
    return int(np.clip(number, low, high))


_FLOAT_RANGES: dict[str, tuple[float, float]] = {
    "blend": (0.0, 1.0),
    "saturation": (0.0, 2.0),
    "black_lift": (0.0, 1.0),
    "ringing": (0.0, 1.0),
    "tracking_jitter": (0.0, 12.0),
    "tape_warp": (0.0, 18.0),
    "dropouts": (0.0, 1.0),
    "ghosting": (0.0, 1.0),
    "scanlines": (0.0, 1.0),
    "color_drift": (0.0, 1.0),
    "luma_smear": (0.0, 1.0),
    "interlace": (0.0, 1.0),
    "vertical_roll": (0.0, 1.0),
    "analog_glow": (0.0, 1.0),
}

_INT_RANGES: dict[str, tuple[int, int]] = {
    "seed": (0, 2**31 - 1),
    "video_noise": (0, 9000),
    "chroma_noise": (0, 32768),
    "chroma_phase_noise": (0, 120),
    "chroma_loss": (0, 5000),
    "color_bleed_h": (0, 24),
    "color_bleed_v": (0, 12),
    "ringing_power": (1, 12),
    "vhs_edge_wave": (0, 12),
    "scanline_phase": (0, 270),
    "scanline_offset": (0, 3),
    "chroma_delay": (0, 24),
}

_BOOL_KEYS = ("vhs_enabled", "head_switching")


def preset_catalog() -> dict[str, Any]:
    """Resolved presets and control bounds, for the browser panel to build from."""
    return {
        "default_preset": DEFAULT_PRESET,
        "presets": {name: _compact_from_preset(name) for name in (*PRESETS, *CUSTOM_PRESETS)},
        "ranges": {key: list(bounds) for key, bounds in {**_FLOAT_RANGES, **_INT_RANGES}.items()},
        "bool_keys": list(_BOOL_KEYS),
    }


def parse_settings_json(settings_json: str | dict[str, Any] | None) -> dict[str, Any]:
    if isinstance(settings_json, dict):
        raw = settings_json
    else:
        try:
            # Rejected per key below, so one bad number costs only its own control.
            raw = json.loads(settings_json or "{}")
        except (TypeError, ValueError, RecursionError):
            raw = {}
    if not isinstance(raw, dict):
        raw = {}

    preset_name = raw.get("preset", DEFAULT_PRESET)
    if not isinstance(preset_name, str) or (preset_name not in PRESETS and preset_name not in CUSTOM_PRESETS):
        preset_name = DEFAULT_PRESET

    settings = _compact_from_preset(preset_name)
    preset_defaults = dict(settings)
    settings.update({key: value for key, value in raw.items() if key in settings})
    settings["preset"] = preset_name

    # A missing value falls back to the preset's, not to the bottom of the range.
    for key, (low, high) in _FLOAT_RANGES.items():
        fallback = _clamp_float(preset_defaults.get(key), low, low, high)
        settings[key] = _clamp_float(settings.get(key), fallback, low, high)
    for key, (low, high) in _INT_RANGES.items():
        fallback = _clamp_int(preset_defaults.get(key), low, low, high)
        settings[key] = _clamp_int(settings.get(key), fallback, low, high)
    for key in _BOOL_KEYS:
        value = settings.get(key)
        settings[key] = bool(preset_defaults.get(key)) if value is None else bool(value)
    return settings


def _frame_scale(shape) -> tuple[float, float]:
    """Frame size relative to the 720x480 picture the effect is authored for."""
    height = float(shape[0])
    width = float(shape[1])
    return max(0.05, width / REFERENCE_WIDTH), max(0.05, height / REFERENCE_HEIGHT)


def _scaled_shift(value: float, scale: float, minimum: int = 0) -> int:
    return max(minimum, int(round(float(value) * scale)))


def _resample_kernel(taps: np.ndarray, scale: float) -> np.ndarray:
    """Stretch a 1-D kernel so it spans the same fraction of a scanline."""
    length = int(round(taps.size * scale))
    length = max(3, length + 1 - length % 2)
    if length == taps.size:
        return taps
    source = np.linspace(0.0, 1.0, taps.size, dtype=np.float32)
    target = np.linspace(0.0, 1.0, length, dtype=np.float32)
    resampled = np.interp(target, source, taps).astype(np.float32)
    total = float(resampled.sum())
    if abs(total) > 1e-6:
        resampled *= float(taps.sum()) / total
    return resampled


@lru_cache(maxsize=64)
def _one_pole_cascade(alpha: float, passes: int) -> tuple[np.ndarray, np.ndarray]:
    """`passes` identical one-pole sections collapsed into one filter."""
    numerator = np.array([1.0], dtype=np.float64)
    denominator = np.array([1.0], dtype=np.float64)
    pole = np.array([1.0, -(1.0 - alpha)], dtype=np.float64)
    for _ in range(passes):
        numerator = np.convolve(numerator, [alpha])
        denominator = np.convolve(denominator, pole)
    return numerator, denominator


def _lowpass_rows(rows: np.ndarray, cutoff: float, passes: int = 3, h_scale: float = 1.0) -> np.ndarray:
    if rows.size == 0:
        return rows
    if passes <= 0:
        return rows.astype(np.float32, copy=True)
    tau = 1.0 / (cutoff * 2.0 * math.pi)
    # A pixel is one sample of a 720-wide line, so a wider frame walks it faster.
    interval = 1.0 / (NTSC_RATE * max(0.05, h_scale))
    alpha = interval / (tau + interval)
    numerator, denominator = _one_pole_cascade(alpha, int(passes))
    return lfilter(numerator, denominator, rows, axis=1).astype(np.float32, copy=False)


_RGB_TO_YIQ = np.array(
    [
        [0.299, 0.587, 0.114],
        [0.596, -0.274, -0.322],
        [0.211, -0.523, 0.312],
    ],
    dtype=np.float32,
)


def _rgb_to_yiq(rgb: np.ndarray) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    y, i, q = cv2.split(cv2.transform(rgb.astype(np.float32), _RGB_TO_YIQ))
    return y, i, q


_YIQ_TO_RGB = np.array(
    [
        [1.0, 0.956, 0.621],
        [1.0, -0.272, -0.647],
        [1.0, -1.106, 1.703],
    ],
    dtype=np.float32,
)


def _yiq_to_rgb(y: np.ndarray, i: np.ndarray, q: np.ndarray) -> np.ndarray:
    # Rounded, not truncated: three conversions in series each darken the picture.
    out = cv2.transform(cv2.merge([y, i, q]), _YIQ_TO_RGB)
    np.clip(out, 0, 255, out=out)
    np.rint(out, out=out)
    return out.astype(np.uint8)


def _pad_width_to_multiple_of_four(rgb: np.ndarray) -> tuple[np.ndarray, int]:
    pad = (-rgb.shape[1]) % 4
    if pad == 0:
        return rgb, 0
    return cv2.copyMakeBorder(rgb, 0, 0, 0, pad, cv2.BORDER_REFLECT_101), pad


def _shift_plane(plane: np.ndarray, x_shift: int, y_shift: int) -> np.ndarray:
    if x_shift == 0 and y_shift == 0:
        return plane
    top = max(y_shift, 0)
    bottom = max(-y_shift, 0)
    left = max(x_shift, 0)
    right = max(-x_shift, 0)
    padded = np.pad(plane, ((top, bottom), (left, right)), mode="edge")
    y0 = 0 if y_shift >= 0 else bottom
    x0 = 0 if x_shift >= 0 else right
    return padded[y0 : y0 + plane.shape[0], x0 : x0 + plane.shape[1]].astype(np.float32, copy=False)


def _shift_rows(plane: np.ndarray, shifts: np.ndarray) -> np.ndarray:
    """Slide every row sideways by its own amount, replicating the edge pixel."""
    out = np.empty_like(plane, dtype=np.float32)
    width = plane.shape[1]
    row_shifts = shifts.astype(np.int32, copy=False)
    for row in range(plane.shape[0]):
        shift = int(row_shifts[row])
        if shift == 0:
            out[row] = plane[row]
        elif shift > 0:
            if shift >= width:
                out[row] = plane[row, 0]
                continue
            out[row, :shift] = plane[row, 0]
            out[row, shift:] = plane[row, : width - shift]
        else:
            left = -shift
            if left >= width:
                out[row] = plane[row, width - 1]
                continue
            out[row, : width - left] = plane[row, left:]
            out[row, width - left :] = plane[row, width - 1]
    return out


def _apply_saturation(i: np.ndarray, q: np.ndarray, settings: dict[str, Any]) -> tuple[np.ndarray, np.ndarray]:
    """Chroma gain on the I/Q planes, applied to the signal going onto the tape so
    the deck's own colour snow and the dot crawl still behave at zero gain."""
    gain = settings["saturation"]
    if gain == 1.0:
        return i, q
    return i * gain, q * gain


def _apply_black_lift(y: np.ndarray, settings: dict[str, Any]) -> np.ndarray:
    """Remap the picture into pedestal..255 so the shadows lift and white stays put."""
    amount = settings["black_lift"]
    if amount <= 0.0:
        return y
    pedestal = BLACK_LIFT_PEDESTAL * amount
    return y * (1.0 - pedestal / 255.0) + pedestal


def _apply_color_bleed(i: np.ndarray, q: np.ndarray, h_shift: int, v_shift: int) -> tuple[np.ndarray, np.ndarray]:
    if h_shift == 0 and v_shift == 0:
        return i, q
    return _shift_plane(i, h_shift, v_shift), _shift_plane(q, h_shift, v_shift)


def _apply_chroma_delay(i: np.ndarray, q: np.ndarray, delay: int) -> tuple[np.ndarray, np.ndarray]:
    if delay <= 0:
        return i, q
    i_delayed = _shift_plane(i, delay, 0)
    q_delayed = _shift_plane(q, max(1, delay // 2), 0)
    return i * 0.35 + i_delayed * 0.65, q * 0.45 + q_delayed * 0.55


def _apply_color_drift(
    i: np.ndarray,
    q: np.ndarray,
    settings: dict[str, Any],
    frame_index: int,
) -> tuple[np.ndarray, np.ndarray]:
    amount = settings["color_drift"]
    if amount <= 0.0:
        return i, q

    rows = np.arange(i.shape[0], dtype=np.float32)[:, None]
    base = math.sin(frame_index * 0.19 + settings["seed"] * 0.00011) * amount * 0.36
    v_scale = settings.get("_v_scale", 1.0)
    row_angle = base + np.sin(rows * ((0.013 + amount * 0.007) / v_scale) + frame_index * 0.11) * amount * 0.16
    sin = np.sin(row_angle)
    cos = np.cos(row_angle)
    chroma_gain = np.float32(1.0 + amount * 0.16)
    out_i = i * cos
    out_i -= q * sin
    out_i *= chroma_gain
    out_q = i * sin
    out_q += q * cos
    out_q *= chroma_gain
    return out_i, out_q


def _apply_ringing(y: np.ndarray, settings: dict[str, Any]) -> np.ndarray:
    strength = settings["ringing"]
    if strength <= 0.0:
        return y
    power = settings["ringing_power"]
    h_scale = settings.get("_h_scale", 1.0)
    blur = cv2.GaussianBlur(y, (0, 0), (1.25 + power * 0.15) * h_scale)
    edge = y - blur
    taps = _resample_kernel(_RINGING_TAPS, h_scale)
    ring = cv2.filter2D(edge, -1, taps.reshape(1, -1), borderType=cv2.BORDER_REFLECT101)
    forward = _scaled_shift(max(1, power), h_scale, 1)
    backward = _scaled_shift(max(1, power // 2), h_scale, 1)
    echo = _shift_plane(edge, forward, 0) - _shift_plane(edge, -backward, 0)
    ring *= np.float32(strength * (1.05 + power * 0.22))
    out = y + ring
    echo *= np.float32(strength)
    echo *= np.float32(0.22)
    out += echo
    return out


def _apply_composite_artifacts(
    y: np.ndarray,
    i: np.ndarray,
    q: np.ndarray,
    settings: dict[str, Any],
) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    h, w = y.shape
    x = np.arange(w, dtype=np.float32)[None, :]
    rows = np.arange(h, dtype=np.float32)[:, None]
    row_phase = math.radians(settings["scanline_phase"])
    h_scale = settings.get("_h_scale", 1.0)
    v_scale = settings.get("_v_scale", 1.0)
    carrier = np.sin(((x / h_scale + settings["scanline_offset"]) * (math.pi / 2.0)) + (rows / v_scale) * row_phase)
    carrier *= np.hypot(i, q)
    carrier *= np.float32(0.045 + settings["scanlines"] * 0.065)
    carrier += y
    y = carrier
    i = _lowpass_rows(i, 1_300_000.0, h_scale=h_scale)
    q = _lowpass_rows(q, 600_000.0, h_scale=h_scale)
    return y, i, q


def _apply_noise(
    y: np.ndarray,
    i: np.ndarray,
    q: np.ndarray,
    settings: dict[str, Any],
    rng: np.random.Generator,
) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    if settings["video_noise"] > 0:
        sigma = settings["video_noise"] / 9000.0 * 42.0
        noise = rng.standard_normal(y.shape, dtype=np.float32)
        noise *= np.float32(sigma)
        noise = _lowpass_rows(noise, 2_000_000.0, passes=1, h_scale=settings.get("_h_scale", 1.0))
        fine = rng.standard_normal(y.shape, dtype=np.float32)
        fine *= np.float32(sigma * 0.28)
        noise += fine
        noise += y
        y = noise

    if settings["chroma_noise"] > 0:
        sigma = np.float32(settings["chroma_noise"] / 32768.0 * 82.0)
        chroma_noise = rng.standard_normal(i.shape, dtype=np.float32)
        chroma_noise *= sigma
        chroma_noise += i
        i = chroma_noise
        chroma_noise = rng.standard_normal(q.shape, dtype=np.float32)
        chroma_noise *= sigma
        chroma_noise += q
        q = chroma_noise

    if settings["chroma_phase_noise"] > 0:
        row_noise = rng.normal(
            0.0,
            settings["chroma_phase_noise"] / 120.0 * 0.72,
            size=(i.shape[0], 1),
        ).astype(np.float32)
        sin = np.sin(row_noise)
        cos = np.cos(row_noise)
        i, q = i * cos - q * sin, i * sin + q * cos

    if settings["chroma_loss"] > 0:
        chance = settings["chroma_loss"] / CHROMA_LOSS_DIVISOR
        mask = rng.random((i.shape[0], 1)) < chance
        i = np.where(mask, i * 0.08, i)
        q = np.where(mask, q * 0.08, q)

    return y, i, q


def _apply_luma_smear(y: np.ndarray, settings: dict[str, Any]) -> np.ndarray:
    amount = settings["luma_smear"]
    if amount <= 0.0:
        return y

    length = max(3, int(round((3 + amount * 23) * settings.get("_h_scale", 1.0))))
    length += 1 - length % 2
    decay = np.exp(-np.linspace(0.0, 3.2, length, dtype=np.float32))
    # A tape smear is causal, so the kernel is reversed and the anchor moved.
    kernel = (decay / max(1e-6, float(decay.sum())))[::-1].copy()
    anchor = (length - 1, 0)
    smear = cv2.filter2D(y, -1, kernel.reshape(1, -1), anchor=anchor, borderType=cv2.BORDER_REPLICATE)
    highlights = y - np.float32(154.0)
    np.maximum(highlights, 0.0, out=highlights)
    glow_trail = cv2.filter2D(
        highlights, -1, kernel.reshape(1, -1), anchor=anchor, borderType=cv2.BORDER_REPLICATE
    )
    smear *= np.float32(amount * 0.18)
    out = y * np.float32(1.0 - amount * 0.18)
    out += smear
    glow_trail *= np.float32(amount * 0.22)
    out += glow_trail
    return out


def _reference_lines(height: int, settings: dict[str, Any]) -> np.ndarray:
    """Which 480-line scan line each pixel row belongs to."""
    pitch = max(1.0, float(settings.get("_v_scale", 1.0)))
    return np.floor(np.arange(height, dtype=np.float64) / pitch).astype(np.int64)


def _scale_one_pole(gain: float, pole: float, v_scale: float) -> tuple[float, float]:
    """Re-derive a one-pole row filter for a picture with v_scale times the
    reference's lines, so the wobble keeps its shape at any resolution."""
    scale = max(1e-6, float(v_scale))
    scaled_pole = float(min(abs(pole), 0.999999)) ** (1.0 / scale)
    scaled_gain = gain * math.sqrt(max(1e-12, (1.0 - scaled_pole**2) / (1.0 - pole**2)))
    return scaled_gain, scaled_pole


def _servo_profile(seed: int, epoch: int, height: int, jitter: float, v_scale: float = 1.0) -> np.ndarray:
    """Slow horizontal tracking error for one epoch, as a per-row shift."""
    rng = np.random.default_rng((int(seed) * 2_654_435_761 + int(epoch) * 40_503) % (2**32))
    raw = rng.normal(0.0, jitter * 0.55, size=height).astype(np.float32)
    gain, pole = _scale_one_pole(0.055, 0.945, v_scale)
    return lfilter([gain], [1.0, -pole], raw).astype(np.float32)


def _apply_tracking_warp(
    y: np.ndarray,
    i: np.ndarray,
    q: np.ndarray,
    settings: dict[str, Any],
    rng: np.random.Generator,
    frame_index: int,
) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    jitter = settings["tracking_jitter"]
    warp = settings["tape_warp"]
    if jitter <= 0.0 and warp <= 0.0:
        return y, i, q

    h, w = y.shape
    h_scale = settings.get("_h_scale", 1.0)
    v_scale = settings.get("_v_scale", 1.0)
    rows = np.arange(h, dtype=np.float32) / v_scale
    shifts = np.zeros(h, dtype=np.float32)
    if warp > 0.0:
        phase = frame_index * 0.17 + settings["seed"] * 0.0003
        shifts += np.sin(rows * (0.020 + warp * 0.0015) + phase) * warp * 0.62
        shifts += np.sin(rows * 0.071 + phase * 1.9) * warp * 0.28

    if jitter > 0.0:
        span = max(1, TEMPORAL_EPOCH_FRAMES)
        epoch = frame_index // span
        mix = (frame_index % span) / float(span)
        current = _servo_profile(settings["seed"], epoch, h, jitter, v_scale)
        upcoming = _servo_profile(settings["seed"], epoch + 1, h, jitter, v_scale)
        shifts += current * (1.0 - mix) + upcoming * mix
        tears = int(jitter // 3)
        for _ in range(tears):
            start = int(rng.integers(0, max(1, h)))
            length = int(rng.integers(max(2, h // 80), max(3, h // 18)))
            end = min(h, start + length)
            if end <= start:
                continue
            amount = rng.normal(0.0, jitter * 1.65)
            shifts[start:end] += np.linspace(amount, 0.0, end - start, dtype=np.float32)

    limit = max(1, w // 3)
    shifts = np.clip(np.round(shifts * h_scale), -limit, limit)
    return _shift_rows(y, shifts), _shift_rows(i, shifts), _shift_rows(q, shifts)


def _apply_interlace_roll(
    y: np.ndarray,
    i: np.ndarray,
    q: np.ndarray,
    settings: dict[str, Any],
    frame_index: int,
) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    roll_amount = settings["vertical_roll"]
    if roll_amount > 0.0:
        offset = int(round(math.sin(frame_index * 0.23 + settings["seed"] * 0.00017) * roll_amount * y.shape[0] * 0.045))
        if offset:
            y = np.roll(y, offset, axis=0)
            i = np.roll(i, offset, axis=0)
            q = np.roll(q, offset, axis=0)

    interlace = settings["interlace"]
    if interlace <= 0.0 or y.shape[0] < 2:
        return y, i, q

    # A field is every other 480-line scan line, not every other pixel row.
    odd = np.flatnonzero(((_reference_lines(y.shape[0], settings) + frame_index) % 2) == 1)
    if odd.size == 0:
        return y, i, q
    step = _scaled_shift(1, settings.get("_h_scale", 1.0), 1)

    def comb(plane: np.ndarray, shift: int, keep: float) -> np.ndarray:
        out = plane.copy()
        rows = out[odd]
        shifted = _shift_plane(rows, shift, 0)
        if shifted is rows:
            shifted = rows.copy()
        shifted *= np.float32(interlace * 0.08)
        rows *= np.float32(keep)
        rows += shifted
        out[odd] = rows
        return out

    y_out = comb(y, step if frame_index % 2 == 0 else -step, 1.0 - interlace * 0.22)
    i_out = comb(i, step, 1.0 - interlace * 0.16)
    q_out = comb(q, step, 1.0 - interlace * 0.16)
    return y_out, i_out, q_out


def _apply_dropouts(
    y: np.ndarray,
    i: np.ndarray,
    q: np.ndarray,
    settings: dict[str, Any],
    rng: np.random.Generator,
) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    strength = settings["dropouts"]
    if strength <= 0.0:
        return y, i, q

    h, w = y.shape
    h_scale = settings.get("_h_scale", 1.0)
    v_scale = settings.get("_v_scale", 1.0)
    count = int(rng.poisson(max(0.0, strength) * max(1.0, h / (22.0 * v_scale))))
    for _ in range(count):
        row = int(rng.integers(0, h))
        band = int(rng.integers(1, max(2, int(round((2 + strength * 7) * v_scale)))))
        y1 = min(h, row + band)
        x0 = int(rng.integers(0, max(1, w)))
        length = int(rng.integers(max(2, w // 10), max(3, w)))
        x1 = min(w, x0 + length)
        if x1 <= x0:
            continue
        alpha = float(np.clip(0.24 + strength * rng.uniform(0.28, 0.72), 0.0, 0.96))
        target = 235.0 if rng.random() > 0.35 else 18.0
        stripe_noise = rng.normal(0.0, 18.0 + strength * 22.0, size=(y1 - row, x1 - x0)).astype(np.float32)
        y[row:y1, x0:x1] = y[row:y1, x0:x1] * (1.0 - alpha) + (target + stripe_noise) * alpha
        i[row:y1, x0:x1] *= 1.0 - alpha * 0.75
        q[row:y1, x0:x1] *= 1.0 - alpha * 0.75

    chance = min(1.0, strength * 0.0018 / max(0.05, h_scale * v_scale))
    speckles = int(rng.binomial(y.size, chance)) if chance > 0.0 else 0
    if speckles:
        if not y.flags["C_CONTIGUOUS"]:
            y = np.ascontiguousarray(y)
        flat = y.reshape(-1)
        flat[rng.integers(0, y.size, size=speckles)] = rng.uniform(0.0, 255.0, size=speckles).astype(np.float32)
    return y, i, q


def _apply_ghosting(y: np.ndarray, i: np.ndarray, q: np.ndarray, settings: dict[str, Any]) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    amount = settings["ghosting"]
    delay = settings["chroma_delay"]
    if amount <= 0.0 and delay <= 0:
        return y, i, q

    h_scale = settings.get("_h_scale", 1.0)
    width = y.shape[1]
    if amount > 0.0:
        echo_delay = max(1, int(round(delay * h_scale + width * amount * 0.035)))
        second_delay = max(echo_delay + 1, int(echo_delay * 2.35))
        echo = _shift_plane(y, echo_delay, 0) * np.float32(amount * 0.15)
        y = y * np.float32(1.0 - amount * 0.18)
        y += echo
        second = _shift_plane(y, second_delay, 0) * np.float32(amount * 0.06)
        y += second
    if delay > 0:
        i, q = _apply_chroma_delay(i, q, _scaled_shift(delay, h_scale, 1))
    return y, i, q


def _apply_scanlines(y: np.ndarray, settings: dict[str, Any], frame_index: int) -> np.ndarray:
    strength = settings["scanlines"]
    if strength <= 0.0:
        return y
    h, w = y.shape
    h_scale = settings.get("_h_scale", 1.0)
    v_scale = settings.get("_v_scale", 1.0)
    rows = (np.arange(h, dtype=np.float32) / v_scale)[:, None]
    x = np.arange(w, dtype=np.float32)[None, :]
    # The line grid belongs to the display, so it must not invert frame to frame.
    alternating = (_reference_lines(h, settings) % 2).astype(np.float32)[:, None]
    triad = 2.0 * math.pi / max(1.0, 3.0 * h_scale)
    darken = np.sin(x * triad + rows * 0.03 + settings["scanline_offset"])
    darken *= np.float32(0.5)
    darken += np.float32(0.5)
    darken *= np.float32(0.08)
    darken += 0.16 * alternating
    darken *= np.float32(strength)
    np.subtract(np.float32(1.0), darken, out=darken)
    darken *= y
    return darken


def _apply_analog_glow_rgb(rgb: np.ndarray, settings: dict[str, Any]) -> np.ndarray:
    amount = settings["analog_glow"]
    if amount <= 0.0:
        return rgb

    rgb_f = rgb.astype(np.float32)
    luma = cv2.transform(rgb_f, _LUMA_ROW)
    if luma.ndim == 2:
        luma = luma[..., None]
    mask = np.clip((luma - 135.0) * np.float32(1.0 / 120.0), 0.0, 1.0)
    bloom = cv2.GaussianBlur(rgb_f * mask, (0, 0), (1.8 + amount * 5.0) * settings.get("_h_scale", 1.0))
    bloom *= _GLOW_CHANNEL_GAIN * np.float32(amount)
    bloom += rgb_f
    np.clip(bloom, 0, 255, out=bloom)
    np.rint(bloom, out=bloom)
    return bloom.astype(np.uint8)


def _edge_wave_profile(
    seed: int, epoch: int, height: int, amplitude: float, v_scale: float = 1.0
) -> np.ndarray:
    """Per-row horizontal wobble of the tape edge for one epoch."""
    rng = np.random.default_rng((int(seed) * 3_732_913_099 + int(epoch) * 40_503) % (2**32))
    raw = rng.normal(0.0, amplitude, size=height).astype(np.float32)
    gain, pole = _scale_one_pole(0.08, 0.92, v_scale)
    return lfilter([gain], [1.0, -pole], raw).astype(np.float32)


def _apply_vhs(
    y: np.ndarray,
    i: np.ndarray,
    q: np.ndarray,
    settings: dict[str, Any],
    rng: np.random.Generator,
) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    if not settings["vhs_enabled"]:
        return y, i, q

    h_scale = settings.get("_h_scale", 1.0)
    y = _lowpass_rows(y, 1_900_000.0, h_scale=h_scale)
    i = _lowpass_rows(i, 300_000.0, h_scale=h_scale)
    q = _lowpass_rows(q, 300_000.0, h_scale=h_scale)

    if settings["vhs_edge_wave"] > 0:
        h, w = y.shape
        span = max(1, TEMPORAL_EPOCH_FRAMES)
        frame_index = int(settings.get("_frame_index", 0))
        epoch = frame_index // span
        mix = (frame_index % span) / float(span)
        amplitude = settings["vhs_edge_wave"] * 0.62 * h_scale
        v_scale = settings.get("_v_scale", 1.0)
        current = _edge_wave_profile(settings["seed"], epoch, h, amplitude, v_scale)
        upcoming = _edge_wave_profile(settings["seed"], epoch + 1, h, amplitude, v_scale)
        smooth = (current * (1.0 - mix) + upcoming * mix).round().astype(np.int32)
        if smooth.any():
            y = _shift_rows(y, smooth)
            i = _shift_rows(i, smooth)
            q = _shift_rows(q, smooth)

    sharp = cv2.GaussianBlur(y, (0, 0), 0.75 * h_scale)
    np.subtract(y, sharp, out=sharp)
    sharp *= np.float32(0.65 + settings["ringing"] * 0.28)
    sharp += y
    y = sharp

    if y.shape[0] >= 2:
        i[1:] = (i[1:] + i[:-1]) * 0.5
        q[1:] = (q[1:] + q[:-1]) * 0.5

    return y, i, q


def _apply_head_switching(
    y: np.ndarray,
    i: np.ndarray,
    q: np.ndarray,
    rng: np.random.Generator,
) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    h, w = y.shape
    band = max(2, int(h * 0.045))
    start = h - band
    base_shift = int(max(4, w * 0.035))
    for row in range(start, h):
        decay = 1.0 - (row - start) / max(1, band)
        shift = max(1, int(base_shift * decay))
        row_noise = rng.normal(0.0, 8.0, size=w).astype(np.float32)
        y[row] = np.pad(y[row], (shift, 0), mode="edge")[:w] * (0.75 + 0.2 * decay) + row_noise
        i[row] *= 0.45
        q[row] *= 0.45
    return y, i, q


def process_rgb_uint8(
    image_rgb: np.ndarray,
    settings_json: str | dict[str, Any] | None = None,
    *,
    frame_index: int = 0,
) -> np.ndarray:
    settings = parse_settings_json(settings_json)
    source = np.ascontiguousarray(image_rgb[..., :3])
    if source.shape[0] == 0 or source.shape[1] == 0:
        return source
    padded, pad = _pad_width_to_multiple_of_four(source)
    h_scale, v_scale = _frame_scale(padded.shape)
    settings["_h_scale"] = h_scale
    settings["_v_scale"] = v_scale
    settings["_frame_index"] = int(frame_index)
    y, i, q = _rgb_to_yiq(padded)

    seed = int(settings["seed"])
    frame_index = int(frame_index)
    # Distinct seeds must not alias onto each other at a different frame.
    rng = np.random.default_rng((seed * 1_000_003 + frame_index * 1009) % (2**32))
    span = max(1, TEMPORAL_EPOCH_FRAMES)
    slow_rng = np.random.default_rng((seed * 2_654_435_761 + (frame_index // span) * 40_503) % (2**32))
    # A half-epoch offset, so warp and dropouts never change on the same frame.
    dropout_rng = np.random.default_rng(
        (seed * 2_246_822_519 + ((frame_index + span // 2) // span) * 40_503) % (2**32)
    )

    i, q = _apply_saturation(i, q, settings)
    y = _apply_black_lift(y, settings)
    i, q = _apply_color_bleed(
        i,
        q,
        _scaled_shift(settings["color_bleed_h"], h_scale),
        _scaled_shift(settings["color_bleed_v"], v_scale),
    )
    i, q = _apply_color_drift(i, q, settings, frame_index)
    y, i, q = _apply_composite_artifacts(y, i, q, settings)
    y = _apply_ringing(y, settings)
    y, i, q = _apply_noise(y, i, q, settings, rng)
    y, i, q = _apply_ghosting(y, i, q, settings)
    y, i, q = _apply_vhs(y, i, q, settings, slow_rng)
    y, i, q = _apply_tracking_warp(y, i, q, settings, slow_rng, frame_index)
    y, i, q = _apply_interlace_roll(y, i, q, settings, frame_index)
    y = _apply_luma_smear(y, settings)
    y, i, q = _apply_dropouts(y, i, q, settings, dropout_rng)
    y = _apply_scanlines(y, settings, frame_index)
    if settings["head_switching"]:
        y, i, q = _apply_head_switching(y, i, q, rng)

    processed = _yiq_to_rgb(y, i, q)
    if pad:
        processed = processed[:, :-pad]
    processed = _apply_analog_glow_rgb(processed, settings)
    blend = settings["blend"]
    if blend < 1.0:
        processed = np.rint(
            np.clip(processed.astype(np.float32) * blend + source.astype(np.float32) * (1.0 - blend), 0, 255)
        ).astype(np.uint8)
    return np.ascontiguousarray(processed)


def _tensor_image_to_rgb_uint8(image: torch.Tensor) -> np.ndarray:
    array = image.detach().cpu().to(torch.float32).numpy()
    if array.ndim != 3:
        raise ValueError("Expected a single HWC image tensor.")
    channels = array.shape[-1]
    if channels == 1:
        array = np.repeat(array, 3, axis=-1)
    elif channels < 3:
        raise ValueError("IMAGE tensors must carry 1, 3 or 4 channels.")
    out = array[..., :3].astype(np.float32)
    np.clip(out, 0.0, 1.0, out=out)
    out *= 255.0
    np.rint(out, out=out)
    return out.astype(np.uint8)


def process_rgb_frames(
    frames,
    settings_json: str | dict[str, Any] | None = None,
    frame_indices=None,
) -> list[np.ndarray]:
    """process_rgb_uint8 over a sequence of frames, across a shared thread pool."""
    frames = list(frames)
    if not frames:
        return []
    indices = list(range(len(frames))) if frame_indices is None else [int(n) for n in frame_indices]
    if len(indices) != len(frames):
        indices = list(range(len(frames)))
    settings = parse_settings_json(settings_json)
    workers = frame_worker_count(len(frames))
    if workers <= 1:
        return [process_rgb_uint8(frame, settings, frame_index=n) for frame, n in zip(frames, indices)]

    def render(frame, index):
        return process_rgb_uint8(frame, settings, frame_index=index)

    pool = _frame_pool(workers)
    out: list[Any] = [None] * len(frames)
    pending: dict[Any, int] = {}
    next_index = 0
    completed = 0
    try:
        while completed < len(frames):
            while next_index < len(frames) and len(pending) < workers:
                pending[pool.submit(render, frames[next_index], indices[next_index])] = next_index
                next_index += 1
            done, _ = wait(list(pending), return_when=FIRST_COMPLETED)
            for future in done:
                out[pending.pop(future)] = future.result()
                completed += 1
    except BaseException:
        for future in pending:
            future.cancel()
        wait(list(pending))
        raise
    return out


def _make_progress_bar(total: int):
    try:
        from comfy.utils import ProgressBar
    except Exception:
        return None
    try:
        return ProgressBar(total)
    except Exception:
        return None


def _raise_if_interrupted() -> None:
    try:
        import comfy.model_management as model_management
    except Exception:
        return
    checker = getattr(model_management, "throw_exception_if_processing_interrupted", None)
    if checker is not None:
        checker()


THREADS_ENV_VAR = "RETROTAPE_THREADS"

# Eight is where the wall clock stops improving: memory bandwidth, not cores.
MAX_FRAME_THREADS = 8

_FRAME_POOLS: dict[int, ThreadPoolExecutor] = {}
_FRAME_POOL_LOCK = Lock()


def _frame_pool(workers: int) -> ThreadPoolExecutor:
    """The shared render pool, sized so an override above MAX_FRAME_THREADS is not a no-op."""
    size = max(MAX_FRAME_THREADS, int(workers))
    with _FRAME_POOL_LOCK:
        pool = _FRAME_POOLS.get(size)
        if pool is None:
            pool = ThreadPoolExecutor(max_workers=size, thread_name_prefix="retrotape-frames")
            _FRAME_POOLS[size] = pool
        return pool


def _thread_override() -> int | None:
    """RETROTAPE_THREADS, or None when it is unset, unreadable or negative."""
    raw = os.environ.get(THREADS_ENV_VAR, "").strip()
    if not raw:
        return None
    try:
        requested = int(raw)
    except ValueError:
        return None
    return requested if requested >= 0 else None


def frame_worker_count(frames: int) -> int:
    """How many frames of a batch to render at once; RETROTAPE_THREADS overrides it,
    with 0 or 1 meaning render sequentially."""
    frames = max(1, int(frames))
    requested = _thread_override()
    if requested is not None:
        return max(1, min(requested, frames)) if requested else 1
    return max(1, min(os.cpu_count() or 1, frames, MAX_FRAME_THREADS))


def _render_frame(image: torch.Tensor, settings: dict[str, Any], frame_index: int) -> np.ndarray:
    rgb = _tensor_image_to_rgb_uint8(image)
    out = process_rgb_uint8(rgb, settings, frame_index=frame_index)
    frame = out.astype(np.float32)
    frame /= 255.0
    return frame


def process_tensor_batch(
    images: torch.Tensor,
    settings_json: str | dict[str, Any] | None = None,
    frame_offset: int = 0,
    *,
    progress=None,
) -> torch.Tensor:
    """Render an NHWC batch; frame_offset is where it sits in a longer clip, which
    the frame-numbered tape artefacts need or every run restarts them.

    A caller that may run this after another path already started reporting -
    the GPU fallback - passes that path's progress bar in rather than a second
    one starting from zero."""
    if images.ndim == 3:
        images = images.unsqueeze(0)
    if images.ndim != 4:
        raise ValueError("IMAGE input must be an NHWC batch.")
    if images.shape[0] == 0:
        return images

    device = images.device
    settings = parse_settings_json(settings_json)
    count = int(images.shape[0])
    progress = _make_progress_bar(count) if progress is None else progress

    processed = torch.empty(
        (count, int(images.shape[1]), int(images.shape[2]), 3),
        dtype=torch.float32,
    )
    offset = int(frame_offset)
    workers = frame_worker_count(count)
    if workers > 1:
        _render_frames_threaded(images, processed, settings, offset, workers, progress)
    else:
        for index in range(count):
            # A long batch must stay cancellable from the ComfyUI queue.
            _raise_if_interrupted()
            processed[index] = torch.from_numpy(_render_frame(images[index], settings, index + offset))
            if progress is not None:
                progress.update(1)

    result = processed.to(device=device, dtype=torch.float32)
    if images.shape[-1] == 4:
        alpha = images[..., 3:4].to(device=device, dtype=torch.float32)
        result = torch.cat([result, alpha], dim=-1)
    return result


def _render_frames_threaded(
    images: torch.Tensor,
    processed: torch.Tensor,
    settings: dict[str, Any],
    offset: int,
    workers: int,
    progress,
) -> None:
    """Render a batch across threads, writing each frame to its own slot."""
    count = int(images.shape[0])
    pending: dict[Any, int] = {}
    next_index = 0
    completed = 0
    pool = _frame_pool(workers)
    try:
        while completed < count:
            _raise_if_interrupted()
            while next_index < count and len(pending) < workers:
                future = pool.submit(_render_frame, images[next_index], settings, next_index + offset)
                pending[future] = next_index
                next_index += 1
            done, _ = wait(list(pending), return_when=FIRST_COMPLETED)
            for future in sorted(done, key=pending.__getitem__):
                index = pending.pop(future)
                processed[index] = torch.from_numpy(future.result())
                completed += 1
                if progress is not None:
                    progress.update(1)
    except BaseException:
        for future in pending:
            future.cancel()
        wait(list(pending))
        raise
