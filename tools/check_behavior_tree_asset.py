#!/usr/bin/env python3
"""Validate the BehaviorTreeAsset JSON contract and legacy migration shape."""

from __future__ import annotations

import copy
import json
import tempfile
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[1]
LAYOUT_KEYS = {"posX", "posY", "sizeW", "sizeH"}


def strip_layout(node: dict) -> dict:
    result = copy.deepcopy(node)
    for key in LAYOUT_KEYS:
        result.pop(key, None)
    result["children"] = [strip_layout(child) for child in result.get("children", [])]
    return result


def migrate_legacy(data: dict) -> dict:
    root = {
        "name": data["setName"],
        "nodeType": data.get("rootType") or "SELECTOR",
        "nodeUID": -1,
        "children": [],
    }
    for combo in data["combos"]:
        child = copy.deepcopy(combo["rootNode"])
        if combo.get("comboName"):
            child["name"] = combo["comboName"]
        root["children"].append(child)

    editor_nodes = []
    for legacy_node in data.get("editorNodes", []):
        editor_nodes.append(
            {
                "node": strip_layout(legacy_node),
                "posX": legacy_node.get("posX", 0.0),
                "posY": legacy_node.get("posY", 0.0),
                "sizeW": legacy_node.get("sizeW", 0.0),
                "sizeH": legacy_node.get("sizeH", 0.0),
            }
        )

    return {
        "formatVersion": 1,
        "name": data["setName"],
        "root": strip_layout(root),
        "editor": {
            "nodes": editor_nodes,
            "links": copy.deepcopy(data.get("editorLinks", [])),
        },
    }


def assert_layout_is_separate(node: dict) -> None:
    assert not (LAYOUT_KEYS & node.keys()), f"runtime node contains layout keys: {node}"
    for child in node.get("children", []):
        assert_layout_is_separate(child)


def validation_codes(asset: dict, registered_types: set[str]) -> set[str]:
    codes: set[str] = set()
    if not 0 < asset.get("formatVersion", 0) <= 1:
        codes.add("UNSUPPORTED_FORMAT_VERSION")

    runtime_uids: set[int] = set()

    def visit_runtime(node: dict) -> None:
        if node.get("nodeType", "") not in registered_types:
            codes.add("UNKNOWN_NODE_TYPE")
        uid = node.get("nodeUID", -1)
        if uid >= 0 and uid in runtime_uids:
            codes.add("DUPLICATE_RUNTIME_UID")
        runtime_uids.add(uid)
        for child in node.get("children", []):
            visit_runtime(child)

    visit_runtime(asset["root"])

    editor_uids: set[int] = set()
    adjacency: dict[int, list[int]] = {}
    incoming: dict[int, int] = {}
    for editor_node in asset.get("editor", {}).get("nodes", []):
        node = editor_node["node"]
        uid = node.get("nodeUID", -1)
        if uid < 0:
            codes.add("INVALID_EDITOR_UID")
        elif uid in editor_uids:
            codes.add("DUPLICATE_EDITOR_UID")
        editor_uids.add(uid)
        if node.get("nodeType", "") not in registered_types:
            codes.add("UNKNOWN_EDITOR_NODE_TYPE")

    for link in asset.get("editor", {}).get("links", []):
        source = link["fromNodeUID"]
        target = link["toNodeUID"]
        if source not in editor_uids:
            codes.add("MISSING_LINK_SOURCE")
        if target not in editor_uids:
            codes.add("MISSING_LINK_TARGET")
        if source == target:
            codes.add("SELF_LINK")
        if source in editor_uids and target in editor_uids:
            adjacency.setdefault(source, []).append(target)
            incoming[target] = incoming.get(target, 0) + 1
            if incoming[target] > 1:
                codes.add("MULTIPLE_PARENTS")

    visiting: set[int] = set()
    visited: set[int] = set()

    def detect_cycle(uid: int) -> None:
        if uid in visiting:
            codes.add("EDITOR_CYCLE")
            return
        if uid in visited:
            return
        visiting.add(uid)
        for child in adjacency.get(uid, []):
            detect_cycle(child)
        visiting.remove(uid)
        visited.add(uid)

    for uid in editor_uids:
        detect_cycle(uid)
    return codes


