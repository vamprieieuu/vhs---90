// The four ntscQT preset names appear here; see the header in ntsc_core.py.
import { app } from "../../../scripts/app.js";
import { api } from "../../../scripts/api.js";

const EXTENSION_NAME = "ComfyUI.RetroTape.Preview";
const NODE_NAMES = ["RetroTape"];
const PREVIEW_ROUTE = "/retrotape/preview";
const PRESETS_ROUTE = "/retrotape/presets";
const CLEAR_CACHE_ROUTE = "/retrotape/cache/clear";
const CUSTOM_PRESET = "Custom";
const MIN_NODE_WIDTH = 430;
const MIN_NODE_HEIGHT = 700;
const MIN_WIDGET_HEIGHT = 260;
const MAX_WIDGET_HEIGHT = 4096;
const PREVIEW_DEBOUNCE_MS = 120;
const STATUS_HOLD_MS = 2500;
const QUICK_PREVIEW_MAX_SIDE = 640;
// A transport size: the server renders at source resolution and scales only the result.
const FULL_PREVIEW_MAX_SIDE = 1280;
const FULL_VIDEO_PREVIEW_FRAMES = 16;
// Mirrors DEVICE_CHOICES in device.py.
const DEVICE_CHOICES = ["auto", "cpu", "gpu"];
// LiteGraph's origin id for a link that starts at a subgraph's own input slot.
const SUBGRAPH_INPUT_ID = -10;
// Stops the walk through reroutes and bypassed nodes on a cycle.
const MAX_ORIGIN_HOPS = 12;
// The -4 cancels the gap LiteGraph leaves after every widget.
const HIDDEN_WIDGET_SIZE = [0, -4];

// Ranges come from the backend; the step is the panel's own grid.
const SLIDERS = [
    ["blend", "Blend", 0.01],
    ["seed", "Seed", 1],
    ["video_noise", "Luma noise", 50],
    ["chroma_noise", "Chroma noise", 100],
    ["chroma_phase_noise", "Phase noise", 1],
    ["chroma_loss", "Chroma loss", 20],
    ["saturation", "Saturation", 0.01],
    ["black_lift", "Black lift", 0.01],
    ["color_bleed_h", "Bleed X", 1],
    ["color_bleed_v", "Bleed Y", 1],
    ["chroma_delay", "Chroma delay", 1],
    ["ringing", "Ringing", 0.01],
    ["ringing_power", "Ring power", 1],
    ["vhs_edge_wave", "Edge wave", 1],
    ["tracking_jitter", "Tracking", 0.1],
    ["tape_warp", "Tape warp", 0.05],
    ["dropouts", "Dropouts", 0.01],
    ["ghosting", "Ghosting", 0.01],
    ["scanlines", "Scanlines", 0.01],
    ["color_drift", "Color drift", 0.01],
    ["luma_smear", "Luma smear", 0.01],
    ["interlace", "Interlace", 0.01],
    ["vertical_roll", "V-roll", 0.01],
    ["analog_glow", "Glow", 0.01],
    ["scanline_phase", "Phase", 1],
    ["scanline_offset", "Phase offset", 1],
];

const TOGGLES = [
    ["vhs_enabled", "VHS deck"],
    ["head_switching", "Head switch"],
];

const SLIDER_GROUPS = [
    { title: "Signal", keys: ["blend", "seed", "video_noise", "chroma_noise", "chroma_phase_noise", "chroma_loss"] },
    { title: "Colour", keys: ["saturation", "color_bleed_h", "color_bleed_v", "chroma_delay", "color_drift", "scanline_phase", "scanline_offset"] },
    { title: "Tape path", keys: ["vhs_edge_wave", "tracking_jitter", "tape_warp", "dropouts"] },
    { title: "Picture", keys: ["black_lift", "ringing", "ringing_power", "ghosting", "scanlines", "luma_smear", "interlace", "vertical_roll", "analog_glow"] },
];

const PRESET_GROUPS = [
    { title: "ntscQT community", presets: ["RGM", "Master Nama", "JaneLon", "yobelkcip"] },
    { title: "Everyday tape", presets: ["First Gen", "Home Movie 1987", "Rental Copy", "Film Transfer", "Sun Bleached"] },
    { title: "Off air", presets: ["Weather Channel", "Late Night Broadcast", "Broadcast Dirty"] },
    { title: "Monochrome", presets: ["Security Cam", "Kinescope"] },
    { title: "Stylised", presets: ["Chrome Dream", "Bootleg Anime", "AE Punch"] },
    { title: "Worn out", presets: ["Damaged Tape", "Tracking Hell"] },
];

const SEQUENCE_ONLY = new Set(["vertical_roll"]);

const VHS_ONLY = new Set(["vhs_edge_wave"]);

const SUBTLE_CONTROLS = new Set(["scanline_phase"]);

const NUMERIC_ONLY = new Set(["seed"]);

const SETTING_KEYS = new Set([
    ...SLIDERS.map(([key]) => key),
    ...TOGGLES.map(([key]) => key),
    "preset",
    "base_preset",
]);

const LIVE_PREVIEW_KEY = "retrotape.livePreview";

function readLivePreview() {
    try {
        return window.localStorage?.getItem(LIVE_PREVIEW_KEY) !== "off";
    } catch {
        return true;
    }
}

function writeLivePreview(on) {
    try {
        window.localStorage?.setItem(LIVE_PREVIEW_KEY, on ? "on" : "off");
    } catch {
    }
}

let livePreview = readLivePreview();
const liveListeners = new Set();

function setLivePreview(on) {
    livePreview = on;
    writeLivePreview(on);
    for (const listener of [...liveListeners]) {
        listener(on);
    }
}

function readSectionState(title) {
    try {
        return window.localStorage?.getItem(`retrotape.section.${title}`) !== "closed";
    } catch {
        return true;
    }
}

function writeSectionState(title, open) {
    try {
        window.localStorage?.setItem(`retrotape.section.${title}`, open ? "open" : "closed");
    } catch {
    }
}

function chainCallback(object, property, callback) {
    if (!object) {
        return;
    }
    if (property in object && object[property]) {
        const original = object[property];
        object[property] = function () {
            const result = original.apply(this, arguments);
            return callback.apply(this, arguments) ?? result;
        };
    } else {
        object[property] = callback;
    }
}

