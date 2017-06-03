#!/usr/bin/env bash

set -e
set -x

pwd

dpkg -L clang-3.7 || echo "none"

export C_COMPILER=$(echo "$COMPILER" | sed 's:clang++:clang:g' | sed 's:g++:gcc:g')
$COMPILER --version
$C_COMPILER --version
cmake --version

sudo apt-get install valgrind
if [ "${BUILD_TYPE}" == "Coverage" -a "${TRAVIS_OS_NAME}" == "linux" ]; then
    PATH=~/.local/bin:${PATH};
    pip install --user --upgrade pip;
    pip install --user cpp-coveralls;
fi

# compile and install external libraries
mkdir -p build/extern_build && cd build/extern_build
cmake -DCMAKE_C_COMPILER=${C_COMPILER} -DCMAKE_CXX_COMPILER=${COMPILER} \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="${EXTRA_FLAGS}" \
      -DCMAKE_INSTALL_PREFIX=`pwd`/../extern_install ../../extern
cmake --build . --config Release
cd -

exit 0
