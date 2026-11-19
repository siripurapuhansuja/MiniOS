#!/bin/bash

# MiniOS Member 5 - Integration Test
# Tests the integrated shell, runtime, jobs, workers and shutdown.

set -u

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT_DIR" || exit 1

echo "======================================"
echo " MiniOS Member 5 Integration Test"
echo "======================================"

# Build MiniOS
echo "[1/7] Building MiniOS..."
if ! make minios; then
    echo "FAIL: MiniOS build failed."
    exit 1
fi
echo "PASS: MiniOS built successfully."

# Run all shell commands in one session
echo "[2/7] Testing shell and help..."
OUTPUT="$(printf 'help\nsystem\nprocesses\nsubmit member5_test\nsleep 2\njobs\nworkers\nrace\nquit\n' | timeout 30s ./minios 2>&1)"
STATUS=$?

if [ "$STATUS" -ne 0 ]; then
    echo "FAIL: MiniOS exited with status $STATUS."
    echo "$OUTPUT"
    exit 1
fi

# Check expected output
echo "[3/7] Checking shell commands..."

for EXPECTED in \
    "Welcome to MiniOS" \
    "MiniOS Available Commands" \
    "System Uptime" \
    "Memory" \
    "Submitted Job" \
    "Completed" \
    "Worker" \
    "Exiting MiniOS"
do
    if ! printf '%s\n' "$OUTPUT" | grep -qi "$EXPECTED"; then
        echo "FAIL: Expected output not found: $EXPECTED"
        echo "$OUTPUT"
        exit 1
    fi
done

echo "PASS: Shell commands produced expected output."

# Check race demonstration
echo "[4/7] Checking race demonstration..."
if ! printf '%s\n' "$OUTPUT" | grep -qi "400000"; then
    echo "FAIL: Expected synchronized counter result was not found."
    echo "$OUTPUT"
    exit 1
fi
echo "PASS: Race demonstration completed."

# Check process command
echo "[5/7] Checking process listing..."
if ! printf '%s\n' "$OUTPUT" | grep -qi "PID"; then
    echo "FAIL: Process listing output was not found."
    echo "$OUTPUT"
    exit 1
fi
echo "PASS: Process listing produced output."

# Check clean shutdown
echo "[6/7] Checking shutdown..."
if ! printf '%s\n' "$OUTPUT" | grep -qi "Goodbye"; then
    echo "FAIL: MiniOS shutdown message was not found."
    exit 1
fi
echo "PASS: MiniOS shutdown completed."

echo "[7/7] All Member 5 integration checks passed."
echo "======================================"
echo " MEMBER 5 TEST: SUCCESS"
echo "======================================"

