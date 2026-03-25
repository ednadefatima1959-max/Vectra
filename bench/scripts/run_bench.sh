#!/usr/bin/env bash
set -euo pipefail

runs="${1:-7}"
out_dir="${2:-bench/results}"
mkdir -p "$out_dir"

make run-bench >/dev/null

scores_file="$out_dir/scores.txt"
formulas_file="$out_dir/score_formula_ids.txt"
coherence_file="$out_dir/coherence_mean.txt"
reuse_file="$out_dir/reuse_ratio.txt"
skip_file="$out_dir/skip_execution_ratio.txt"
reconstruction_file="$out_dir/reconstruction_success_rate.txt"
invariant_file="$out_dir/invariant_violation_count.txt"
: > "$scores_file"
: > "$formulas_file"
: > "$coherence_file"
: > "$reuse_file"
: > "$skip_file"
: > "$reconstruction_file"
: > "$invariant_file"

extract_json_number() {
  local key="$1"
  local file="$2"
  awk -v k="\"$key\"" -F: '$1 ~ k {gsub(/[ ,]/, "", $2); print $2; exit}' "$file"
}

for i in $(seq 1 "$runs"); do
  csv="$out_dir/run_${i}.csv"
  json="$out_dir/run_${i}.json"
  ./build/bench/rmr_bench "$csv" "$json" | tee "$out_dir/run_${i}.log"
  score=$(awk -F= '/bench_total_score=/{print $2}' "$out_dir/run_${i}.log")
  if [ -z "$score" ]; then
    score=$(awk -F: '/"total_score"/{gsub(/[ ,]/, "", $2); print $2; exit}' "$json")
  fi
  formula_id=$(awk -F: '/"score_formula_id"/{gsub(/[ ",]/, "", $2); print $2; exit}' "$json")
  if [ -z "$score" ]; then
    echo "missing score in run $i" >&2
    exit 1
  fi
  if [ -z "$formula_id" ]; then
    echo "missing score_formula_id in run $i" >&2
    exit 1
  fi
  coherence_mean=$(extract_json_number "coherence_mean" "$json")
  reuse_ratio=$(extract_json_number "reuse_ratio" "$json")
  skip_execution_ratio=$(extract_json_number "skip_execution_ratio" "$json")
  reconstruction_success_rate=$(extract_json_number "reconstruction_success_rate" "$json")
  invariant_violation_count=$(extract_json_number "invariant_violation_count" "$json")
  if [ -z "$coherence_mean" ] || [ -z "$reuse_ratio" ] || [ -z "$skip_execution_ratio" ] || [ -z "$reconstruction_success_rate" ] || [ -z "$invariant_violation_count" ]; then
    echo "missing required extended metrics in run $i" >&2
    exit 1
  fi
  echo "$score" >> "$scores_file"
  echo "$formula_id" >> "$formulas_file"
  echo "$coherence_mean" >> "$coherence_file"
  echo "$reuse_ratio" >> "$reuse_file"
  echo "$skip_execution_ratio" >> "$skip_file"
  echo "$reconstruction_success_rate" >> "$reconstruction_file"
  echo "$invariant_violation_count" >> "$invariant_file"
done

count=$(wc -l < "$scores_file" | tr -d ' ')
if [ "$count" -eq 0 ]; then
  echo "no scores" >&2
  exit 1
fi

compute_stats_json() {
  local name="$1"
  local input="$2"
  local sorted="$out_dir/${name}_sorted.txt"
  sort -n "$input" > "$sorted"
  local median
  median=$(awk -v n="$count" '{a[NR]=$1} END{if(n%2==1){print a[(n+1)/2]}else{print int((a[n/2]+a[n/2+1])/2)}}' "$sorted")
  local p95_index=$(( (95*count + 99)/100 ))
  if [ "$p95_index" -lt 1 ]; then p95_index=1; fi
  local p95
  p95=$(awk -v idx="$p95_index" 'NR==idx{print $1}' "$sorted")
  local minv
  minv=$(awk 'NR==1{print $1}' "$sorted")
  local maxv
  maxv=$(awk -v n="$count" 'NR==n{print $1}' "$sorted")
  local values_json
  values_json=$(awk 'BEGIN{printf "["} {if(NR>1)printf ","; printf "%s", $1} END{printf "]"}' "$sorted")
  printf '"median": %s, "p95": %s, "min": %s, "max": %s, "values_sorted": %s' "$median" "$p95" "$minv" "$maxv" "$values_json"
}

