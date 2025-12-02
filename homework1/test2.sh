#!/bin/bash
# test2.sh - Signal handling test (SIGINT, SIGCHLD)

set -e

echo "[TEST2] Starting signal handling test..."

# Run procman in background and send SIGINT after a short delay
(
  sleep 1
  pkill -INT procman || true
) &

../procman <<EOF
create sleep 30
EOF

echo "[TEST2] Completed (check that procman exited gracefully)."
