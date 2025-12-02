#!/bin/bash
# test_creator.sh - Test zombie_creator and zombie_detector

set -e

echo "[TEST] Running zombie_creator 5 in background..."
../zombie_creator 5 &
CREATOR_PID=$!
sleep 2

echo "[TEST] Running zombie_detector..."
../zombie_detector || true

echo "[TEST] Killing zombie_creator (PID $CREATOR_PID)..."
kill "$CREATOR_PID" 2>/dev/null || true
wait "$CREATOR_PID" 2>/dev/null || true

echo "[TEST] Done."