formula_id=$(sort -u "$formulas_file")
formula_count=$(printf '%s\n' "$formula_id" | sed '/^$/d' | wc -l | tr -d ' ')
if [ "$formula_count" -ne 1 ]; then
  echo "inconsistent score_formula_id values:" >&2
  sort -u "$formulas_file" >&2
  exit 1
fi

score_stats=$(compute_stats_json "score" "$scores_file")
coherence_stats=$(compute_stats_json "coherence_mean" "$coherence_file")
reuse_stats=$(compute_stats_json "reuse_ratio" "$reuse_file")
skip_stats=$(compute_stats_json "skip_execution_ratio" "$skip_file")
reconstruction_stats=$(compute_stats_json "reconstruction_success_rate" "$reconstruction_file")
invariant_stats=$(compute_stats_json "invariant_violation_count" "$invariant_file")

score_min_threshold=1
coherence_min_threshold=500000
reuse_min_threshold=0
skip_max_threshold=50000
reconstruction_min_threshold=1000000
invariant_max_threshold=0

score_min_observed=$(awk 'NR==1{print $1}' "$out_dir/score_sorted.txt")
coherence_min_observed=$(awk 'NR==1{print $1}' "$out_dir/coherence_mean_sorted.txt")
reuse_min_observed=$(awk 'NR==1{print $1}' "$out_dir/reuse_ratio_sorted.txt")
skip_max_observed=$(awk -v n="$count" 'NR==n{print $1}' "$out_dir/skip_execution_ratio_sorted.txt")
reconstruction_min_observed=$(awk 'NR==1{print $1}' "$out_dir/reconstruction_success_rate_sorted.txt")
invariant_max_observed=$(awk -v n="$count" 'NR==n{print $1}' "$out_dir/invariant_violation_count_sorted.txt")

score_status="pass"; if [ "$score_min_observed" -lt "$score_min_threshold" ]; then score_status="fail"; fi
coherence_status="pass"; if [ "$coherence_min_observed" -lt "$coherence_min_threshold" ]; then coherence_status="fail"; fi
reuse_status="pass"; if [ "$reuse_min_observed" -lt "$reuse_min_threshold" ]; then reuse_status="fail"; fi
skip_status="pass"; if [ "$skip_max_observed" -gt "$skip_max_threshold" ]; then skip_status="fail"; fi
reconstruction_status="pass"; if [ "$reconstruction_min_observed" -lt "$reconstruction_min_threshold" ]; then reconstruction_status="fail"; fi
invariant_status="pass"; if [ "$invariant_max_observed" -gt "$invariant_max_threshold" ]; then invariant_status="fail"; fi

overall_status="pass"
if [ "$score_status" = "fail" ] || [ "$coherence_status" = "fail" ] || [ "$reuse_status" = "fail" ] || [ "$skip_status" = "fail" ] || [ "$reconstruction_status" = "fail" ] || [ "$invariant_status" = "fail" ]; then
  overall_status="fail"
fi

cat > "$out_dir/summary.json" <<JSON
{
  "schema_version": "bench_summary_v2",
  "runs": $count,
  "score_formula_id": "${formula_id}",
  "overall_status": "$overall_status",
  "metrics": {
    "score": {$score_stats},
    "coherence_mean": {$coherence_stats},
    "reuse_ratio": {$reuse_stats},
    "skip_execution_ratio": {$skip_stats},
    "reconstruction_success_rate": {$reconstruction_stats},
    "invariant_violation_count": {$invariant_stats}
  },
  "thresholds": {
    "score": {"mode": "min", "value": $score_min_threshold, "status": "$score_status"},
    "coherence_mean": {"mode": "min", "value": $coherence_min_threshold, "status": "$coherence_status"},
    "reuse_ratio": {"mode": "min", "value": $reuse_min_threshold, "status": "$reuse_status"},
    "skip_execution_ratio": {"mode": "max", "value": $skip_max_threshold, "status": "$skip_status"},
    "reconstruction_success_rate": {"mode": "min", "value": $reconstruction_min_threshold, "status": "$reconstruction_status"},
    "invariant_violation_count": {"mode": "max", "value": $invariant_max_threshold, "status": "$invariant_status"}
  }
}
JSON
cat "$out_dir/summary.json"
