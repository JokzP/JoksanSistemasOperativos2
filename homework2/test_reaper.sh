#!/bin/bash
# test_reaper.sh - Test reaping strategies

set -e

for strategy in 1 2 3; do
    echo "[TEST] Testing strategy $strategy..."
    ../zombie_reaper "$strategy"
    echo
done

echo "[TEST] Reaper tests completed."
