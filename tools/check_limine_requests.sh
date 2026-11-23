#!/usr/bin/env bash
set -euo pipefail
elf="${1:-build/psre.elf}"
command -v readelf >/dev/null 2>&1 || { echo "readelf is required"; exit 2; }
[[ -f "$elf" ]] || { echo "ELF not found: $elf"; exit 2; }

sections="$(readelf -SW "$elf")"
symbols="$(readelf -sW "$elf")"
grep -Eq '\.limine_requests[[:space:]]' <<<"$sections" || {
  echo "FAIL: .limine_requests output section missing"; exit 1;
}
grep -Eq ' [Ww][Aa]? ' <<<"$(grep -E '\.limine_requests[[:space:]]' <<<"$sections")" || {
  echo "FAIL: .limine_requests is not writable"; exit 1;
}
for sym in psre_framebuffer_request psre_memmap_request psre_hhdm_request; do
  grep -q "$sym" <<<"$symbols" || { echo "FAIL: missing symbol $sym"; exit 1; }
done
grep -q '__limine_requests_start' <<<"$symbols" || { echo "FAIL: start marker boundary missing"; exit 1; }
grep -q '__limine_requests_end' <<<"$symbols" || { echo "FAIL: end marker boundary missing"; exit 1; }
echo "PASS: Limine request block exists, is writable, and required request symbols are present."
