#!/bin/bash
# test_detector.sh - Simple run of zombie_detector

set -e

echo "[TEST] Running zombie_detector..."
../zombie_detector || true
echo "[TEST] Done."
