#!/bin/bash
# test_disconnect.sh - Test abrupt client disconnect

set -e
echo "[TEST] Starting client..."
../chat_client &
PID=$!
sleep 2
echo "[TEST] Killing client PID $PID..."
kill -9 "$PID" 2>/dev/null || true
sleep 2
echo "[TEST] Check server output for disconnect handling."