function installStyle() {
    if (document.getElementById("retrotape-style")) {
        return;
    }
    const style = document.createElement("style");
    style.id = "retrotape-style";
    style.textContent = `
        /* The chrome follows ComfyUI's own palette; a picture is judged on black. */
        .rt-root {
            --rt-bg: var(--comfy-menu-secondary-bg, #101316);
            --rt-panel: var(--comfy-menu-bg, #171d21);
            --rt-panel-2: var(--content-bg, #20282d);
            --rt-well: var(--comfy-input-bg, #151b1f);
            --rt-line: var(--border-color, #4e4e4e);
            --rt-line-soft: rgba(120, 120, 120, 0.45);
            --rt-line-soft: color-mix(in srgb, var(--border-color, #4e4e4e) 45%, transparent);
            --rt-text: var(--input-text, #f0f3f2);
            --rt-muted: var(--descrip-text, #98a6a5);
            /* Mixed towards the text colour, so one accent reads on either theme;
               the plain value first is what a browser without color-mix keeps. */
            --rt-cyan: #2f8f86;
            --rt-cyan: color-mix(in oklch, #0f8f84 62%, var(--fg-color, #f0f3f2));
            --rt-amber: #9c7c2a;
            --rt-amber: color-mix(in oklch, #a67a0a 45%, var(--fg-color, #f0f3f2));
            /* Keeps the picture the biggest thing in the node. */
            --rt-controls-share: 46%;
            box-sizing: border-box;
            width: 100%;
            height: 100%;
            color: var(--rt-text);
            font: 12px/1.35 ui-monospace, SFMono-Regular, Menlo, Consolas, monospace;
            user-select: none;
        }
        .rt-root * { box-sizing: border-box; }
        .rt-root :focus-visible {
            outline: 2px solid var(--rt-cyan);
            outline-offset: 1px;
        }
        .rt-shell {
            background: linear-gradient(180deg, var(--rt-panel), var(--rt-bg));
            border: 1px solid var(--rt-line);
            border-radius: 7px;
            height: 100%;
            min-height: 0;
            display: flex;
            flex-direction: column;
            /* A node dragged shorter than its controls scrolls them into reach. */
            overflow: hidden auto;
            overscroll-behavior: contain;
            scrollbar-width: thin;
        }
        .rt-preview {
            position: relative;
            min-height: 120px;
            flex: 1 1 auto;
            background:
                linear-gradient(90deg, rgba(91,210,200,0.11), transparent 34%, rgba(217,164,65,0.10)),
                repeating-linear-gradient(0deg, rgba(255,255,255,0.045) 0 1px, transparent 1px 4px),
                #080a0b;
            display: flex;
            align-items: center;
            justify-content: center;
            overflow: hidden;
        }
        .rt-preview img {
            width: 100%;
            height: 100%;
            object-fit: contain;
            image-rendering: auto;
        }
        /* These colours do not follow the theme: they sit on the black bed. */
        .rt-status {
            position: absolute;
            left: 0;
            right: 0;
            bottom: 0;
            padding: 16px 10px 7px;
            background: linear-gradient(180deg, transparent, rgba(3,5,6,0.86) 62%);
            color: #b6c3c2;
            text-shadow: 0 1px 2px #000;
            /* An error message is worth copying. */
            user-select: text;
            cursor: text;
        }
        .rt-status[data-kind="ready"] { color: #6fdcd2; }
        .rt-status[data-kind="error"] { color: #ff8f7d; }
        .rt-frames {
            position: absolute;
            top: 7px;
            right: 8px;
            padding: 2px 6px;
            border-radius: 4px;
            background: rgba(3,5,6,0.74);
            color: #e0ae52;
            font-size: 10px;
            letter-spacing: 0.05em;
            pointer-events: none;
        }
        /* A narrow node stacks the actions instead of shrinking them. */
        .rt-bar {
            display: flex;
            flex-wrap: wrap;
            gap: 6px;
            padding: 8px;
            background: var(--rt-panel-2);
            border-top: 1px solid var(--rt-line-soft);
            flex: 0 0 auto;
        }
        .rt-select,
        .rt-button {
            height: 27px;
            border: 1px solid var(--rt-line);
            border-radius: 6px;
            background: var(--rt-well);
            color: var(--rt-text);
            font: inherit;
        }
        /* The preset name takes a row of its own; the actions share the next. */
        .rt-presetrow {
            flex: 1 1 100%;
            display: flex;
            gap: 6px;
            align-items: center;
            min-width: 0;
        }
        .rt-select {
            flex: 1 1 auto;
            min-width: 0;
            padding: 0 4px;
        }
        .rt-root[data-preset="custom"] .rt-select { color: var(--rt-amber); }
        .rt-modified {
            flex: 0 0 auto;
            align-self: center;
            color: var(--rt-amber);
            font-size: 10px;
            letter-spacing: 0.06em;
            text-transform: uppercase;
        }
        .rt-button {
            flex: 1 1 auto;
            min-width: 32px;
            max-width: 108px;
            padding: 0 9px;
            cursor: pointer;
        }
        .rt-button:hover {
            border-color: var(--rt-cyan);
            color: var(--rt-cyan);
        }
        .rt-button[aria-pressed="false"] {
            color: var(--rt-muted);
            border-style: dashed;
        }
        .rt-controls {
            display: flex;
            flex-direction: column;
            gap: 6px;
            padding: 9px;
            background: var(--rt-bg);
            flex: 0 1 auto;
            min-height: 0;
            max-height: var(--rt-controls-share);
            overflow-y: auto;
            overscroll-behavior: contain;
            scrollbar-width: thin;
        }
        .rt-section {
            border: 1px solid var(--rt-line-soft);
            border-radius: 6px;
            background: var(--rt-panel);
        }
        .rt-section > summary {
            cursor: pointer;
            padding: 5px 8px;
            color: var(--rt-cyan);
            letter-spacing: 0.08em;
            text-transform: uppercase;
            font-size: 10px;
            list-style: none;
            user-select: none;
        }
        .rt-section > summary::-webkit-details-marker { display: none; }
        .rt-section > summary::before {
            content: "▸";
            display: inline-block;
            width: 11px;
            color: var(--rt-muted);
            transition: transform 0.12s ease;
        }
        .rt-section[open] > summary::before { transform: rotate(90deg); }
        .rt-section > summary:hover { color: var(--rt-amber); }
        /* Two columns only once each can hold a track worth dragging. */
        .rt-fields {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(min(260px, 100%), 1fr));
            gap: 7px 10px;
            padding: 2px 8px 9px;
        }
        .rt-field[data-sequence-only="true"] label::after,
        .rt-field[data-vhs-only="true"] label::after {
            content: " •";
            color: var(--rt-amber);
        }
        .rt-field[data-subtle="true"] label::after {
            content: " •";
            color: var(--rt-muted);
        }
        .rt-field {
            display: grid;
            grid-template-columns: 92px minmax(0, 1fr) 54px;
            gap: 6px;
            align-items: center;
            min-width: 0;
        }
        .rt-field[data-numeric="true"] {
            grid-template-columns: 92px minmax(0, 1fr) auto;
        }
        /* A ten-digit seed reads left to right, a slider value right-aligned. */
        .rt-field[data-numeric="true"] .rt-number { text-align: left; }
        .rt-field label {
            color: var(--rt-muted);
            overflow: hidden;
            text-overflow: ellipsis;
            white-space: nowrap;
            cursor: pointer;
        }
        .rt-field .rt-number {
            width: 100%;
            min-width: 0;
            text-align: right;
            color: var(--rt-amber);
            background: var(--rt-well);
            border: 1px solid var(--rt-line);
            border-radius: 4px;
            padding: 2px 4px;
            font: inherit;
        }
        /* The spin buttons are unusable in a 54px field; arrow keys still step. */
        .rt-number::-webkit-outer-spin-button,
        .rt-number::-webkit-inner-spin-button { -webkit-appearance: none; margin: 0; }
        .rt-number { appearance: textfield; -moz-appearance: textfield; }
        .rt-field .rt-number:focus { border-color: var(--rt-cyan); }
        .rt-field input[type="range"] {
            width: 100%;
            min-width: 0;
            height: 18px;
            margin: 0;
            accent-color: var(--rt-cyan);
            cursor: ew-resize;
        }
        .rt-roll {
            height: 22px;
            padding: 0 7px;
            border: 1px solid var(--rt-line);
            border-radius: 4px;
            background: var(--rt-well);
            color: var(--rt-text);
            font: inherit;
            cursor: pointer;
        }
        .rt-roll:hover {
            border-color: var(--rt-cyan);
            color: var(--rt-cyan);
        }
        .rt-switches {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(min(110px, 100%), 1fr));
            gap: 7px;
            padding: 0 9px 9px;
            background: var(--rt-bg);
            flex: 0 0 auto;
        }
        .rt-toggle {
            height: 30px;
            display: flex;
            align-items: center;
            gap: 7px;
            padding: 0 8px;
            border-radius: 6px;
            background: var(--rt-well);
            border: 1px solid var(--rt-line);
            color: var(--rt-muted);
            min-width: 0;
            cursor: pointer;
        }
        .rt-toggle input { accent-color: var(--rt-cyan); }
        .rt-toggle span {
            overflow: hidden;
            text-overflow: ellipsis;
            white-space: nowrap;
        }
    `;
    document.head.appendChild(style);
}

