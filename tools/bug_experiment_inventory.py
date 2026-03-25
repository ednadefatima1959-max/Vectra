#!/usr/bin/env python3
"""Inventário e migração gradual de artefatos de bug para bug/experiments/."""

from __future__ import annotations

import argparse
import datetime as dt
import json
import re
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
BUG_DIR = ROOT / "bug"
EXPERIMENTS_DIR = BUG_DIR / "experiments"
DATASET_ERRORS = ROOT / "dataset" / "errors" / "error_samples.jsonl"
OUTPUT_PATH = EXPERIMENTS_DIR / "inventory.json"

IGNORE_NAMES = {
    "README.md",
    "FILES_MAP.md",
}


def slugify(name: str) -> str:
    slug = re.sub(r"[^a-zA-Z0-9]+", "-", name.lower()).strip("-")
    return slug or "experiment"


def load_dataset_error_ids(path: Path) -> set[str]:
    ids: set[str] = set()
    if not path.exists():
        return ids
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line:
            continue
        row = json.loads(line)
        row_id = row.get("id")
        if isinstance(row_id, str) and row_id:
            ids.add(row_id)
    return ids


def find_legacy_artifacts(bug_dir: Path) -> list[Path]:
    artifacts: list[Path] = []
    for path in sorted(bug_dir.iterdir()):
        if path.name == "experiments":
            continue
        if not path.is_file():
            continue
        if path.name in IGNORE_NAMES:
            continue
        if path.suffix.lower() not in {".md", ".txt"}:
            continue
        artifacts.append(path)
    return artifacts


def find_experiment_metadata(experiments_dir: Path) -> list[Path]:
    return sorted(experiments_dir.glob("*/*/metadata.json"))


def read_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def ensure_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def build_stub_metadata(artifact_rel: str, title: str, status: str) -> dict[str, Any]:
    return {
        "context": {
            "title": title,
            "source_artifacts": [artifact_rel],
            "owner": "unassigned",
            "created_at": dt.date.today().isoformat(),
        },
        "input": {
            "scenario": f"Migrated from {artifact_rel}",
            "error_dataset_id": None,
        },
        "expected": {
            "behavior": "Definir comportamento esperado durante a normalização.",
        },
        "observed": {
            "behavior": "Artefato legado inventariado e pendente de curadoria.",
        },
        "invariant_broken": None,
        "status": status,
    }


def migrate(
    artifacts: list[Path],
    experiments_dir: Path,
    limit: int,
    status: str,
    apply: bool,
) -> list[dict[str, str]]:
    migrated: list[dict[str, str]] = []
    if limit <= 0:
        return migrated

    target_root = experiments_dir / status
    ensure_dir(target_root)

    for artifact in artifacts[:limit]:
        slug = slugify(artifact.stem)
        exp_dir = target_root / slug
        metadata_path = exp_dir / "metadata.json"
        if metadata_path.exists():
            continue

        artifact_rel = artifact.relative_to(ROOT).as_posix()
        payload = build_stub_metadata(
            artifact_rel=artifact_rel,
            title=artifact.stem,
            status=status,
        )
        migrated.append(
            {
                "source_artifact": artifact_rel,
                "metadata_path": metadata_path.relative_to(ROOT).as_posix(),
            }
        )

        if apply:
            ensure_dir(exp_dir)
            metadata_path.write_text(
                json.dumps(payload, ensure_ascii=False, indent=2) + "\n",
                encoding="utf-8",
            )

    return migrated


def collect_experiment_state(
    metadata_paths: list[Path],
    dataset_error_ids: set[str],
) -> tuple[list[dict[str, Any]], list[dict[str, str]]]:
    experiments: list[dict[str, Any]] = []
    failed_links: list[dict[str, str]] = []

    for meta_path in metadata_paths:
        data = read_json(meta_path)
        rel_meta = meta_path.relative_to(ROOT).as_posix()
        status = data.get("status")
        input_obj = data.get("input") if isinstance(data.get("input"), dict) else {}
        error_dataset_id = input_obj.get("error_dataset_id")

        item = {
            "metadata_path": rel_meta,
            "status": status,
            "context_title": (data.get("context") or {}).get("title"),
            "error_dataset_id": error_dataset_id,
        }
        experiments.append(item)

        if status == "failed":
            if not isinstance(error_dataset_id, str) or not error_dataset_id:
                failed_links.append(
                    {
                        "metadata_path": rel_meta,
                        "status": "missing_error_dataset_id",
                    }
                )
            elif error_dataset_id not in dataset_error_ids:
                failed_links.append(
                    {
                        "metadata_path": rel_meta,
                        "status": "dataset_id_not_found",
                        "error_dataset_id": error_dataset_id,
                    }
                )
            else:
                failed_links.append(
                    {
                        "metadata_path": rel_meta,
                        "status": "linked",
                        "error_dataset_id": error_dataset_id,
                    }
                )

    return experiments, failed_links


def build_report(
    legacy_artifacts: list[Path],
    metadata_paths: list[Path],
    migrated: list[dict[str, str]],
    experiments: list[dict[str, Any]],
    failed_links: list[dict[str, str]],
    dataset_error_count: int,
) -> dict[str, Any]:
    return {
        "generated_at": dt.datetime.utcnow().replace(microsecond=0).isoformat() + "Z",
        "summary": {
            "legacy_artifacts_root_bug": len(legacy_artifacts),
            "experiment_metadata_files": len(metadata_paths),
            "migrated_in_run": len(migrated),
            "dataset_error_ids": dataset_error_count,
            "failed_links_total": len(failed_links),
            "failed_links_linked": sum(1 for x in failed_links if x.get("status") == "linked"),
        },
        "migrated": migrated,
        "legacy_artifacts": [p.relative_to(ROOT).as_posix() for p in legacy_artifacts],
        "experiments": experiments,
        "failed_links": failed_links,
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--apply", action="store_true", help="Escreve mudanças em disco.")
    parser.add_argument(
        "--migrate-limit",
        type=int,
        default=0,
        help="Quantidade máxima de artefatos legados para migrar nesta execução.",
    )
    parser.add_argument(
        "--status-target",
        choices=["unstable", "failed", "validated"],
        default="unstable",
        help="Status/subárvore alvo para a migração gradual.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    dataset_error_ids = load_dataset_error_ids(DATASET_ERRORS)
    legacy_artifacts = find_legacy_artifacts(BUG_DIR)

    migrated = migrate(
        artifacts=legacy_artifacts,
        experiments_dir=EXPERIMENTS_DIR,
        limit=args.migrate_limit,
        status=args.status_target,
        apply=args.apply,
    )

    metadata_paths = find_experiment_metadata(EXPERIMENTS_DIR)
    experiments, failed_links = collect_experiment_state(metadata_paths, dataset_error_ids)

    report = build_report(
        legacy_artifacts=legacy_artifacts,
        metadata_paths=metadata_paths,
        migrated=migrated,
        experiments=experiments,
        failed_links=failed_links,
        dataset_error_count=len(dataset_error_ids),
    )

    if args.apply:
        ensure_dir(OUTPUT_PATH.parent)
        OUTPUT_PATH.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        print(f"[ok] wrote {OUTPUT_PATH.relative_to(ROOT).as_posix()}")
    else:
        print(json.dumps(report, ensure_ascii=False, indent=2))

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
