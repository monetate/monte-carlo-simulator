#!/bin/bash

# Functional tests that ensure `simulate` binary behaves as expected.

# Usage: (from project root directory)
# make functional_tests

FAILED_TEST=64

# Test that the simulator is producing the expected output (in ./results.csv)
result=$(head -n 10 ./samples.csv | (../simulate 500 0.25 0.25 0.5) | head -n 18 | diff - ./results.csv 2>&1)
if [ $? != 0 ]; then
  echo "---------"
  echo "Expected '../simulate 500 0.25 0.25 0.5' first 18 lines to match ./results.csv"
  echo "Diff Output"
  echo "$result"
  echo "---------"
  exit $FAILED_TEST
fi

# Test that 500 interations with 3 groups produces 1500 simulation lines
result=$(head -n 10 ./samples.csv | (../simulate 500 0.25 0.25 0.5) | wc -l | diff - <(echo "1500") 2>&1)
if [ $? != 0 ]; then
  echo "---------"
  echo "Expected '../simulate 500 0.25 0.25 0.5' to produce 1500 lines"
  echo "Diff Output"
  echo "$result"
  echo "---------"
  exit $FAILED_TEST
fi

# Test that 1000 interations with 2 groups produces 2000 simulation lines
result=$(head -n 10 ./samples.csv | (../simulate 1000 0.25 0.25) | wc -l | diff - <(echo "2000") 2>&1)
if [ $? != 0 ]; then
  echo "---------"
  echo "Expected '../simulate 500 0.25 0.25' to produce 2000 lines"
  echo "Diff Output"
  echo "$result"
  echo "---------"
  exit $FAILED_TEST
fi
