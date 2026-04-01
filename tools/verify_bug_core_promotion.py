#!/usr/bin/env python3
"""Verifica governança de promoção entre bug/core e engine/rmr/src.

Compara pares *.c por nome e valida:
- assinatura de funções
- ordem de declaração/definição
- símbolos exportados (não-static)
- includes críticos

Quando houver incompatibilidade de API/ABI, o script bloqueia promoção em bloco
com exit code != 0 e gera relatório com candidatas à promoção incremental.
"""

from __future__ import annotations

import argparse
import hashlib
import re
import sys
from dataclasses import dataclass
from pathlib import Path


FUNC_DEF_RE = re.compile(
    r"(^|\n)\s*((?:[A-Za-z_]\w*\s+|\*|\s)+?)\s+([A-Za-z_]\w*)\s*\(([^;{}]*)\)\s*\{",
    re.M,
)
INCLUDE_RE = re.compile(r'^\s*#\s*include\s*[<\"]([^>\"]+)[>\"]', re.M)
COMMENT_RE = re.compile(r"/\*.*?\*/|//.*?$", re.S | re.M)
RESERVED_NAMES = {"if", "for", "while", "switch", "return", "do"}


@dataclass
class FunctionInfo:
    name: str
    signature: str
    is_static: bool
    order: int
    body_hash: str


@dataclass
class PairReport:
    filename: str
    api_mismatch: list[str]
    abi_mismatch: list[str]
    order_mismatch: bool
    includes_mismatch: list[str]
    candidate_functions: list[str]
    blocked: bool


def normalize_ws(text: str) -> str:
    return " ".join(text.replace("\n", " ").split())


def strip_comments(text: str) -> str:
    return COMMENT_RE.sub("", text)


def extract_body_hash(src: str, brace_start: int) -> str:
    depth = 0
    i = brace_start
    end = len(src)
    while i < len(src):
        ch = src[i]
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                end = i + 1
                break
        i += 1
    body = src[brace_start:end]
    return hashlib.sha256(normalize_ws(body).encode("utf-8")).hexdigest()[:12]


def parse_functions(path: Path) -> list[FunctionInfo]:
    raw = path.read_text(encoding="utf-8", errors="ignore")
    text = strip_comments(raw)
    out: list[FunctionInfo] = []
    order = 0
    for m in FUNC_DEF_RE.finditer(text):
        ret = normalize_ws(m.group(2))
        name = m.group(3)
        if name in RESERVED_NAMES:
            continue
        params = normalize_ws(m.group(4))
        sig = f"{ret} {name}({params})"
        is_static = bool(re.search(r"\bstatic\b", ret))
        brace_pos = m.end() - 1
        out.append(
            FunctionInfo(
                name=name,
                signature=sig,
                is_static=is_static,
                order=order,
                body_hash=extract_body_hash(text, brace_pos),
            )
        )
        order += 1
    return out


def parse_includes(path: Path) -> set[str]:
    text = path.read_text(encoding="utf-8", errors="ignore")
    return set(INCLUDE_RE.findall(text))


def critical_includes(includes: set[str]) -> set[str]:
    return {
        inc
        for inc in includes
        if inc.startswith("rmr_")
        or inc == "zero.h"
        or inc.startswith("zipraf")
        or inc.startswith("bitraf")
    }


def compare_pair(bug_file: Path, engine_file: Path) -> PairReport:
    bug_funcs = parse_functions(bug_file)
    eng_funcs = parse_functions(engine_file)

    bug_by_name = {f.name: f for f in bug_funcs}
    eng_by_name = {f.name: f for f in eng_funcs}

    shared = sorted(set(bug_by_name) & set(eng_by_name))

    api_mismatch: list[str] = []
    for name in shared:
        if bug_by_name[name].signature != eng_by_name[name].signature:
            api_mismatch.append(name)

    bug_exports = {f.name for f in bug_funcs if not f.is_static}
    eng_exports = {f.name for f in eng_funcs if not f.is_static}
    abi_mismatch = sorted((bug_exports ^ eng_exports))

    bug_order_shared = [f.name for f in bug_funcs if f.name in shared]
    eng_order_shared = [f.name for f in eng_funcs if f.name in shared]
    order_mismatch = bug_order_shared != eng_order_shared

    bug_inc = critical_includes(parse_includes(bug_file))
    eng_inc = critical_includes(parse_includes(engine_file))
    includes_mismatch = sorted(bug_inc ^ eng_inc)

    candidates = [
        name
        for name in shared
        if name not in api_mismatch
        and bug_by_name[name].is_static == eng_by_name[name].is_static
        and bug_by_name[name].body_hash != eng_by_name[name].body_hash
    ]

    blocked = bool(api_mismatch or abi_mismatch or order_mismatch or includes_mismatch)

    return PairReport(
        filename=bug_file.name,
        api_mismatch=sorted(api_mismatch),
        abi_mismatch=abi_mismatch,
        order_mismatch=order_mismatch,
        includes_mismatch=includes_mismatch,
        candidate_functions=sorted(candidates),
        blocked=blocked,
    )


