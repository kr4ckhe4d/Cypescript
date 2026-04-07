#!/bin/bash
# Cypescript Test Runner (wrapper)
# Runs the test suite from the tests/ directory
exec "$(dirname "$0")/tests/run_tests.sh" "$@"
