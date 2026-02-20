#!/usr/bin/env python3
"""Valida presença e integridade básica dos bootstraps versionados no app."""

from __future__ import annotations

import hashlib
import sys
import tarfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BOOTSTRAP_DIR = ROOT / "app" / "src" / "main" / "assets" / "bootstrap"
REQUIRED_BOOTSTRAPS = [
    "arm64-v8a.tar",
    "armeabi-v7a.tar",
    "x86.tar",
    "x86_64.tar",
]


def sha256_prefix(path: Path, limit_bytes: int = 1024 * 1024) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        remaining = limit_bytes
        while remaining > 0:
            chunk = handle.read(min(65536, remaining))
            if not chunk:
                break
            digest.update(chunk)
            remaining -= len(chunk)
    return digest.hexdigest()[:16]


def validate_tar(path: Path) -> tuple[int, str]:
    with tarfile.open(path, "r") as archive:
        members = archive.getmembers()
        if not members:
            raise RuntimeError("arquivo tar vazio")
        return len(members), members[0].name


def main() -> int:
    print("[verify_bootstrap_assets] Validando bootstraps do repositório...")

    if not BOOTSTRAP_DIR.exists():
        print(f"[verify_bootstrap_assets] FALHA: diretório ausente: {BOOTSTRAP_DIR.relative_to(ROOT)}")
        return 1

    failures: list[str] = []

    for name in REQUIRED_BOOTSTRAPS:
        path = BOOTSTRAP_DIR / name
        if not path.exists():
            failures.append(f"ausente: {path.relative_to(ROOT)}")
            continue

        size = path.stat().st_size
        if size <= 0:
            failures.append(f"vazio: {path.relative_to(ROOT)}")
            continue

        try:
            member_count, first_member = validate_tar(path)
        except (tarfile.TarError, RuntimeError) as exc:
            failures.append(f"inválido: {path.relative_to(ROOT)} ({exc})")
            continue

        print(
            f"  - OK {path.relative_to(ROOT)} size={size} entries={member_count} "
            f"sha256_prefix={sha256_prefix(path)} first_entry={first_member}"
        )

    if failures:
        print("\n[verify_bootstrap_assets] FALHAS:")
        for failure in failures:
            print(f"  - {failure}")
        return 1

    print("\n[verify_bootstrap_assets] OK: bootstraps essenciais estão versionados e íntegros.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
