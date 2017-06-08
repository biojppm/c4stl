#!/usr/bin/env bash

set -e
set -x

export CC_=$(echo "$CXX_" | sed 's:clang++:clang:g' | sed 's:g++:gcc:g')

pwd
C4STL_DIR=$(pwd)

if [ "$SAN" == "ON" ] ; then
    set CMFLAGS="$CMFLAGS -DC4STL_SANITIZE=ON"
fi

cd build

cmake -DCMAKE_C_COMPILER=$CC_ -DCMAKE_CXX_COMPILER=$CXX_ \
      -DCMAKE_BUILD_TYPE=$BT -DCMAKE_CXX_FLAGS="$XFLAGS" \
      -DC4STL_EXTERN_DIR=`pwd`/extern_install \
      -DC4STL_PEDANTIC=ON -DC4STL_WERROR=ON \
      $CMFLAGS \
      $C4STL_DIR

make CTEST_OUTPUT_ON_FAILURE=1 c4stl-test
cd -

exit 0
