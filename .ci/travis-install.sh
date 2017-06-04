#!/usr/bin/env bash

set -e
set -x


if [ $COMPILER == "clang++-3.7" ] ; then
    sudo bash -c 'cat >> /etc/apt/sources.list <<EOF

deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.7 main
EOF'
    sudo -E apt-get update
    sudo -E apt-get install -y clang-3.7
fi

sudo -E apt-get install -y build-essential valgrind
if [ "${BUILD_TYPE}" == "Coverage" -a "${TRAVIS_OS_NAME}" == "linux" ]; then
    PATH=~/.local/bin:${PATH};
    pip install --user --upgrade pip;
    pip install --user cpp-coveralls;
fi

export C_COMPILER=$(echo "$COMPILER" | sed 's:clang++:clang:g' | sed 's:g++:gcc:g')
$COMPILER --version
$C_COMPILER --version
cmake --version

C4STL_DIR=$(pwd)
echo $C4STL_DIR
pwd

# compile and install external libraries
mkdir -p build/extern_build && cd build/extern_build
cmake -DCMAKE_C_COMPILER=${C_COMPILER} -DCMAKE_CXX_COMPILER=${COMPILER} \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="${EXTRA_FLAGS}" \
      -DCMAKE_INSTALL_PREFIX=`pwd`/../extern_install $C4STL_DIR/extern
cmake --build . --config Release
cd -

exit 0
