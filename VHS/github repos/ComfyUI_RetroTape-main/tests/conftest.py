from __future__ import annotations

import sys
from pathlib import Path
from types import SimpleNamespace

CUSTOM_NODES_DIR = Path(__file__).resolve().parents[2]
if str(CUSTOM_NODES_DIR) not in sys.path:
    sys.path.insert(0, str(CUSTOM_NODES_DIR))


def fake_folder_paths(directory):
    """A folder_paths stand-in whose every folder is `directory`."""
    return SimpleNamespace(
        get_directory_by_type=lambda folder_type: str(directory) if folder_type == "input" else None,
        get_input_directory=lambda: str(directory),
        get_annotated_filepath=lambda name, default_dir=None: str(directory / name),
    )


class _Routes:
    """The route table PromptServer would hand the module, keyed by method."""

    def __init__(self):
        self.handlers = {}

    def _register(self, method, path):
        def register(handler):
            self.handlers[(method, path)] = handler
            return handler

        return register

    def get(self, path):
        return self._register("GET", path)

    def post(self, path):
        return self._register("POST", path)


def route_handlers(monkeypatch):
    from ComfyUI_RetroTape import server

    routes = _Routes()
    instance = SimpleNamespace(routes=routes, app=SimpleNamespace(router=SimpleNamespace(frozen=False)))
    monkeypatch.setattr(server, "_get_prompt_server_class", lambda: SimpleNamespace(instance=instance))
    assert server.register_routes()
    return routes.handlers
