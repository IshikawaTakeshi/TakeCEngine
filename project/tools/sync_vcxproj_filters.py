#!/usr/bin/env python3
"""
Synchronize a Visual Studio .vcxproj.filters file from the file paths in a
.vcxproj file.

The generated filters mirror the project-relative directory structure:

    engine\\AI\\OnnxModel.cpp -> filter "engine\\AI"

Items without a directory remain at the project root. Existing custom filter
grouping is intentionally replaced so the result is deterministic and easy to
re-run after file additions.
"""

from __future__ import annotations

import argparse
import difflib
import shutil
import sys
import uuid
import xml.etree.ElementTree as ET
from collections import OrderedDict
from pathlib import PureWindowsPath
from typing import Iterable


MSBUILD_NS = "http://schemas.microsoft.com/developer/msbuild/2003"
ET.register_namespace("", MSBUILD_NS)

FILTERABLE_ITEM_TAGS = {
    "ClCompile",
    "ClInclude",
    "FxCompile",
    "None",
    "Image",
    "Text",
    "Xml",
    "ResourceCompile",
    "CustomBuild",
    "Content",
}


def qname(tag: str) -> str:
    return f"{{{MSBUILD_NS}}}{tag}"


def local_name(tag: str) -> str:
    if tag.startswith("{"):
        return tag.rsplit("}", 1)[1]
    return tag


def normalize_include(path: str) -> str:
    return path.replace("/", "\\")


def filter_for_include(include: str) -> str | None:
    include = normalize_include(include)

    # MSBuild property paths are usually imported package content. Keep them at
    # root instead of creating filters like "$(MSBuildThisFileDirectory)..".
    if "$(" in include:
        return None

    parent = str(PureWindowsPath(include).parent)
    if parent in ("", "."):
        return None
    return parent


def iter_project_items(project_root: ET.Element) -> Iterable[tuple[str, str]]:
    for item in project_root.iter():
        tag = local_name(item.tag)
        if tag not in FILTERABLE_ITEM_TAGS:
            continue

        include = item.attrib.get("Include")
        if not include:
            continue

        yield tag, normalize_include(include)


def add_text_element(parent: ET.Element, tag: str, text: str) -> ET.Element:
    element = ET.SubElement(parent, qname(tag))
    element.text = text
    return element


def deterministic_guid(filter_name: str) -> str:
    value = uuid.uuid5(uuid.NAMESPACE_URL, f"TakeCEngine.vcxproj.filters/{filter_name}")
    return "{" + str(value).upper() + "}"


def collect_filters(items: Iterable[tuple[str, str]]) -> list[str]:
    filters: OrderedDict[str, None] = OrderedDict()

    for _, include in items:
        filter_name = filter_for_include(include)
        if not filter_name:
            continue

        parts = filter_name.split("\\")
        for index in range(1, len(parts) + 1):
            filters.setdefault("\\".join(parts[:index]), None)

    return list(filters.keys())


def build_filters_xml(project_items: list[tuple[str, str]]) -> ET.ElementTree:
    root = ET.Element(qname("Project"), {
        "ToolsVersion": "4.0",
    })

    filters = collect_filters(project_items)
    if filters:
        item_group = ET.SubElement(root, qname("ItemGroup"))
        for filter_name in filters:
            filter_element = ET.SubElement(item_group, qname("Filter"), {
                "Include": filter_name,
            })
            add_text_element(filter_element, "UniqueIdentifier", deterministic_guid(filter_name))

    grouped_items: OrderedDict[str, list[str]] = OrderedDict()
    for tag, include in project_items:
        grouped_items.setdefault(tag, []).append(include)

    for tag, includes in grouped_items.items():
        item_group = ET.SubElement(root, qname("ItemGroup"))
        seen: set[str] = set()
        for include in includes:
            key = f"{tag}:{include}"
            if key in seen:
                continue
            seen.add(key)

            item = ET.SubElement(item_group, qname(tag), {
                "Include": include,
            })
            filter_name = filter_for_include(include)
            if filter_name:
                add_text_element(item, "Filter", filter_name)

    ET.indent(root, space="  ")
    return ET.ElementTree(root)


def xml_to_string(tree: ET.ElementTree) -> str:
    return ET.tostring(tree.getroot(), encoding="unicode", xml_declaration=False)


def write_xml(tree: ET.ElementTree, path: str) -> None:
    body = xml_to_string(tree)
    with open(path, "w", encoding="utf-8", newline="\r\n") as file:
        file.write('<?xml version="1.0" encoding="utf-8"?>\n')
        file.write(body)
        file.write("\n")


def read_text(path: str) -> str:
    with open(path, "r", encoding="utf-8-sig") as file:
        return file.read()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Regenerate a .vcxproj.filters file from .vcxproj item paths.",
    )
    parser.add_argument(
        "project",
        nargs="?",
        default="DirectXGame.vcxproj",
        help="Path to the .vcxproj file.",
    )
    parser.add_argument(
        "filters",
        nargs="?",
        default=None,
        help="Path to the .vcxproj.filters file. Defaults to <project>.filters.",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="Only report whether the filters file is up to date.",
    )
    parser.add_argument(
        "--diff",
        action="store_true",
        help="Print a unified diff instead of writing.",
    )
    parser.add_argument(
        "--no-backup",
        action="store_true",
        help="Do not create <filters>.bak before overwriting.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    project_path = args.project
    filters_path = args.filters or f"{project_path}.filters"

    project_tree = ET.parse(project_path)
    project_items = list(iter_project_items(project_tree.getroot()))
    filters_tree = build_filters_xml(project_items)
    generated = '<?xml version="1.0" encoding="utf-8"?>\n' + xml_to_string(filters_tree) + "\n"

    try:
        current = read_text(filters_path)
    except FileNotFoundError:
        current = ""

    if args.check:
        if current == generated:
            print(f"{filters_path}: up to date")
            return 0
        print(f"{filters_path}: out of date")
        return 1

    if args.diff:
        diff = difflib.unified_diff(
            current.splitlines(keepends=True),
            generated.splitlines(keepends=True),
            fromfile=filters_path,
            tofile=f"{filters_path} (generated)",
        )
        sys.stdout.writelines(diff)
        return 0

    if current == generated:
        print(f"{filters_path}: up to date")
        return 0

    if current and not args.no_backup:
        backup_path = f"{filters_path}.bak"
        shutil.copy2(filters_path, backup_path)
        print(f"backup: {backup_path}")

    write_xml(filters_tree, filters_path)
    print(f"updated: {filters_path}")
    print(f"items: {len(project_items)}")
    print(f"filters: {len(collect_filters(project_items))}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