// One successful fetch is shared by every node; a failed one is not, so the next retries.
let catalogRequest = null;

function fallbackCatalog(settingsWidget) {
    let defaults = {};
    try {
        defaults = JSON.parse(settingsWidget?.value || "{}") || {};
    } catch {
    }
    const name = typeof defaults.preset === "string" ? defaults.preset : "Default";
    defaults.preset = name;
    return { presets: { [name]: defaults }, ranges: {}, defaultPreset: name, degraded: true };
}

function loadCatalog() {
    catalogRequest ??= (async () => {
        const response = await api.fetchApi(PRESETS_ROUTE);
        if (!response.ok) {
            throw new Error(String(response.status));
        }
        const data = await response.json();
        return {
            presets: data.presets,
            ranges: data.ranges,
            defaultPreset: data.default_preset,
            degraded: false,
        };
    })();
    return catalogRequest.catch((error) => {
        catalogRequest = null;
        throw error;
    });
}

function fromPreset(catalog, name) {
    return { ...(catalog.presets[name] ?? catalog.presets[catalog.defaultPreset]) };
}

// Snaps to the control's own grid, so a typed value lands where dragging can return to.
function quantize(value, step, range) {
    let number = Number(value);
    if (!Number.isFinite(number)) {
        number = range ? range[0] : 0;
    }
    const origin = range ? range[0] : 0;
    number = origin + Math.round((number - origin) / step) * step;
    if (range) {
        number = Math.min(range[1], Math.max(range[0], number));
    }
    return step >= 1 ? Math.round(number) : Number(number.toFixed(3));
}

function pickSettings(raw) {
    const picked = {};
    for (const [key, value] of Object.entries(raw || {})) {
        if (SETTING_KEYS.has(key)) {
            picked[key] = value;
        }
    }
    return picked;
}

function normalizeSettings(catalog, raw) {
    const picked = pickSettings(raw);
    if (catalog.degraded) {
        // No catalog to resolve a name against, so the saved name and values stand.
        const settings = { ...fromPreset(catalog, catalog.defaultPreset), ...picked };
        settings.preset = typeof picked.preset === "string" ? picked.preset : catalog.defaultPreset;
        settings.base_preset = typeof picked.base_preset === "string" ? picked.base_preset : settings.preset;
        return quantizeSettings(catalog, settings);
    }
    // base_preset is where Reset and a double-click go back to once preset reads "Custom".
    const requested = picked.preset;
    const isCustom = requested === CUSTOM_PRESET;
    const preset = isCustom ? CUSTOM_PRESET : (catalog.presets[requested] ? requested : catalog.defaultPreset);
    const base = isCustom ? (catalog.presets[picked.base_preset] ? picked.base_preset : catalog.defaultPreset) : preset;
    const settings = { ...fromPreset(catalog, base), ...picked };
    settings.preset = preset;
    settings.base_preset = base;
    return quantizeSettings(catalog, settings);
}

function quantizeSettings(catalog, settings) {
    for (const [key, , step] of SLIDERS) {
        settings[key] = quantize(settings[key], step, catalog.ranges[key]);
    }
    for (const [key] of TOGGLES) {
        settings[key] = Boolean(settings[key]);
    }
    return settings;
}

function matchesPreset(catalog, settings, name) {
    const preset = fromPreset(catalog, name);
    for (const [key] of SLIDERS) {
        if (Math.abs(Number(settings[key]) - Number(preset[key])) > 1e-6) {
            return false;
        }
    }
    for (const [key] of TOGGLES) {
        if (Boolean(settings[key]) !== Boolean(preset[key])) {
            return false;
        }
    }
    return true;
}

function parseWidgetRaw(widget) {
    try {
        const value = JSON.parse(widget?.value || "{}");
        return value && typeof value === "object" && !Array.isArray(value) ? value : {};
    } catch {
        return {};
    }
}

function hideSettingsWidget(widget) {
    if (!widget) {
        return;
    }
    widget.hidden = true;
    widget.computeSize = () => HIDDEN_WIDGET_SIZE.slice();
    const element = widget.element ?? widget.inputEl;
    if (element) {
        element.style.display = "none";
    }
}

function writeSettings(widget, state) {
    if (!widget) {
        return;
    }
    widget.value = JSON.stringify(state);
    widget.callback?.(widget.value);
}

function getGraphLink(graph, linkId) {
    if (!graph || linkId == null) {
        return null;
    }
    if (typeof graph.links?.get === "function") {
        return graph.links.get(linkId) ?? null;
    }
    return graph.links?.[linkId] ?? null;
}

function getGraphNode(graph, nodeId) {
    if (!graph || nodeId == null) {
        return null;
    }
    return graph.getNodeById?.(Number(nodeId)) ?? graph.getNodeById?.(nodeId) ?? graph._nodes_by_id?.[nodeId] ?? null;
}

