#!/bin/bash
# RunAll.sh — Compile and run all Java Core History examples
#
# Usage:
#   chmod +x RunAll.sh
#   ./RunAll.sh
#
# Or run a single era:
#   javac Era01_Foundation.java && java Era01_Foundation

set -e  # exit on first error

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

JAVA_VERSION=$(java -version 2>&1 | head -1 | sed 's/.*version "\([0-9]*\).*/\1/')
echo "Detected Java version: $JAVA_VERSION"
echo ""

ERAS=(
    "Era01_Foundation"
    "Era03_Java5_Features"
    "Era05_Java8_Lambda"
    "Era07_Java11_Modern"
    "Era09_Java21_Patterns"
)

PASSED=0
FAILED=0

for ERA in "${ERAS[@]}"; do
    if [ ! -f "${ERA}.java" ]; then
        echo "⚠  ${ERA}.java not found — skipping"
        continue
    fi

    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "▶  Compiling and running: ${ERA}"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

    # Compile
    if javac "${ERA}.java" 2>/dev/null || javac --enable-preview --release "$JAVA_VERSION" "${ERA}.java" 2>/dev/null; then
        # Run
        if java -cp . "$ERA" 2>/dev/null || java --enable-preview -cp . "$ERA" 2>/dev/null; then
            PASSED=$((PASSED + 1))
        else
            echo "❌ Runtime error in ${ERA}"
            FAILED=$((FAILED + 1))
        fi
    else
        echo "❌ Compile error in ${ERA}"
        FAILED=$((FAILED + 1))
    fi
    echo ""
done

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Results: $PASSED passed, $FAILED failed"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
