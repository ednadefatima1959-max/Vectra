#!/usr/bin/env bash
set -euo pipefail

out="${1:-reports/baremetal/dir_integrity_matrix.json}"
mkdir -p "$(dirname "$out")"

collect_files() {
  # Deterministic file source: prefer tracked files, fallback to filesystem.
  if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    git ls-files | LC_ALL=C sort -u
  else
    find . -type f -print | sed 's#^\./##' | LC_ALL=C sort -u
  fi
}

mapfile -t files < <(collect_files)

# Build deterministic index with one file hash per file (no re-hashing per directory).
idx_file="$(mktemp)"
trap 'rm -f "$idx_file"' EXIT

for f in "${files[@]}"; do
  [ -f "$f" ] || continue
  h="$(sha256sum "$f" | awk '{print $1}')"
  printf '%s\t%s\n' "$f" "$h" >> "$idx_file"
done

mapfile -t dirs < <(
  awk -F'\t' '
    {
      file=$1;
      print ".";
      n=split(file, p, "/");
      if (n > 1) {
        d="";
        for (i=1; i<n; i++) {
          d=(d=="" ? p[i] : d "/" p[i]);
          print d;
        }
      }
    }
  ' "$idx_file" | LC_ALL=C sort -u
)

count_files() {
  local d="$1"
  local p="$d"
  [ "$p" = "." ] && p=""

  awk -F'\t' -v p="$p" '
    {
      if (p == "" || index($1, p "/") == 1) c++;
    }
    END { print c + 0 }
  ' "$idx_file"
}

hash_dir() {
  local d="$1"
  local p="$d"
  [ "$p" = "." ] && p=""

  awk -F'\t' -v p="$p" '
    {
      if (p == "" || index($1, p "/") == 1) print $2 "  " $1;
    }
  ' "$idx_file" | sha256sum | awk '{print $1}'
}

{
  echo "{"
  echo '  "schema": "baremetal-integrity-v1",'
  echo '  "directories": ['
  n=${#dirs[@]}
  i=0
  for d in "${dirs[@]}"; do
    i=$((i+1))
    h="$(hash_dir "$d")"
    c="$(count_files "$d")"
    comma=","
    [ "$i" -eq "$n" ] && comma=""
    echo "    {\"path\": \"$d\", \"files\": $c, \"sha256_tree\": \"$h\"}$comma"
  done
  echo "  ]"
  echo "}"
} > "$out"

cat "$out"
