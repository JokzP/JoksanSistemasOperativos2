#!/bin/bash
# test1.sh - Basic process creation and termination

set -e

echo "[TEST1] Starting basic process creation test..."

../procman <<EOF
create sleep 2
list
wait
list
quit
EOF

echo "[TEST1] Completed."