def main() -> int:
    legacy = {
        "setName": "EnemyCombat",
        "rootType": "SELECTOR",
        "combos": [
            {
                "comboName": "AttackSequence",
                "rootNode": {
                    "name": "OldName",
                    "nodeType": "SEQUENCE",
                    "nodeUID": 10,
                    "posX": 12.0,
                    "posY": 24.0,
                    "sizeW": 180.0,
                    "sizeH": 80.0,
                    "children": [
                        {
                            "name": "Attack",
                            "nodeType": "ACTION",
                            "nodeUID": 11,
                            "targetState": "ATTACK",
                            "children": [],
                        }
                    ],
                },
            }
        ],
        "editorNodes": [
            {
                "name": "AttackSequence",
                "nodeType": "SEQUENCE",
                "nodeUID": 10,
                "posX": 12.0,
                "posY": 24.0,
                "sizeW": 180.0,
                "sizeH": 80.0,
                "children": [],
            }
        ],
        "editorLinks": [],
    }

    migrated = migrate_legacy(legacy)
    assert migrated["formatVersion"] == 1
    assert migrated["root"]["children"][0]["name"] == "AttackSequence"
    assert migrated["editor"]["nodes"][0]["posX"] == 12.0
    assert_layout_is_separate(migrated["root"])
    assert_layout_is_separate(migrated["editor"]["nodes"][0]["node"])

    registered_types = {"SELECTOR", "SEQUENCE", "ACTION"}
    assert validation_codes(migrated, registered_types) == set()

    invalid = copy.deepcopy(migrated)
    invalid["formatVersion"] = 99
    invalid["editor"]["nodes"].append(copy.deepcopy(invalid["editor"]["nodes"][0]))
    child_editor_node = copy.deepcopy(invalid["editor"]["nodes"][0])
    child_editor_node["node"]["nodeUID"] = 11
    child_editor_node["node"]["nodeType"] = "ACTION"
    invalid["editor"]["nodes"].append(child_editor_node)
    invalid["editor"]["links"] = [
        {"fromNodeUID": 10, "fromPinIndex": 0, "toNodeUID": 10, "toPinIndex": 0},
        {"fromNodeUID": 11, "fromPinIndex": 0, "toNodeUID": 10, "toPinIndex": 0},
        {"fromNodeUID": 999, "fromPinIndex": 0, "toNodeUID": 10, "toPinIndex": 0},
    ]
    invalid_codes = validation_codes(invalid, registered_types)
    assert {
        "UNSUPPORTED_FORMAT_VERSION",
        "DUPLICATE_EDITOR_UID",
        "SELF_LINK",
        "EDITOR_CYCLE",
        "MISSING_LINK_SOURCE",
        "MULTIPLE_PARENTS",
    } <= invalid_codes

    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / "EnemyCombat.json"
        path.write_text(json.dumps(migrated, indent=4), encoding="utf-8")
        loaded = json.loads(path.read_text(encoding="utf-8"))
        assert loaded == migrated

    util_source = (
        REPOSITORY_ROOT / "project/engine/BehaviorTree/BehaviorTreeUtil.cpp"
    ).read_text(encoding="utf-8-sig")
    loader_header = (
        REPOSITORY_ROOT / "project/engine/Utility/JsonLoader.h"
    ).read_text(encoding="utf-8-sig")
    for marker in (
        "ConvertComboSetToBehaviorTreeAsset",
        "RemoveEditorLayout(rootJson)",
        'j.contains("combos")',
    ):
        assert marker in util_source, f"missing C++ migration marker: {marker}"
    for marker in ("SaveJsonDataAt", "LoadJsonDataAt", "GetJsonDataListAt"):
        assert marker in loader_header, f"missing injected-directory API: {marker}"

    validator_source = (
        REPOSITORY_ROOT / "project/engine/BehaviorTree/BehaviorTreeAssetValidator.cpp"
    ).read_text(encoding="utf-8-sig")
    for marker in invalid_codes:
        assert marker in validator_source, f"missing C++ validation marker: {marker}"

    print("BehaviorTreeAsset contract check passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
