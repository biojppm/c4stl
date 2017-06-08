#!/usr/bin/env bash

set -e
set -x

# setup apt for installing
sudo -E add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo bash -c 'cat >> /etc/apt/sources.list <<EOF

deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.7 main
#deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.8 main # not needed
deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.9 main
deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-4.0 main
EOF'
sudo -E apt-get update

if [ ! -z "$DPKG" ] ; then
    echo "additional packages: $DPKG"
fi

# g++-5 is needed for clang; otherwise it uses libstdc++ from g++4.8
# which is not fully C++11
sudo -E apt-get install -y \
     build-essential \
     g++-5 g++-5-multilib \
     valgrind \
     $DPKG

if [ "${BUILD_TYPE}" == "Coverage" -a "${TRAVIS_OS_NAME}" == "linux" ]; then
    PATH=~/.local/bin:${PATH};
    pip install --user --upgrade pip;
    pip install --user cpp-coveralls;
fi

export CC_=$(echo "$CXX_" | sed 's:clang++:clang:g' | sed 's:g++:gcc:g')
$CXX_ --version
$CC_ --version
cmake --version

C4STL_DIR=$(pwd)
echo $C4STL_DIR
pwd

# compile and install external libraries
mkdir -p build/extern_build && cd build/extern_build
cmake -DCMAKE_C_COMPILER=$CC_ -DCMAKE_CXX__COMPILER=$CXX_ \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX__FLAGS="$XFLAGS" \
      -DCMAKE_INSTALL_PREFIX=`pwd`/../extern_install \
      $C4STL_DIR/extern
cmake --build . --config Release
cd -

exit 0
