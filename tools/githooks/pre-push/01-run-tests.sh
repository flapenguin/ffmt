#!/bin/sh

root="$(pwd)"
. "$root/tools/config.sh"

err=0

run() {
  local dir="$root/build/hook-$1"
  rm -rf "$dir"
  mkdir -p "$dir"

  echo "=============================================="
  echo "= Building and testing $1 configuration in $dir"
  echo "=============================================="
  cd "$dir"
  cmake -D CMAKE_BUILD_TYPE=$1 ../../ || err=1
  cmake --build . || err=1
  ctest || err=1

  rm -rf "$dir"
}

run debug
run release

[ $err -ne 0 ] && printf "\n\n"
exit $err