function getWidgetValue(node, name) {
    return node?.widgets?.find((widget) => widget.name === name)?.value;
}

function numericWidgetValue(node, name, fallback = 0) {
    const value = Number(getWidgetValue(node, name));
    return Number.isFinite(value) ? value : fallback;
}

// The preview must render where the node will: the two draw different grain.
function deviceWidgetValue(node) {
    const value = getWidgetValue(node, "device");
    return DEVICE_CHOICES.includes(value) ? value : "auto";
}

// The node feeding this one, or {boundary: true} when it comes through a subgraph input.
function traceOrigin(node, inputName, depth = 0) {
    if (!node || depth > MAX_ORIGIN_HOPS) {
        return null;
    }
    const input = node.inputs?.find((entry) => entry.name === inputName) ?? node.inputs?.[0];
    const link = getGraphLink(node.graph, input?.link);
    if (!link) {
        return null;
    }
    if (Number(link.origin_id) === SUBGRAPH_INPUT_ID) {
        return { boundary: true };
    }
    const origin = getGraphNode(node.graph, link.origin_id);
    if (!origin) {
        return null;
    }
    const originClass = String(origin.comfyClass ?? origin.type ?? "");
    if (originClass.includes("Reroute") || origin.mode === 4) {
        return traceOrigin(origin, inputName, depth + 1);
    }
    return { node: origin };
}

function traceImageOrigin(node, depth = 0) {
    return traceOrigin(node, "image", depth);
}

// A VIDEO object is not an image until something decomposes it, so the file is one hop further back.
function traceVideoLoader(node) {
    let current = node;
    for (let hop = 0; current && hop <= MAX_ORIGIN_HOPS; hop += 1) {
        if (videoFileOf(current) !== null) {
            return current;
        }
        current = traceOrigin(current, "video")?.node ?? null;
    }
    return null;
}

function videoFileOf(node) {
    const value = getWidgetValue(node, "video") ?? getWidgetValue(node, "file");
    return typeof value === "string" && value ? value : null;
}

function describeOrigin(origin) {
    return origin ? String(origin.title ?? origin.comfyClass ?? origin.type ?? "an upstream node") : "an upstream node";
}

function getConnectedSource(node) {
    const sourceNode = traceImageOrigin(node)?.node ?? null;
    if (sourceNode?.mode === 2) {
        // Muted: its file is not what the queued workflow would produce.
        return null;
    }
    const nodeClass = String(sourceNode?.comfyClass ?? sourceNode?.type ?? "");
    const title = String(sourceNode?.title ?? "");

    const isLoadVideo = nodeClass.includes("LoadVideo") || title.includes("Load Video");
    const decomposesVideo = nodeClass.includes("VideoComponents");
    if (sourceNode && (isLoadVideo || decomposesVideo)) {
        const loader = decomposesVideo ? traceVideoLoader(sourceNode) : sourceNode;
        const video = videoFileOf(loader);
        if (!video || loader.mode === 2) {
            return null;
        }
        const loaderClass = String(loader.comfyClass ?? loader.type ?? "");
        const isPath = loaderClass.includes("Path") || String(loader.title ?? "").includes("(Path)");
        const frameLoadCap = Math.max(0, numericWidgetValue(loader, "frame_load_cap", 0));
        return {
            source_video: {
                filename: video,
                path: isPath,
                type: "input",
                frame_load_cap: frameLoadCap,
                force_rate: Math.max(0, numericWidgetValue(loader, "force_rate", 0)),
                start_time: Math.max(0, numericWidgetValue(loader, "start_time", 0)),
                skip_first_frames: Math.max(0, numericWidgetValue(loader, "skip_first_frames", 0)),
                select_every_nth: Math.max(1, numericWidgetValue(loader, "select_every_nth", 1)),
                max_frames: frameLoadCap > 0 ? Math.min(frameLoadCap, FULL_VIDEO_PREVIEW_FRAMES) : FULL_VIDEO_PREVIEW_FRAMES,
            },
        };
    }

    const isLoadImage = nodeClass.includes("LoadImage") || title === "Load Image" || title === "Load Image (Output)";
    if (!sourceNode || !isLoadImage) {
        return null;
    }

    const image = getWidgetValue(sourceNode, "image");
    if (!image || typeof image !== "string") {
        return null;
    }

    const isOutput = nodeClass.includes("LoadImageOutput") || title === "Load Image (Output)";
    return {
        source_image: {
            filename: image,
            type: isOutput ? "output" : "input",
        },
    };
}

function sourceFingerprint(node) {
    return JSON.stringify(getConnectedSource(node) ?? null);
}

async function copyText(value) {
    try {
        if (navigator.clipboard?.writeText) {
            await navigator.clipboard.writeText(value);
            return true;
        }
    } catch {
        // navigator.clipboard is unavailable over plain http; fall through.
    }
    const scratch = document.createElement("textarea");
    try {
        scratch.value = value;
        scratch.style.position = "fixed";
        scratch.style.opacity = "0";
        document.body.appendChild(scratch);
        scratch.select();
        return document.execCommand("copy");
    } catch {
        return false;
    } finally {
        scratch.remove();
    }
}

async function readClipboardText() {
    try {
        if (navigator.clipboard?.readText) {
            return await navigator.clipboard.readText();
        }
    } catch {
    }
    try {
        // The desktop build has no window.prompt and returns null here.
        return window.prompt?.("Paste settings JSON") ?? null;
    } catch {
        return null;
    }
}

function previewStatusText(headers, quick, requestedDevice) {
    // The header reports the path that ran, not the one asked for.
    const device = (headers.get("X-RetroTape-Device") ?? "").toLowerCase();
    let deviceNote = "";
    if (device === "cuda") {
        deviceNote = " on GPU";
    } else if (device === "cpu") {
        deviceNote = requestedDevice === "gpu" ? " (GPU unavailable, used CPU)" : " on CPU";
    }
    if (quick) {
        return "Quick preview. Refining...";
    }
    const renderSize = headers.get("X-RetroTape-Render-Size") ?? "";
    const sourceSize = headers.get("X-RetroTape-Source-Size") ?? "";
    if (renderSize && sourceSize && renderSize !== sourceSize) {
        return `Rendered at ${renderSize}${deviceNote}; output will be ${sourceSize}.`;
    }
    if (renderSize) {
        return `Matches the output: ${renderSize}${deviceNote}.`;
    }
    return `Preview ready${deviceNote}.`;
}

// Ctrl+Z restores a widget write only if the tracker was asked to look.
function takeUndoSnapshot() {
    const tracker = app.extensionManager?.workflow?.activeWorkflow?.changeTracker
        ?? app.workflowManager?.activeWorkflow?.changeTracker;
    if (typeof tracker?.checkState !== "function") {
        return false;
    }
    tracker.checkState();
    return true;
}

