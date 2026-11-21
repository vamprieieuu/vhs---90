"""The browser panel builds its controls from GET /retrotape/presets.

Values and ranges come from the backend, so nothing can drift. What stays in
the panel is presentation - labels, sections, dropdown groups - and these tests
check that it covers exactly the keys the route hands out.
"""

from __future__ import annotations

import json
import re
from pathlib import Path

import pytest
from conftest import route_handlers

from ComfyUI_RetroTape import ntsc_core

SCRIPT = Path(__file__).resolve().parents[1] / "web" / "js" / "retrotape.js"


@pytest.fixture(scope="module")
def catalog():
    return ntsc_core.preset_catalog()


@pytest.fixture(scope="module")
def source():
    return SCRIPT.read_text(encoding="utf-8")


def _js_const(source: str, name: str) -> str:
    """A simple `const NAME = "value";` or `const NAME = 123;` from the panel."""
    match = re.search(r"const %s = ([^;]+);" % re.escape(name), source)
    assert match, f"{name} not found in {SCRIPT.name}"
    return match.group(1).strip()


def _js_keys(source: str, name: str, pattern: str) -> list[str]:
    """The key names out of one of the panel's presentation tables."""
    match = re.search(r"const %s = [\[({](.*?)\n[\])}];" % re.escape(name), source, re.S)
    assert match, f"{name} not found in {SCRIPT.name}"
    return re.findall(pattern, match.group(1))




def test_the_route_lists_every_preset(catalog):
    assert set(catalog["presets"]) == set(ntsc_core.PRESETS) | set(ntsc_core.CUSTOM_PRESETS)
    assert catalog["default_preset"] in catalog["presets"]


def test_the_route_survives_a_json_round_trip(catalog):
    """The panel posts these values straight back, so the backend has to accept
    them unchanged: a value the route hands out and the parser then moves is a
    control that snaps somewhere else the moment it is touched."""
    payload = json.loads(json.dumps(catalog))
    for name, values in payload["presets"].items():
        parsed = ntsc_core.parse_settings_json(json.dumps(values))
        assert parsed == values, name


def test_the_route_ranges_are_the_backend_clamps(catalog):
    expected = {key: list(bounds) for key, bounds in {**ntsc_core._FLOAT_RANGES, **ntsc_core._INT_RANGES}.items()}
    assert catalog["ranges"] == expected
    assert catalog["bool_keys"] == list(ntsc_core._BOOL_KEYS)


def test_a_presets_values_are_all_inside_their_ranges(catalog):
    outside = []
    for name, values in catalog["presets"].items():
        for key, (low, high) in catalog["ranges"].items():
            if not low <= values[key] <= high:
                outside.append("%s.%s = %r, range (%r, %r)" % (name, key, values[key], low, high))
    assert not outside, "preset values outside their control:\n" + "\n".join(outside)


def test_the_route_answers_a_get_with_the_catalog(monkeypatch, catalog):
    import asyncio

    from ComfyUI_RetroTape import server

    if server.web is None:
        pytest.skip("aiohttp is unavailable")
    handler = route_handlers(monkeypatch)[("GET", server.PRESETS_ROUTE_PATH)]
    response = asyncio.run(handler(None))
    assert response.headers["Cache-Control"] == "no-store"
    assert json.loads(response.text) == catalog




def test_the_panel_has_a_label_for_every_key_the_route_returns(source, catalog):
    sliders = _js_keys(source, "SLIDERS", r'\["(\w+)"')
    toggles = _js_keys(source, "TOGGLES", r'\["(\w+)"')
    assert len(sliders) == len(set(sliders)), "a slider is listed twice"
    assert set(sliders) == set(catalog["ranges"])
    assert set(toggles) == set(catalog["bool_keys"])


def test_every_slider_appears_in_exactly_one_section(source):
    sliders = _js_keys(source, "SLIDERS", r'\["(\w+)"')
    grouped = _js_keys(source, "SLIDER_GROUPS", r'"(\w+)"')
    grouped = [key for key in grouped if key in set(sliders)]
    assert len(grouped) == len(set(grouped)), "a slider is listed in two sections"
    assert set(grouped) == set(sliders)


def test_every_preset_appears_in_exactly_one_dropdown_group(source, catalog):
    grouped = _js_keys(source, "PRESET_GROUPS", r'presets: \[([^\]]*)\]')
    names = [name for entry in grouped for name in re.findall(r'"([^"]+)"', entry)]
    assert len(names) == len(set(names)), "a preset is listed in two groups"
    assert set(names) == set(catalog["presets"])


