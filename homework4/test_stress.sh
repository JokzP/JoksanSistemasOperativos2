#!/bin/bash
# test_stress.sh - Simple stress test skeleton

set -e
echo "[TEST] Stress test - starting 5 clients..."
for i in $(seq 1 5); do
    ( printf "user$i\nHello from user$i\n/quit\n" | ../chat_client ) &
done
wait
echo "[TEST] Stress test completed."