function setStatus(statusEl, text, kind = "") {
    statusEl.textContent = text;
    statusEl.dataset.kind = kind;
}

// node.id is still -1 while onNodeCreated runs, so element ids cannot use it.
let panelSerial = 0;

function fillPresetOptions(select, catalog, current) {
    const addOption = (parent, name) => {
        const option = document.createElement("option");
        option.value = name;
        option.textContent = name;
        parent.append(option);
    };
    for (const group of PRESET_GROUPS) {
        const holder = document.createElement("optgroup");
        holder.label = group.title;
        for (const name of group.presets) {
            if (catalog.presets[name]) {
                addOption(holder, name);
            }
        }
        if (holder.childElementCount) {
            select.append(holder);
        }
    }
    for (const name of [...Object.keys(catalog.presets), current]) {
        if (name && name !== CUSTOM_PRESET && !select.querySelector(`option[value="${CSS.escape(name)}"]`)) {
            addOption(select, name);
        }
    }
    addOption(select, CUSTOM_PRESET);
}

function createPanelShell() {
    const root = document.createElement("div");
    root.className = "rt-root";
    root.innerHTML = `
        <div class="rt-shell">
            <div class="rt-preview">
                <img alt="Tape preview" decoding="async" hidden />
                <div class="rt-frames" hidden></div>
                <div class="rt-status" role="status" aria-live="polite">Connect Load Image/Video or run once.</div>
            </div>
            <div class="rt-bar">
                <div class="rt-presetrow">
                    <select class="rt-select" aria-label="Preset"></select>
                    <span class="rt-modified" hidden>modified</span>
                </div>
                <button class="rt-button" data-act="compare" type="button" aria-label="Compare with the source" title="Hold to see the untouched source">A/B</button>
                <button class="rt-button" data-act="refresh" type="button" title="Render the preview again">Refresh</button>
                <button class="rt-button" data-act="reset" type="button">Reset</button>
                <button class="rt-button" data-act="copy" type="button" title="Copy the settings as JSON">Copy</button>
                <button class="rt-button" data-act="paste" type="button" title="Replace the settings from JSON on the clipboard">Paste</button>
                <button class="rt-button" data-act="live" type="button" aria-pressed="true">Live</button>
            </div>
            <div class="rt-controls"></div>
            <div class="rt-switches"></div>
        </div>
    `;
    const button = (action) => root.querySelector(`.rt-button[data-act="${action}"]`);
    const ui = {
        root,
        image: root.querySelector("img"),
        frames: root.querySelector(".rt-frames"),
        status: root.querySelector(".rt-status"),
        preset: root.querySelector(".rt-select"),
        modified: root.querySelector(".rt-modified"),
        controls: root.querySelector(".rt-controls"),
        switches: root.querySelector(".rt-switches"),
        compare: button("compare"),
        refresh: button("refresh"),
        reset: button("reset"),
        copy: button("copy"),
        paste: button("paste"),
        live: button("live"),
    };
    return ui;
}

function createSettingsStore(ctx, preview) {
    const { ui, catalog } = ctx;
    const groupOf = (name) => PRESET_GROUPS.find((group) => group.presets.includes(name))?.title ?? "";

    function syncPresetIndicator() {
        if (catalog.degraded) {
            ui.modified.hidden = true;
            return;
        }
        const base = ctx.state.base_preset;
        const custom = ctx.state.preset === CUSTOM_PRESET;
        ui.root.dataset.preset = custom ? "custom" : "preset";
        ui.modified.hidden = !custom;
        ui.modified.title = `Edited away from ${base}. Reset puts it back.`;
        ui.preset.title = custom
            ? `Custom - edited away from ${base}`
            : `${groupOf(ctx.state.preset)}: ${ctx.state.preset}`.replace(/^: /, "");
        ui.reset.title = `Put ${base} back`;
        ui.reset.setAttribute("aria-label", `Reset to ${base}`);
    }

    function syncPresetName() {
        if (catalog.degraded) {
            syncPresetIndicator();
            return;
        }
        const name = matchesPreset(catalog, ctx.state, ctx.state.base_preset) ? ctx.state.base_preset : CUSTOM_PRESET;
        if (ctx.state.preset !== name) {
            ctx.state.preset = name;
            ui.preset.value = name;
        }
        syncPresetIndicator();
    }

    function syncControls() {
        ui.preset.value = ctx.state.preset;
        syncPresetIndicator();
        for (const [key, control] of ctx.controls) {
            if (control.input.type === "checkbox") {
                control.input.checked = Boolean(ctx.state[key]);
            } else {
                control.input.value = String(ctx.state[key]);
                control.output.value = String(ctx.state[key]);
            }
        }
    }

    function persist() {
        writeSettings(ctx.settingsWidget, ctx.stored);
    }

    // ctx.state drives the controls; ctx.stored is what the widget keeps.
    function adopt(raw) {
        ctx.state = normalizeSettings(catalog, raw);
        ctx.stored = catalog.degraded ? pickSettings(raw) : ctx.state;
    }

    function touch(key, value) {
        ctx.state[key] = value;
        ctx.stored[key] = value;
        syncPresetName();
        persist();
        preview.schedule();
    }

    function setState(next) {
        adopt(next);
        // A pasted object can name a preset its own values no longer match.
        syncPresetName();
        persist();
        syncControls();
        preview.schedule();
        ctx.node.setDirtyCanvas?.(true, true);
        ctx.node.graph?.setDirtyCanvas?.(true, true);
    }

    function loadFromWidget() {
        adopt(parseWidgetRaw(ctx.settingsWidget));
        syncControls();
    }

    function applyPreset(name) {
        if (catalog.degraded) {
            preview.status("Presets unavailable, so Reset does not know what to put back.", "error", { hold: true });
            return;
        }
        if (!catalog.presets[name]) {
            return;
        }
        setState(fromPreset(catalog, name));
        preview.status(`${name} applied.`, "", { hold: true });
    }

    return { adopt, loadFromWidget, syncControls, syncPresetName, setState, applyPreset, persist, touch };
}

