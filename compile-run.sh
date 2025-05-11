#!/bin/bash

# Description: Add a brief description of what this script does.
# Usage: Add usage instructions if necessary.

# Exit immediately if a command exits with a non-zero status
set -e

# Your script logic goes here
./build/cscript ./example/hello.csc &&  \
llc -filetype=obj -relocation-model=pic output.ll -o output.o && \
clang output.o -o my_program && \
./my_program