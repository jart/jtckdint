#!/bin/sh
set -ex

for cc in cc clang; do
  for opt in -O0 -O3 -fsanitize=undefined; do
    make clean
    make CC="$cc -Wall -Wextra -Werror $opt"
    make clean
    make CC="$cc -Wall -Wextra -Werror -pedantic-errors $opt -std=c11"
  done
done

for cc in c++ clang++; do
  for opt in -O0 -O3 -fsanitize=undefined; do
    make clean
    make CC="$cc -Wall -Wextra -Werror $opt" CFLAGS="-xc++"
    make clean
    make CC="$cc -Wall -Wextra -Werror -pedantic-errors $opt -std=c++11" CFLAGS="-xc++"
  done
done

make clean
make