function buildControlRows(ctx, store) {
    const { ui, catalog } = ctx;
    const sections = new Map();
    const sectionFor = (key) => {
        const group = SLIDER_GROUPS.find((entry) => entry.keys.includes(key));
        const title = group?.title ?? "Other";
        let body = sections.get(title);
        if (!body) {
            const holder = document.createElement("details");
            holder.className = "rt-section";
            holder.open = readSectionState(title);
            holder.innerHTML = `<summary>${title}</summary><div class="rt-fields"></div>`;
            holder.addEventListener("toggle", () => writeSectionState(title, holder.open));
            ui.controls.append(holder);
            body = holder.querySelector(".rt-fields");
            sections.set(title, body);
        }
        return body;
    };
    for (const group of SLIDER_GROUPS) {
        if (group.keys.some((key) => SLIDERS.some(([sliderKey]) => sliderKey === key))) {
            sectionFor(group.keys[0]);
        }
    }

    for (const [key, label, step] of SLIDERS) {
        const range = catalog.ranges[key];
        const trackless = !range || NUMERIC_ONLY.has(key);
        const rollable = Boolean(range) && NUMERIC_ONLY.has(key);
        const row = document.createElement("div");
        row.className = "rt-field";
        row.innerHTML = `
            <label></label>
            ${trackless ? "" : `<input type="range" />`}
            <input type="number" class="rt-number" />
            ${rollable ? `<button type="button" class="rt-roll">Roll</button>` : ""}
        `;
        const labelEl = row.querySelector("label");
        const output = row.querySelector('input[type="number"]');
        const input = row.querySelector('input[type="range"]') ?? output;
        const inputId = `rt-${ctx.panelId}-${key}`;
        labelEl.textContent = label;
        const note = SEQUENCE_ONLY.has(key)
            ? " - needs a frame sequence; it does nothing on a single image"
            : VHS_ONLY.has(key)
                ? " - does nothing while the VHS deck is off"
                : SUBTLE_CONTROLS.has(key)
                    ? " - very subtle; it only moves where a fine pattern lands"
                    : "";
        labelEl.title = `${label}${note}. Double-click to reset to the preset value.`;
        labelEl.htmlFor = inputId;
        if (SEQUENCE_ONLY.has(key)) {
            row.dataset.sequenceOnly = "true";
        }
        if (VHS_ONLY.has(key)) {
            row.dataset.vhsOnly = "true";
        }
        if (SUBTLE_CONTROLS.has(key)) {
            row.dataset.subtle = "true";
        }
        if (trackless) {
            row.dataset.numeric = "true";
        }
        input.id = inputId;
        const bounds = range ? { min: range[0], max: range[1], step } : { step };
        Object.assign(input, bounds);
        Object.assign(output, bounds);
        output.setAttribute("aria-label", `${label} value`);
        if (input !== output) {
            input.setAttribute("aria-label", label);
        }
        const commit = (raw) => {
            const value = quantize(raw, step, range);
            input.value = String(value);
            output.value = String(value);
            if (ctx.state[key] === value) {
                return;
            }
            store.touch(key, value);
        };
        if (input !== output) {
            input.addEventListener("input", () => commit(input.value));
        }
        output.addEventListener("change", () => commit(output.value));
        if (rollable) {
            const roll = row.querySelector(".rt-roll");
            roll.title = `New random ${label.toLowerCase()}`;
            roll.setAttribute("aria-label", `Randomise ${label.toLowerCase()}`);
            roll.addEventListener("click", () => commit(Math.floor(Math.random() * (range[1] - range[0] + 1)) + range[0]));
        }
        labelEl.addEventListener("dblclick", (event) => {
            event.preventDefault();
            commit(fromPreset(catalog, ctx.state.base_preset)[key]);
        });
        sectionFor(key).append(row);
        ctx.controls.set(key, { input, output });
    }

    for (const [key, label] of TOGGLES) {
        const row = document.createElement("label");
        row.className = "rt-toggle";
        row.innerHTML = `<input type="checkbox" /><span></span>`;
        const input = row.querySelector("input");
        row.querySelector("span").textContent = label;
        input.addEventListener("change", () => store.touch(key, input.checked));
        ui.switches.append(row);
        ctx.controls.set(key, { input });
    }
}