def test_a_presets_values_all_sit_on_its_controls_step(source, catalog):
    """The panel gives every control a track and a typed field, both carrying
    the slider's step. A preset value off that grid shows as an out-of-step
    value in the field and cannot be returned to by dragging."""
    steps = dict(re.findall(r'\["(\w+)", "[^"]+", ([\d.]+)\],', source))
    assert set(steps) == set(catalog["ranges"])
    off_grid = []
    for name, values in catalog["presets"].items():
        for key, step in steps.items():
            if key == "seed":
                continue
            low = catalog["ranges"][key][0]
            offset = (float(values[key]) - float(low)) / float(step)
            if abs(offset - round(offset)) > 1e-6:
                off_grid.append("%s.%s = %r, step %s" % (name, key, values[key], step))
    assert not off_grid, "preset values off the slider grid:\n" + "\n".join(off_grid)




def test_panel_and_server_agree_on_the_routes(source):
    from ComfyUI_RetroTape import server

    assert json.loads(_js_const(source, "PREVIEW_ROUTE")) == server.ROUTE_PATH
    assert json.loads(_js_const(source, "PRESETS_ROUTE")) == server.PRESETS_ROUTE_PATH
    assert json.loads(_js_const(source, "CLEAR_CACHE_ROUTE")) == server.CLEAR_CACHE_ROUTE_PATH


def test_panel_attaches_to_every_registered_node_id(source):
    from ComfyUI_RetroTape.nodes import NODE_CLASS_MAPPINGS

    assert set(json.loads(_js_const(source, "NODE_NAMES"))) == set(NODE_CLASS_MAPPINGS)


def test_panel_preview_sizes_match_the_server(source):
    from ComfyUI_RetroTape import server

    assert int(_js_const(source, "QUICK_PREVIEW_MAX_SIDE")) == server.QUICK_PREVIEW_MAX_SIDE
    # A transport size, so it need only be one the server accepts without clamping.
    full_side = int(_js_const(source, "FULL_PREVIEW_MAX_SIDE"))
    assert server.MIN_PREVIEW_SIDE <= full_side <= server.MAX_PREVIEW_SIDE


def test_panel_video_preview_frames_fit_the_server_ceiling(source):
    from ComfyUI_RetroTape import server

    assert int(_js_const(source, "FULL_VIDEO_PREVIEW_FRAMES")) <= server.MAX_PREVIEW_FRAMES


def test_custom_is_a_label_and_never_a_preset_the_panel_can_apply(source, catalog):
    """The panel must not be able to resolve "Custom" to a preset body: doing so
    is what let a stray dropdown event replace the user's settings."""
    label = json.loads(_js_const(source, "CUSTOM_PRESET"))
    assert label not in catalog["presets"]


def test_the_fallback_default_is_a_preset_the_panel_can_edit():
    """When the route does not answer the panel falls back to the widget's own
    default, so that default has to be a complete settings object."""
    from ComfyUI_RetroTape.nodes import NODE_CLASS_MAPPINGS

    node = NODE_CLASS_MAPPINGS["RetroTape"]
    default = node.INPUT_TYPES()["required"]["settings_json"][1]["default"]
    assert json.loads(default) == ntsc_core.parse_settings_json(default)


def test_this_projects_own_looks_are_further_apart_than_the_ntscQT_four():
    """The set is only worth having if the names mean something. The four
    ntscQT looks are left as their authors made them, and the distance between
    the closest two of them is the bar every look original to this project has
    to clear against every other one. Measured on the rendered picture rather
    than on the settings, because two very different settings vectors can still
    produce the same frame."""
    import itertools

    import numpy as np

    width, height = 128, 96
    x = np.linspace(0, 255, width, dtype=np.float32)[None, :]
    y = np.linspace(0, 255, height, dtype=np.float32)[:, None]
    frame = np.zeros((height, width, 3), dtype=np.uint8)
    frame[..., 0] = np.clip(x * 0.6 + y * 0.4, 0, 255)
    frame[..., 1] = np.clip(255 - y * 0.8, 0, 255)
    frame[..., 2] = np.clip(x * 0.3 + 60, 0, 255)
    frame[:, ::8] = 245
    frame[::6, :] = 15

    names = list(ntsc_core.PRESETS) + list(ntsc_core.CUSTOM_PRESETS)
    rendered = {
        name: ntsc_core.process_rgb_uint8(frame, {**ntsc_core._compact_from_preset(name), "seed": 5}).astype(
            np.float32
        )
        for name in names
    }
    distance = {
        pair: float(np.abs(rendered[pair[0]] - rendered[pair[1]]).mean())
        for pair in itertools.combinations(names, 2)
    }
    community = set(ntsc_core.PRESETS)
    bar = min(value for pair, value in distance.items() if community.issuperset(pair))
    too_close = [
        "%s <-> %s: %.2f" % (a, b, value)
        for (a, b), value in sorted(distance.items(), key=lambda item: item[1])
        if not community.intersection((a, b)) and value < bar
    ]
    assert not too_close, (
        "the ntscQT four sit %.2f levels apart at their closest; these are closer:\n%s"
        % (bar, "\n".join(too_close))
    )
