#!/usr/bin/env bash

set -e
set -x

export C_COMPILER=$(echo "$COMPILER" | sed 's:clang++:clang:g' | sed 's:g++:gcc:g')

pwd
C4STL_DIR=$(pwd)

cd build
if [ "$PEDANTIC" == "ON" ] ; then
    cmake -DCMAKE_C_COMPILER=$C_COMPILER -DCMAKE_CXX_COMPILER=$COMPILER \
          -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_CXX_FLAGS="$EXTRA_FLAGS" \
          -DC4STL_PEDANTIC=ON -DC4STL_WERROR=ON \
          -DC4STL_EXTERN_DIR=`pwd`/extern_install \
          -DC4STL_CTEST_OPTIONS="-V" \
          $C4STL_DIR
else
    cmake -DCMAKE_C_COMPILER=$C_COMPILER -DCMAKE_CXX_COMPILER=$COMPILER \
          -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_CXX_FLAGS="$EXTRA_FLAGS" \
          -DC4STL_EXTERN_DIR=`pwd`/extern_install \
          -DC4STL_CTEST_OPTIONS="-V" \
          $C4STL_DIR
fi
make CTEST_OUTPUT_ON_FAILURE=1 c4stl-test
cd -

exit 0
