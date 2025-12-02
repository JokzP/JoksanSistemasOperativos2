#!/bin/bash
# test3.sh - Process tree visualization test

set -e

echo "[TEST3] Starting process tree test..."

../procman <<EOF
create sleep 5
list
tree
wait
quit
EOF

echo "[TEST3] Completed."
