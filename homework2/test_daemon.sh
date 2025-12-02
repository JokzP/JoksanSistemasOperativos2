#!/bin/bash
# test_daemon.sh - Basic daemon test (requires manual inspection for zombies)

set -e

echo "[TEST] Starting process_daemon..."
../process_daemon &
DAEMON_PID=$!
echo "Daemon PID: $DAEMON_PID"

echo "[TEST] Sleeping for 15 seconds..."
sleep 15

echo "[TEST] Checking for defunct processes (manual inspection may be needed)..."
ps aux | grep defunct || true

echo "[TEST] Stopping daemon..."
kill "$DAEMON_PID" 2>/dev/null || true
sleep 2
echo "[TEST] Done."
