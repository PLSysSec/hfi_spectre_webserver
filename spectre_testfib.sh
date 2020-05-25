#!/bin/bash
PROTECTION=$1
RESULT=$(curl "http://localhost:8000/fib_c?protection=$PROTECTION&num=4" 2>/dev/null)
EXPECTED="3"
if [[ "$RESULT" != "$EXPECTED" ]]; then
    echo "Incorrect result from fib_c: Got: '$RESULT'. Expected: '$EXPECTED'"
    exit 1
fi
