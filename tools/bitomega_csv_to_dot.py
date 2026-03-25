#!/usr/bin/env python3
"""BitOmega CSV pipeline with schema validation and deterministic dataset outputs."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import pathlib
import re
import sys
from collections import Counter
from dataclasses import dataclass
from typing import Any

SCHEMA_VERSIONS: dict[str, dict[str, set[str]]] = {
  "v1": {
    "required": {"schema_version", "state_prev", "state_new", "direction"},
    "optional": {"input_pattern", "invariants", "expected_transition", "error_label", "row_id"},
  },
}

CANONICAL_STATES = {
  "neg",
  "zero",
  "pos",
  "mix",
  "void",
  "edge",
  "flow",
  "lock",
  "noise",
  "meta",
}

CANONICAL_DIRECTIONS = {"none", "up", "down", "forward", "recurse", "null"}

CANONICAL_PROCESSED_COLUMNS = (
  "input_pattern",
  "extracted_invariants",
  "expected_transition",
  "error_label",
)

DEFAULT_SPLIT_SEED = 0xB1706A


@dataclass
class ValidationIssue:
  row_number: int
  reason: str


@dataclass
class ParseResult:
  schema_version: str
  rows: list[dict[str, str]]
  invalid_rows: list[ValidationIssue]


class SchemaError(RuntimeError):
  pass


def parse_args(argv: list[str]) -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="BitOmega CSV low-level dataset toolchain")
  subparsers = parser.add_subparsers(dest="command", required=True)

  normalize = subparsers.add_parser("normalize", help="validate + normalize CSV into canonical ordering")
  normalize.add_argument("input_csv", type=pathlib.Path)
  normalize.add_argument("output_csv", type=pathlib.Path)
  normalize.add_argument("--quality-report", type=pathlib.Path)

  validate = subparsers.add_parser("validate", help="validate CSV schema and values")
  validate.add_argument("input_csv", type=pathlib.Path)
  validate.add_argument("--quality-report", type=pathlib.Path)

  export_dot = subparsers.add_parser("export-dot", help="export transitions to Graphviz DOT")
  export_dot.add_argument("input_csv", type=pathlib.Path)
  export_dot.add_argument("output_dot", type=pathlib.Path)
  export_dot.add_argument("--summary-md", type=pathlib.Path)
  export_dot.add_argument("--quality-report", type=pathlib.Path)

  export_jsonl = subparsers.add_parser("export-jsonl", help="export canonical processed JSONL")
  export_jsonl.add_argument("input_csv", type=pathlib.Path)
  export_jsonl.add_argument(
    "output_jsonl",
    nargs="?",
    default=pathlib.Path("dataset/processed/bitomega_canonical.jsonl"),
    type=pathlib.Path,
  )
  export_jsonl.add_argument("--quality-report", type=pathlib.Path)

  split = subparsers.add_parser("split", help="deterministic train/valid/test split")
  split.add_argument("input_jsonl", type=pathlib.Path)
  split.add_argument(
    "--output-dir",
    default=pathlib.Path("dataset/processed"),
    type=pathlib.Path,
    help="directory for train.jsonl, valid.jsonl, test.jsonl",
  )
  split.add_argument("--seed", type=int, default=DEFAULT_SPLIT_SEED)
  split.add_argument("--train-ratio", type=float, default=0.8)
  split.add_argument("--valid-ratio", type=float, default=0.1)
  split.add_argument("--quality-report", type=pathlib.Path)

  return parser.parse_args(argv)


def require_input(path: pathlib.Path) -> None:
  if not path.exists():
    raise SchemaError(f"missing input: {path}")


def canonical_token(value: str) -> str:
  return value.strip().lower()


def is_hex_or_name(value: str) -> bool:
  token = value.strip()
  return bool(re.fullmatch(r"(?:0x[0-9a-fA-F]+|[A-Za-z][A-Za-z0-9_\-]*)", token))


def parse_and_validate_csv(path: pathlib.Path) -> ParseResult:
  require_input(path)

  with path.open("r", encoding="utf-8", newline="") as fh:
    reader = csv.DictReader(fh)
    if reader.fieldnames is None:
      raise SchemaError("invalid CSV: missing header")

    header = {field.strip() for field in reader.fieldnames if field is not None}
    missing_schema = "schema_version" not in header
    if missing_schema:
      raise SchemaError("invalid CSV header: schema_version is required (supported: v1)")

    schema_version: str | None = None
    rows: list[dict[str, str]] = []
    invalid_rows: list[ValidationIssue] = []

    for idx, raw_row in enumerate(reader, start=2):
      row = {k.strip(): (v or "").strip() for k, v in raw_row.items() if k is not None}
      local_schema = row.get("schema_version", "")
      if not local_schema:
        invalid_rows.append(ValidationIssue(idx, "schema_version empty"))
        continue
      if local_schema not in SCHEMA_VERSIONS:
        invalid_rows.append(ValidationIssue(idx, f"unsupported schema_version={local_schema}"))
        continue
      if schema_version is None:
        schema_version = local_schema
      elif schema_version != local_schema:
        invalid_rows.append(ValidationIssue(idx, f"mixed schema versions: {schema_version} vs {local_schema}"))
        continue

      schema_def = SCHEMA_VERSIONS[local_schema]
      row_fields = set(row.keys())
      missing_required = sorted(schema_def["required"] - row_fields)
      if missing_required:
        invalid_rows.append(ValidationIssue(idx, f"missing required columns in row: {','.join(missing_required)}"))
        continue

      unknown_cols = sorted(row_fields - (schema_def["required"] | schema_def["optional"]))
      if unknown_cols:
        invalid_rows.append(ValidationIssue(idx, f"unknown columns: {','.join(unknown_cols)}"))
        continue

      prev = canonical_token(row.get("state_prev", ""))
      nxt = canonical_token(row.get("state_new", ""))
      direction = canonical_token(row.get("direction", ""))

      if prev not in CANONICAL_STATES:
        invalid_rows.append(ValidationIssue(idx, f"state_prev invalid: {row.get('state_prev', '')}"))
        continue
      if nxt not in CANONICAL_STATES:
        invalid_rows.append(ValidationIssue(idx, f"state_new invalid: {row.get('state_new', '')}"))
        continue
      if direction not in CANONICAL_DIRECTIONS:
        invalid_rows.append(ValidationIssue(idx, f"direction invalid: {row.get('direction', '')}"))
        continue

      invariants = row.get("invariants", "")
      if invariants:
        invariant_tokens = [token.strip() for token in invariants.split("|") if token.strip()]
        if not invariant_tokens:
          invalid_rows.append(ValidationIssue(idx, "invariants malformed"))
          continue
        if not all(is_hex_or_name(token) for token in invariant_tokens):
          invalid_rows.append(ValidationIssue(idx, f"invariants malformed: {invariants}"))
          continue

      expected_transition = row.get("expected_transition", "")
      if expected_transition and "->" not in expected_transition:
        invalid_rows.append(ValidationIssue(idx, "expected_transition must contain '->'"))
        continue

      row["state_prev"] = prev
      row["state_new"] = nxt
      row["direction"] = direction
      rows.append(row)

  if schema_version is None:
    schema_version = "v1"

  return ParseResult(schema_version=schema_version, rows=rows, invalid_rows=invalid_rows)


def extracted_invariants(row: dict[str, str]) -> list[str]:
  raw = row.get("invariants", "")
  if not raw:
    return []
  tokens = [tok.strip() for tok in raw.split("|") if tok.strip()]
  return sorted(set(tokens))


def canonical_input_pattern(row: dict[str, str]) -> str:
  explicit = row.get("input_pattern", "").strip()
  if explicit:
    return explicit
  return f"{row['state_prev']}:{row['direction']}"


def canonical_expected_transition(row: dict[str, str]) -> str:
  explicit = row.get("expected_transition", "").strip()
  if explicit:
    return explicit
  return f"{row['state_prev']} --{row['direction']}--> {row['state_new']}"


def build_canonical_record(row: dict[str, str]) -> dict[str, Any]:
  return {
    "input_pattern": canonical_input_pattern(row),
    "extracted_invariants": extracted_invariants(row),
    "expected_transition": canonical_expected_transition(row),
    "error_label": row.get("error_label", "").strip() or None,
  }


def write_quality_report(
  path: pathlib.Path,
  *,
  total_rows: int,
  valid_rows: int,
  invalid_rows: list[ValidationIssue],
  records: list[dict[str, Any]],
) -> None:
  invalid_count = len(invalid_rows)
  coverage = 0.0 if total_rows == 0 else valid_rows / total_rows
  error_counter = Counter((rec.get("error_label") or "<none>") for rec in records)
  expected_counter = Counter(rec.get("expected_transition") for rec in records)

  payload = {
    "metrics": {
      "total_rows": total_rows,
      "valid_rows": valid_rows,
      "invalid_rows": invalid_count,
      "coverage": round(coverage, 6),
    },
    "class_balance": {
      "error_label": dict(sorted(error_counter.items())),
      "expected_transition": dict(sorted(expected_counter.items())),
    },
    "invalid_rows": [
      {"row_number": issue.row_number, "reason": issue.reason}
      for issue in invalid_rows
    ],
  }

  path.parent.mkdir(parents=True, exist_ok=True)
  path.write_text(json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def cmd_validate(args: argparse.Namespace) -> int:
  result = parse_and_validate_csv(args.input_csv)
  total_rows = len(result.rows) + len(result.invalid_rows)
  print(
    f"schema={result.schema_version} total={total_rows} valid={len(result.rows)} invalid={len(result.invalid_rows)}"
  )
  if args.quality_report is not None:
    records = [build_canonical_record(row) for row in result.rows]
    write_quality_report(
      args.quality_report,
      total_rows=total_rows,
      valid_rows=len(result.rows),
      invalid_rows=result.invalid_rows,
      records=records,
    )
    print(f"quality report: {args.quality_report}")
  return 0 if not result.invalid_rows else 1


def cmd_normalize(args: argparse.Namespace) -> int:
  result = parse_and_validate_csv(args.input_csv)
  ordered_fields = [
    "schema_version",
    "row_id",
    "state_prev",
    "state_new",
    "direction",
    "input_pattern",
    "invariants",
    "expected_transition",
    "error_label",
  ]
  args.output_csv.parent.mkdir(parents=True, exist_ok=True)
  with args.output_csv.open("w", encoding="utf-8", newline="") as fh:
    writer = csv.DictWriter(fh, fieldnames=ordered_fields, extrasaction="ignore")
    writer.writeheader()
    for idx, row in enumerate(result.rows, start=1):
      out = dict(row)
      out.setdefault("row_id", str(idx))
      for field in ordered_fields:
        out.setdefault(field, "")
      writer.writerow(out)

  if args.quality_report is not None:
    total_rows = len(result.rows) + len(result.invalid_rows)
    records = [build_canonical_record(row) for row in result.rows]
    write_quality_report(
      args.quality_report,
      total_rows=total_rows,
      valid_rows=len(result.rows),
      invalid_rows=result.invalid_rows,
      records=records,
    )

  print(f"normalized rows={len(result.rows)} invalid={len(result.invalid_rows)} -> {args.output_csv}")
  return 0 if not result.invalid_rows else 1


def cmd_export_dot(args: argparse.Namespace) -> int:
  result = parse_and_validate_csv(args.input_csv)
  transitions = [(row["state_prev"], row["state_new"], row["direction"]) for row in result.rows]
  edge_counts = Counter(transitions)

  args.output_dot.parent.mkdir(parents=True, exist_ok=True)
  with args.output_dot.open("w", encoding="utf-8") as fh:
    fh.write("digraph bitomega_transition_graph {\n")
    fh.write("  rankdir=LR;\n")
    fh.write('  node [shape=ellipse, style=filled, fillcolor="#e6f2ff", color="#1f4d7a"];\n')
    for (prev, nxt, direction), count in sorted(edge_counts.items()):
      fh.write(f'  "{prev}" -> "{nxt}" [label="{direction} ({count})"];\n')
    fh.write("}\n")

  if args.summary_md is not None:
    args.summary_md.parent.mkdir(parents=True, exist_ok=True)
    with args.summary_md.open("w", encoding="utf-8") as fh:
      fh.write("# BitΩ Smoke Test Summary\n\n")
      fh.write(f"Total transitions: **{len(transitions)}**\n\n")
      fh.write("| State Prev | State New | Direction | Count |\n")
      fh.write("|---|---|---|---:|\n")
      for (prev, nxt, direction), count in sorted(edge_counts.items()):
        fh.write(f"| {prev} | {nxt} | {direction} | {count} |\n")

  if args.quality_report is not None:
    total_rows = len(result.rows) + len(result.invalid_rows)
    records = [build_canonical_record(row) for row in result.rows]
    write_quality_report(
      args.quality_report,
      total_rows=total_rows,
      valid_rows=len(result.rows),
      invalid_rows=result.invalid_rows,
      records=records,
    )

  print(f"dot edges={len(edge_counts)} invalid={len(result.invalid_rows)} -> {args.output_dot}")
  return 0 if not result.invalid_rows else 1


def cmd_export_jsonl(args: argparse.Namespace) -> int:
  result = parse_and_validate_csv(args.input_csv)
  records = [build_canonical_record(row) for row in result.rows]

  args.output_jsonl.parent.mkdir(parents=True, exist_ok=True)
  with args.output_jsonl.open("w", encoding="utf-8") as fh:
    for rec in records:
      fh.write(json.dumps(rec, ensure_ascii=False, sort_keys=True) + "\n")

  report_path = args.quality_report
  if report_path is None:
    report_path = args.output_jsonl.with_suffix(".quality.json")

  total_rows = len(result.rows) + len(result.invalid_rows)
  write_quality_report(
    report_path,
    total_rows=total_rows,
    valid_rows=len(result.rows),
    invalid_rows=result.invalid_rows,
    records=records,
  )

  print(f"jsonl records={len(records)} invalid={len(result.invalid_rows)} -> {args.output_jsonl}")
  print(f"quality report: {report_path}")
  return 0 if not result.invalid_rows else 1


def load_jsonl(path: pathlib.Path) -> list[dict[str, Any]]:
  require_input(path)
  rows: list[dict[str, Any]] = []
  with path.open("r", encoding="utf-8") as fh:
    for idx, line in enumerate(fh, start=1):
      stripped = line.strip()
      if not stripped:
        continue
      try:
        rec = json.loads(stripped)
      except json.JSONDecodeError as exc:
        raise SchemaError(f"invalid JSONL at line {idx}: {exc}") from exc
      if not isinstance(rec, dict):
        raise SchemaError(f"invalid JSONL record at line {idx}: object expected")
      missing = [field for field in CANONICAL_PROCESSED_COLUMNS if field not in rec]
      if missing:
        raise SchemaError(f"invalid JSONL record at line {idx}: missing {','.join(missing)}")
      rows.append(rec)
  return rows


def deterministic_key(record: dict[str, Any], seed: int) -> str:
  canonical = json.dumps(record, sort_keys=True, ensure_ascii=False)
  return hashlib.sha256(f"{seed}:{canonical}".encode("utf-8")).hexdigest()


def write_jsonl(path: pathlib.Path, records: list[dict[str, Any]]) -> None:
  path.parent.mkdir(parents=True, exist_ok=True)
  with path.open("w", encoding="utf-8") as fh:
    for rec in records:
      fh.write(json.dumps(rec, ensure_ascii=False, sort_keys=True) + "\n")


def cmd_split(args: argparse.Namespace) -> int:
  records = load_jsonl(args.input_jsonl)
  if args.train_ratio <= 0.0 or args.valid_ratio <= 0.0:
    raise SchemaError("train-ratio and valid-ratio must be > 0")
  if args.train_ratio + args.valid_ratio >= 1.0:
    raise SchemaError("train-ratio + valid-ratio must be < 1.0")

  keyed = sorted(records, key=lambda rec: deterministic_key(rec, args.seed))
  size = len(keyed)
  train_end = int(size * args.train_ratio)
  valid_end = train_end + int(size * args.valid_ratio)

  train_records = keyed[:train_end]
  valid_records = keyed[train_end:valid_end]
  test_records = keyed[valid_end:]

  write_jsonl(args.output_dir / "train.jsonl", train_records)
  write_jsonl(args.output_dir / "valid.jsonl", valid_records)
  write_jsonl(args.output_dir / "test.jsonl", test_records)

  report_path = args.quality_report
  if report_path is None:
    report_path = args.output_dir / "split.quality.json"

  write_quality_report(
    report_path,
    total_rows=size,
    valid_rows=size,
    invalid_rows=[],
    records=keyed,
  )

  split_report = {
    "seed": args.seed,
    "ratios": {
      "train": args.train_ratio,
      "valid": args.valid_ratio,
      "test": 1.0 - args.train_ratio - args.valid_ratio,
    },
    "counts": {
      "train": len(train_records),
      "valid": len(valid_records),
      "test": len(test_records),
      "total": size,
    },
  }
  split_report_path = args.output_dir / "split.manifest.json"
  split_report_path.write_text(json.dumps(split_report, indent=2) + "\n", encoding="utf-8")

  print(
    "split complete "
    f"train={len(train_records)} valid={len(valid_records)} test={len(test_records)} seed={args.seed}"
  )
  print(f"split manifest: {split_report_path}")
  print(f"quality report: {report_path}")
  return 0


def main(argv: list[str] | None = None) -> int:
  args = parse_args(sys.argv[1:] if argv is None else argv)
  try:
    if args.command == "validate":
      return cmd_validate(args)
    if args.command == "normalize":
      return cmd_normalize(args)
    if args.command == "export-dot":
      return cmd_export_dot(args)
    if args.command == "export-jsonl":
      return cmd_export_jsonl(args)
    if args.command == "split":
      return cmd_split(args)
    raise SchemaError(f"unknown command: {args.command}")
  except SchemaError as exc:
    print(str(exc), file=sys.stderr)
    return 2


if __name__ == "__main__":
  raise SystemExit(main())