function createPreviewController(ctx) {
    const { node, ui } = ctx;
    let previewTimer = null;
    let requestSequence = 0;
    let objectUrl = null;
    let compareUrl = null;
    let compareAbort = null;
    let comparing = false;
    let statusBeforeCompare = null;
    let disposed = false;
    let holdUntil = 0;

    const status = (text, kind = "", { hold = false, transient = false } = {}) => {
        if (transient && Date.now() < holdUntil) {
            return;
        }
        holdUntil = hold ? Date.now() + STATUS_HOLD_MS : 0;
        setStatus(ui.status, text, kind);
    };

    const dropCompare = () => {
        comparing = false;
        compareAbort?.abort();
        compareAbort = null;
        if (compareUrl) {
            URL.revokeObjectURL(compareUrl);
            compareUrl = null;
        }
    };

    const previewBody = (previewMode) => {
        const source = getConnectedSource(node);
        const body = {
            // Inside a subgraph the backend keys frames by a composite id.
            node_id: String(node.rtExecutionId ?? node.id),
            settings_json: JSON.stringify(ctx.state),
            preview_mode: previewMode,
            max_side: previewMode === "fast" ? QUICK_PREVIEW_MAX_SIDE : FULL_PREVIEW_MAX_SIDE,
            // Tape artefacts follow the frame number, so a still needs its own.
            frame_offset: Math.max(0, numericWidgetValue(node, "frame_offset", 0)),
            device: deviceWidgetValue(node),
        };
        if (source?.source_image) {
            body.source_image = source.source_image;
        }
        if (source?.source_video) {
            body.source_video = {
                ...source.source_video,
                max_frames: previewMode === "fast" ? 1 : source.source_video.max_frames,
            };
        }
        return body;
    };

    async function renderSourceFrame() {
        compareAbort?.abort();
        const controller = new AbortController();
        compareAbort = controller;
        try {
            const response = await api.fetchApi(PREVIEW_ROUTE, {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                signal: controller.signal,
                body: JSON.stringify({
                    ...previewBody("full"),
                    // blend 0 returns the source untouched, same path, same frame.
                    settings_json: JSON.stringify({ ...ctx.state, blend: 0 }),
                }),
            });
            if (!response.ok) {
                throw new Error(String(response.status));
            }
            const url = URL.createObjectURL(await response.blob());
            if (disposed || controller.signal.aborted || !comparing) {
                URL.revokeObjectURL(url);
                return null;
            }
            compareUrl = url;
            return url;
        } catch (error) {
            if (error?.name !== "AbortError" && !disposed) {
                status("Could not render the source to compare.", "error");
            }
            return null;
        } finally {
            if (compareAbort === controller) {
                compareAbort = null;
            }
        }
    }

    node.rtShowSource = async function (show) {
        if (!objectUrl || comparing === show) {
            return;
        }
        if (!show) {
            comparing = false;
            compareAbort?.abort();
            compareAbort = null;
            ui.image.src = objectUrl;
            if (statusBeforeCompare) {
                status(statusBeforeCompare.text, statusBeforeCompare.kind);
            }
            return;
        }
        comparing = true;
        statusBeforeCompare = { text: ui.status.textContent, kind: ui.status.dataset.kind ?? "" };
        if (!compareUrl) {
            status("Rendering the source...");
            if (!(await renderSourceFrame())) {
                comparing = false;
                return;
            }
        }
        if (comparing) {
            ui.image.src = compareUrl;
            status("Source, untouched. Release to go back.");
        }
    };

    node.rtSchedulePreview = function () {
        clearTimeout(previewTimer);
        if (!livePreview) {
            status("Paused. Press Refresh to render.", "", { transient: true });
            return;
        }
        status("Preview pending...", "", { transient: true });
        previewTimer = setTimeout(() => node.rtRefreshPreview(), PREVIEW_DEBOUNCE_MS);
    };

    node.rtRefreshPreview = async function () {
        clearTimeout(previewTimer);

        // A newer refresh replaces node.rtAbort, so this call keeps its own.
        const sequence = ++requestSequence;
        const previousAbort = node.rtAbort;
        const controller = new AbortController();
        node.rtAbort = controller;
        previousAbort?.abort();

        const isCurrent = () => sequence === requestSequence && !controller.signal.aborted && !disposed;

        if (node.mode === 4) {
            status("Node is bypassed; this preview will not be applied.", "error");
        } else if (node.mode === 2) {
            status("Node is muted; this preview will not be applied.", "error");
        } else {
            status("Rendering preview...", "", { transient: true });
        }

        try {
            const requestPreview = (previewMode) => api.fetchApi(PREVIEW_ROUTE, {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                signal: controller.signal,
                body: JSON.stringify(previewBody(previewMode)),
            });
            const showResponse = async (response, quick = false) => {
                if (!isCurrent()) {
                    return false;
                }
                if (response.status === 409) {
                    status("Replaced by a newer preview.");
                    return false;
                }
                if (!response.ok) {
                    if (response.status === 404) {
                        ui.image.hidden = true;
                        ui.frames.hidden = true;
                        const detail = await response.json().catch(() => null);
                        if (detail?.reason === "source_unavailable") {
                            status(String(detail.error ?? "Preview source is unavailable."), "error");
                            return false;
                        }
                        const origin = traceImageOrigin(node);
                        if (!origin) {
                            status("Connect an image input to preview.");
                        } else if (origin.boundary) {
                            status("Queue once to preview (input comes from outside the subgraph).");
                        } else {
                            status(`Queue once to preview. Input comes from ${describeOrigin(origin.node)}.`);
                        }
                        return false;
                    }
                    let message = `${response.status} ${response.statusText}`;
                    try {
                        const data = await response.json();
                        message = data.error || message;
                    } catch {
                    }
                    status(message, "error");
                    return false;
                }

                const blob = await response.blob();
                if (!isCurrent()) {
                    return false;
                }
                if (objectUrl) {
                    URL.revokeObjectURL(objectUrl);
                }
                objectUrl = URL.createObjectURL(blob);
                if (!quick) {
                    dropCompare();
                }
                ui.image.src = objectUrl;
                ui.image.hidden = false;
                // An animated WebP gives no visible frame count.
                const frameCount = Number(response.headers.get("X-RetroTape-Frames") || 1);
                ui.frames.hidden = !(frameCount > 1);
                if (frameCount > 1) {
                    ui.frames.textContent = `${frameCount} frames`;
                }
                status(previewStatusText(response.headers, quick, deviceWidgetValue(node)), "ready");
                return true;
            };

            const quickResponse = await requestPreview("fast");
            const quickOk = await showResponse(quickResponse, true);
            if (!quickOk || !isCurrent()) {
                return;
            }

            const fullResponse = await requestPreview("full");
            await showResponse(fullResponse, false);
        } catch (error) {
            if (error?.name !== "AbortError" && isCurrent()) {
                status("Preview request failed.", "error");
            }
        }
    };

    let onLiveChange = null;
    const applyLive = (on) => {
        onLiveChange?.(on);
        if (on) {
            node.rtSchedulePreview();
        } else {
            clearTimeout(previewTimer);
            status("Automatic previews paused for every Retro Tape node. Press Refresh.");
        }
    };
    liveListeners.add(applyLive);

    return {
        status,
        dropCompare,
        schedule: () => node.rtSchedulePreview(),
        isLive: () => livePreview,
        setLive: (on) => setLivePreview(on),
        bindLiveIndicator(callback) {
            onLiveChange = callback;
        },
        start() {
            if (!livePreview) {
                status("Paused. Press Refresh to render.");
                return;
            }
            node.rtRefreshPreview();
        },
        dispose() {
            disposed = true;
            liveListeners.delete(applyLive);
            clearTimeout(previewTimer);
            node.rtAbort?.abort();
            dropCompare();
            if (objectUrl) {
                URL.revokeObjectURL(objectUrl);
                objectUrl = null;
            }
        },
    };
}

function wireActionBar(ctx, store, preview) {
    const { ui, catalog } = ctx;

    ui.preset.addEventListener("change", () => {
        if (!catalog.presets[ui.preset.value]) {
            ui.preset.value = ctx.state.preset;
            return;
        }
        store.applyPreset(ui.preset.value);
    });
    // Reset does this, not the dropdown: a <select> fires click when it is opened.
    ui.reset.addEventListener("click", () => store.applyPreset(ctx.state.base_preset));
    ui.refresh.addEventListener("click", () => ctx.node.rtRefreshPreview());

    ui.compare.addEventListener("pointerdown", (event) => {
        try {
            ui.compare.setPointerCapture(event.pointerId);
        } catch {
            // Without capture, pointerleave still releases the compare.
        }
        ctx.node.rtShowSource(true);
    });
    for (const name of ["pointerup", "pointercancel", "pointerleave"]) {
        ui.compare.addEventListener(name, () => ctx.node.rtShowSource(false));
    }

    ui.copy.addEventListener("click", async () => {
        const copied = await copyText(JSON.stringify(ctx.state, null, 2));
        preview.status(
            copied ? "Settings JSON copied." : "Copy failed; select the JSON manually.",
            copied ? "ready" : "error",
            { hold: true },
        );
    });

    // A copied node is JSON too, so only an object carrying settings is one.
    const parseSettingsText = (text) => {
        let parsed;
        try {
            parsed = JSON.parse(text);
        } catch {
            return null;
        }
        if (!parsed || typeof parsed !== "object" || Array.isArray(parsed)) {
            return null;
        }
        return Object.keys(parsed).some((key) => SETTING_KEYS.has(key)) ? parsed : null;
    };

    const applySettings = (parsed) => {
        const undoable = takeUndoSnapshot();
        store.setState(parsed);
        takeUndoSnapshot();
        const pasted = `Settings pasted: ${ctx.state.preset}.`;
        preview.status(undoable ? `${pasted} Ctrl+Z undoes it.` : pasted, "ready", { hold: true });
    };

    ui.paste.addEventListener("click", async () => {
        const text = await readClipboardText();
        if (text == null) {
            // The desktop build blocks both the clipboard read and the prompt.
            preview.status("Clipboard is blocked here - press Ctrl+V now.", "error", { hold: true });
            ui.paste.focus();
            return;
        }
        const parsed = parseSettingsText(text);
        if (!parsed) {
            preview.status("That is not settings JSON.", "error", { hold: true });
            return;
        }
        applySettings(parsed);
    });
    ui.root.addEventListener("paste", (event) => {
        const parsed = parseSettingsText(event.clipboardData?.getData("text") ?? "");
        if (!parsed) {
            return;
        }
        event.preventDefault();
        event.stopPropagation();
        applySettings(parsed);
    });

    const syncLiveButton = (live = preview.isLive()) => {
        ui.live.textContent = live ? "Live" : "Paused";
        ui.live.setAttribute("aria-pressed", String(live));
        ui.live.title = live
            ? "Previewing on every change, in every Retro Tape node. Click to pause them all."
            : "Automatic previews are off in every Retro Tape node. Refresh renders one.";
    };
    syncLiveButton();
    preview.bindLiveIndicator(syncLiveButton);
    ui.live.addEventListener("click", () => preview.setLive(!preview.isLive()));
}

