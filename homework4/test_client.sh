#!/bin/bash
# test_client.sh - Start two clients for manual testing

set -e
echo "[TEST] Starting two chat_client instances..."
( printf "alice\nHello from alice\n/quit\n" | ../chat_client ) &
sleep 1
( printf "bob\nHello from bob\n/quit\n" | ../chat_client ) &
wait
echo "[TEST] Clients finished."
