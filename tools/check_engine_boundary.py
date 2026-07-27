#!/usr/bin/env python3
"""Fail when engine source directly includes application source."""

from __future__ import annotations

import re
import sys
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[1]
ENGINE_ROOT = REPOSITORY_ROOT / "project" / "engine"
SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".ixx"}
INCLUDE_PATTERN = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]', re.MULTILINE)


def main() -> int:
    violations: list[str] = []
    for source_path in sorted(ENGINE_ROOT.rglob("*")):
        if not source_path.is_file() or source_path.suffix.lower() not in SOURCE_SUFFIXES:
            continue

        source = source_path.read_text(encoding="utf-8-sig", errors="replace")
        for include_path in INCLUDE_PATTERN.findall(source):
            normalized = include_path.replace("\\", "/").lower()
            if normalized.startswith("application/") or "/application/" in normalized:
                relative_source = source_path.relative_to(REPOSITORY_ROOT)
                violations.append(f"{relative_source}: includes {include_path}")

    if violations:
        print("Engine boundary violations found:", file=sys.stderr)
        for violation in violations:
            print(f"  {violation}", file=sys.stderr)
        return 1

    print("Engine boundary check passed: project/engine has no application includes.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