function installNodeHooks(ctx, store, preview) {
    const { node } = ctx;

    // LiteGraph restores saved widget values after the node is created.
    node.rtSyncFromWidget = function () {
        store.loadFromWidget();
        preview.schedule();
    };
    node.rtResetSettings = () => store.applyPreset(ctx.state.base_preset);
    node.rtCopySettings = () => copyText(JSON.stringify(ctx.state, null, 2));
    node.rtClearCache = async () => {
        try {
            const response = await api.fetchApi(CLEAR_CACHE_ROUTE, { method: "POST" });
            if (!response.ok) {
                throw new Error(String(response.status));
            }
            const freed = (await response.json())?.freed_bytes ?? 0;
            // One cache for the whole server, so the figure is not this node's.
            preview.status(
                `Preview cache cleared for every node, ${(freed / 1048576).toFixed(1)} MB freed.`,
                "",
                { hold: true },
            );
        } catch {
            preview.status("Could not clear the preview cache.", "error", { hold: true });
        }
    };

    // Changing an upstream file leaves the link alone, so onConnectionsChange never fires.
    let fingerprint = sourceFingerprint(node);
    const onGraphChanged = () => {
        const next = sourceFingerprint(node);
        if (next === fingerprint) {
            return;
        }
        fingerprint = next;
        preview.schedule();
    };
    api.addEventListener("graphChanged", onGraphChanged);

    chainCallback(node, "onRemoved", () => {
        api.removeEventListener("graphChanged", onGraphChanged);
        preview.dispose();
    });
}

function buildWidget(node, settingsWidget) {
    installStyle();
    const ctx = {
        node,
        settingsWidget,
        panelId: ++panelSerial,
        controls: new Map(),
        ui: createPanelShell(),
    };
    setStatus(ctx.ui.status, "Loading presets...");
    // The node can be deleted before the catalog lands; nothing is wired yet.
    chainCallback(node, "onRemoved", () => {
        ctx.removed = true;
    });
    loadCatalog().catch(() => fallbackCatalog(settingsWidget)).then((catalog) => {
        if (ctx.removed) {
            return;
        }
        ctx.catalog = catalog;
        const preview = createPreviewController(ctx);
        const store = createSettingsStore(ctx, preview);
        store.adopt(parseWidgetRaw(settingsWidget));
        fillPresetOptions(ctx.ui.preset, catalog, ctx.state.preset);
        buildControlRows(ctx, store);
        wireActionBar(ctx, store, preview);
        installNodeHooks(ctx, store, preview);
        store.syncControls();
        preview.start();
        if (catalog.degraded) {
            ctx.ui.preset.disabled = true;
            ctx.ui.preset.title = "The presets route did not answer.";
            ctx.ui.reset.disabled = true;
            ctx.ui.reset.title = "The presets route did not answer, so there is nothing to put back.";
            preview.status(
                "Presets unavailable; edits keep the saved settings and touch only what you move.",
                "error",
                { hold: true },
            );
        }
    });
    return ctx.ui.root;
}

app.registerExtension({
    name: EXTENSION_NAME,
    async beforeRegisterNodeDef(nodeType, nodeData) {
        if (!NODE_NAMES.includes(nodeData?.name)) {
            return;
        }

        chainCallback(nodeType.prototype, "onNodeCreated", function () {
            const settingsWidget = this.widgets?.find((widget) => widget.name === "settings_json");
            hideSettingsWidget(settingsWidget);

            const deviceWidget = this.widgets?.find((widget) => widget.name === "device");
            if (deviceWidget) {
                chainCallback(deviceWidget, "callback", () => this.rtSchedulePreview?.());
            }

            const root = buildWidget(this, settingsWidget);
            const widget = this.addDOMWidget("retrotape_preview", "preview", root, {
                serialize: false,
                // A floor, not a fixed height: computeSize would tie the minimum to the current height.
                getMinHeight: () => MIN_WIDGET_HEIGHT,
                getMaxHeight: () => MAX_WIDGET_HEIGHT,
                getValue() {
                    return null;
                },
                setValue() {},
            });
            // The DOM widget manager reads the property, not the option.
            widget.serialize = false;
            this.size[0] = Math.max(this.size[0], MIN_NODE_WIDTH);
            this.size[1] = Math.max(this.size[1], MIN_NODE_HEIGHT);
        });

        chainCallback(nodeType.prototype, "onResize", function (size) {
            if (!size) {
                return;
            }
            size[0] = Math.max(size[0], MIN_NODE_WIDTH);
            size[1] = Math.max(size[1], MIN_NODE_HEIGHT);
        });

        chainCallback(nodeType.prototype, "onConfigure", function () {
            this.rtSyncFromWidget?.();
        });

        chainCallback(nodeType.prototype, "onExecuted", function (message) {
            const executionId = message?.retrotape_preview_ready?.[0];
            if (executionId) {
                this.rtExecutionId = String(executionId);
            }
            this.rtRefreshPreview?.();
        });

        chainCallback(nodeType.prototype, "onConnectionsChange", function () {
            this.rtSchedulePreview?.();
        });

        chainCallback(nodeType.prototype, "getExtraMenuOptions", function (_, options) {
            options.push(
                null,
                {
                    content: "Refresh tape preview",
                    callback: () => this.rtRefreshPreview?.(),
                },
                {
                    content: "Reset tape settings",
                    callback: () => this.rtResetSettings?.(),
                },
                {
                    content: "Copy tape settings JSON",
                    callback: () => this.rtCopySettings?.(),
                },
                {
                    content: "Clear preview cache",
                    callback: () => this.rtClearCache?.(),
                },
            );
        });
    },
});
