#!/usr/bin/env bash
set -euo pipefail

app="${1:-./sudoku}"
output="$({ printf 'help\nnew easy\nsolution\nquit\n'; } | "$app")"

grep -q "Terminal Sudoku" <<<"$output"
grep -q "Started a new easy game" <<<"$output"
grep -q "Solution:" <<<"$output"
grep -q "Goodbye." <<<"$output"

echo "CLI integration test passed."
