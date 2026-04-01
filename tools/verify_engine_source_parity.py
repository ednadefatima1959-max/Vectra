#!/usr/bin/env python3
from __future__ import annotations

import platform
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "engine/rmr/sources.cmake"
MK_MANIFEST = ROOT / "engine/rmr/sources.mk"
ROOT_CMAKE = ROOT / "CMakeLists.txt"
APP_CMAKE = ROOT / "app/src/main/cpp/CMakeLists.txt"
MAKEFILE = ROOT / "Makefile"

SET_PATTERN = re.compile(r"^set\((\w+)\s*$")


def fail(msg: str) -> None:
    print(f"[source-parity] ERROR: {msg}")
    sys.exit(1)


def parse_cmake_sets(path: Path) -> dict[str, list[str]]:
    sets: dict[str, list[str]] = {}
    current_name: str | None = None
    current_values: list[str] = []

    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.split("#", 1)[0].strip()
        if not line:
            continue

        if current_name is None:
            m = SET_PATTERN.match(line)
            if m:
                current_name = m.group(1)
                current_values = []
            continue

        if line == ")":
            sets[current_name] = current_values
            current_name = None
            current_values = []
            continue

        current_values.append(line)

    if current_name is not None:
        fail(f"unterminated set() block for {current_name} in {path}")

    return sets


def parse_make_plus_assignments(path: Path, var: str) -> list[str]:
    values: list[str] = []
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if line.startswith(f"{var} += "):
            values.append(line.split("+=", 1)[1].strip())
    return values


def ensure_contains(path: Path, snippet: str) -> None:
    if snippet not in path.read_text(encoding="utf-8"):
        fail(f"expected snippet missing in {path}: {snippet}")


def with_policy_and_host_asm(manifest: dict[str, list[str]]) -> list[str]:
    result = list(manifest["RMR_ENGINE_CORE_SOURCES"])
    result.extend(manifest["RMR_ENGINE_POLICY_SOURCES"])

    machine = platform.machine().lower()
    if machine in {"x86_64", "amd64"}:
        result.extend(manifest["RMR_ENGINE_ASM_X86_64_LOWLEVEL_SOURCES"])
        result.extend(manifest["RMR_ENGINE_ASM_X86_64_CASM_SOURCES"])
    elif machine in {"aarch64", "arm64"}:
        result.extend(manifest["RMR_ENGINE_ASM_ARM64_SOURCES"])
    elif machine == "riscv64":
        result.extend(manifest["RMR_ENGINE_ASM_RISCV64_SOURCES"])

    deduped: list[str] = []
    seen: set[str] = set()
    for src in result:
        if src not in seen:
            deduped.append(src)
            seen.add(src)
    return deduped


def main() -> None:
    manifest = parse_cmake_sets(MANIFEST)

    required = [
        "RMR_ENGINE_CORE_SOURCES",
        "RMR_ENGINE_POLICY_SOURCES",
        "RMR_ENGINE_ASM_X86_64_LOWLEVEL_SOURCES",
        "RMR_ENGINE_ASM_X86_64_CASM_SOURCES",
        "RMR_ENGINE_ASM_ARM64_SOURCES",
        "RMR_ENGINE_ASM_RISCV64_SOURCES",
    ]
    for key in required:
        if key not in manifest:
            fail(f"{key} missing from {MANIFEST}")

    for key in required:
        mk_values = parse_make_plus_assignments(MK_MANIFEST, key)
        if mk_values != manifest[key]:
            fail(f"{key} differs between sources.cmake and sources.mk")

    cmake_expected = with_policy_and_host_asm(manifest)
    make_expected = with_policy_and_host_asm(manifest)
    if cmake_expected != make_expected:
        fail("derived root CMake and Make source lists differ")

    ensure_contains(ROOT_CMAKE, 'include(${CMAKE_SOURCE_DIR}/engine/rmr/sources.cmake)')
    ensure_contains(APP_CMAKE, 'include(${VECTRA_REPO_ROOT}/engine/rmr/sources.cmake)')
    ensure_contains(MAKEFILE, "include engine/rmr/sources.mk")

    print("[source-parity] OK: canonical manifest, root CMake include path, and Make lists are aligned")


if __name__ == "__main__":
    main()
