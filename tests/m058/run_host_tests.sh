#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
SRC="$ROOT/kernel/src/m022/interrupt.c"
AUDIT="$ROOT/kernel/src/m058/diagnostics.c"
MAIN="$ROOT/kernel/src/main.c"
# Guard against accidental endpoint-type regression: CErr=3 and Interrupt-IN=7.
grep -q 'M022_EP_CERR_VALUE 3u' "$SRC"
grep -q 'M022_EP_TYPE_INTERRUPT_IN 7u' "$SRC"
grep -q 'M022_EP_CERR_VALUE<<1).*M022_EP_TYPE_INTERRUPT_IN<<3' "$SRC"
# M058 must remain observational and be called after M056 diagnostics.
grep -q 'Diagnostic-only audit; no PCI writes, xHCI reset, USB re-enumeration' "$AUDIT"
grep -q 'psre_m058_initialize();' "$MAIN"
# Desktop handoff/render and event loop statements must remain present.
grep -q 'psre_m035_render()' "$MAIN"
grep -q 'psre_m050_run_once()' "$MAIN"
echo 'M058 host checks: PASS'
