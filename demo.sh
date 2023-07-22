#!/bin/sh

filter() {
  grep -v cfi |
  grep -v \\.file |
  grep -v \\.text |
  grep -v \\.size |
  grep -v \\.type |
  grep -v \\.LF |
  sed 's/.*\.globl.*//' |
  grep -v \\.section
}

echo
echo ========================================================================
echo jtckdint.h using compiler builtins
echo cc -S -Os -o - demo.c
echo ========================================================================
echo
cc -S -Os -o - demo.c | filter

echo
echo ========================================================================
echo jtckdint.h using c11 polyfill
echo cc -S -Os -std=c11 -o - demo.c
echo ========================================================================
echo
cc -S -Os -std=c11 -o - demo.c | filter

echo
echo ========================================================================
echo jtckdint.h using c++11 polyfill
echo c++ -S -Os -std=c++11 -xc++ -o - demo.c
echo ========================================================================
echo
c++ -S -Os -std=c++11 -xc++ -o - demo.c | filter
