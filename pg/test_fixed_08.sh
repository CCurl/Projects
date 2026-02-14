#!/bin/bash

# Test runner for just test_08
echo "Testing fixed test_08_advanced.lisp..."

if timeout 30s ./l99 < test_08_advanced.lisp > results_test_08_fixed.txt 2>&1; then
    echo "✅ Test 08 (Advanced) PASSED"
    echo "Sample results:"
    grep -v "ERR" results_test_08_fixed.txt | tail -10
else
    echo "❌ Test 08 (Advanced) FAILED"
    echo "Error output:"
    tail -5 results_test_08_fixed.txt
fi