#!/usr/bin/env bash

set -e
set -x

pwd
C4STL_DIR=$(pwd)

export CC_=$(echo "$CXX_" | sed 's:clang++:clang:g' | sed 's:g++:gcc:g')

CMFLAGS="-DC4STL_LINT=OFF"

  if [ "$SAN" == "ALL" ] ; then CMFLAGS="$CMFLAGS -DC4STL_SANITIZE=ON -DC4STL_SANITIZE_ONLY=ON"
elif [ "$SAN" == "A"   ] ; then CMFLAGS="$CMFLAGS -DC4STL_SANITIZE=ON -DC4STL_SANITIZE_ONLY=ON -DC4STL_ASAN=ON  -DC4STL_TSAN=OFF -DC4STL_MSAN=OFF -DC4STL_UBSAN=OFF"
elif [ "$SAN" == "T"   ] ; then CMFLAGS="$CMFLAGS -DC4STL_SANITIZE=ON -DC4STL_SANITIZE_ONLY=ON -DC4STL_ASAN=OFF -DC4STL_TSAN=ON  -DC4STL_MSAN=OFF -DC4STL_UBSAN=OFF"
elif [ "$SAN" == "M"   ] ; then CMFLAGS="$CMFLAGS -DC4STL_SANITIZE=ON -DC4STL_SANITIZE_ONLY=ON -DC4STL_ASAN=OFF -DC4STL_TSAN=OFF -DC4STL_MSAN=ON  -DC4STL_UBSAN=OFF"
elif [ "$SAN" == "UB"  ] ; then CMFLAGS="$CMFLAGS -DC4STL_SANITIZE=ON -DC4STL_SANITIZE_ONLY=ON -DC4STL_ASAN=OFF -DC4STL_TSAN=OFF -DC4STL_MSAN=OFF -DC4STL_UBSAN=ON"
else
    CMFLAGS="$CMFLAGS -DC4STL_SANITIZE=OFF"
fi

if [ "$VG" == "ON" ] ; then
    CMFLAGS="$CMFLAGS -DC4STL_TEST_VALGRIND=ON"
elif [ "$VG" == "OFF" ] ; then
    CMFLAGS="$CMFLAGS -DC4STL_TEST_VALGRIND=OFF"
fi

if [ "$BM" == "ON" ] ; then
    CMFLAGS="$CMFLAGS -DC4STL_BUILD_BENCHMARKS=ON"
elif [ "$BM" == "OFF" ] || [ "$BM" == "" ] ; then
    CMFLAGS="$CMFLAGS -DC4STL_BUILD_BENCHMARKS=OFF"
fi

if [ ! -z "$CMFLAGS" ] ; then
    echo "additional cmake flags: $CMFLAGS"
fi

if [ "$A" == "32" ] ; then
    XFLAGS="-m32"
fi

#--------------------------------------------------

if [ "$SONARQUBE" != "1" ] ; then
    cd build
    cmake -DCMAKE_C_COMPILER=$CC_ -DCMAKE_CXX_COMPILER=$CXX_ \
          -DCMAKE_BUILD_TYPE=$BT -DCMAKE_CXX_FLAGS="$XFLAGS" \
          -DC4STL_EXTERN_DIR=`pwd`/extern_install \
          -DC4STL_DEV=ON \
          $CMFLAGS \
          $C4STL_DIR
    make help | sed 1d | sort
    make CTEST_OUTPUT_ON_FAILURE=1 c4stl-test
    cd -
else
    sonar-scanner
fi

#--------------------------------------------------

exit 0