def write_report(path: Path, reports: list[PairReport]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    blocked_count = sum(1 for r in reports if r.blocked)
    lines: list[str] = []
    lines.append("# Relatório de Governança de Promoção (bug/core → engine/rmr/src)")
    lines.append("")
    lines.append(f"- Pares analisados: **{len(reports)}**")
    lines.append(f"- Promoção em bloco bloqueada em: **{blocked_count}** pares")
    lines.append("")
    lines.append("## Resultado por par")
    lines.append("")
    lines.append("| Par | API (assinatura) | ABI (exports) | Ordem | Includes críticos | Candidatas incrementais | Status |")
    lines.append("|---|---:|---:|---:|---:|---:|---|")

    for r in reports:
        status = "BLOQUEADO" if r.blocked else "COMPATÍVEL"
        lines.append(
            f"| `{r.filename}` | {len(r.api_mismatch)} | {len(r.abi_mismatch)} | "
            f"{'1' if r.order_mismatch else '0'} | {len(r.includes_mismatch)} | {len(r.candidate_functions)} | {status} |"
        )

    lines.append("")
    lines.append("## Detalhes")
    lines.append("")
    for r in reports:
        lines.append(f"### {r.filename}")
        lines.append(f"- Status: **{'BLOQUEADO' if r.blocked else 'COMPATÍVEL'}**")
        if r.api_mismatch:
            lines.append(f"- API divergente (assinatura): `{', '.join(r.api_mismatch)}`")
        if r.abi_mismatch:
            lines.append(f"- ABI divergente (exports): `{', '.join(r.abi_mismatch)}`")
        if r.order_mismatch:
            lines.append("- Ordem das funções compartilhadas diverge.")
        if r.includes_mismatch:
            lines.append(f"- Includes críticos divergentes: `{', '.join(r.includes_mismatch)}`")
        if r.candidate_functions:
            lines.append(f"- Funções candidatas à promoção incremental: `{', '.join(r.candidate_functions)}`")
        if not (r.api_mismatch or r.abi_mismatch or r.order_mismatch or r.includes_mismatch):
            lines.append("- Sem divergências de API/ABI/ordem/includes críticos.")
        lines.append("")

    path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--bug-dir", default="bug/core")
    parser.add_argument("--engine-dir", default="engine/rmr/src")
    parser.add_argument(
        "--report",
        default="reports/promotion_governance_report.md",
        help="caminho do relatório markdown",
    )
    args = parser.parse_args()

    bug_dir = Path(args.bug_dir)
    engine_dir = Path(args.engine_dir)

    if not bug_dir.is_dir() or not engine_dir.is_dir():
        print("[promotion-governance] diretórios inválidos", file=sys.stderr)
        return 2

    bug_files = {p.name: p for p in bug_dir.glob("*.c")}
    engine_files = {p.name: p for p in engine_dir.glob("*.c")}
    common_names = sorted(set(bug_files) & set(engine_files))

    reports = [compare_pair(bug_files[name], engine_files[name]) for name in common_names]
    write_report(Path(args.report), reports)

    blocked = [r.filename for r in reports if r.blocked]
    if blocked:
        print("[promotion-governance] BLOQUEIO: incompatibilidades de API/ABI detectadas")
        print("[promotion-governance] pares bloqueados:", ", ".join(blocked))
        print(f"[promotion-governance] relatório: {args.report}")
        return 1

    print("[promotion-governance] OK: sem incompatibilidades de API/ABI")
    print(f"[promotion-governance] relatório: {args.report}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
