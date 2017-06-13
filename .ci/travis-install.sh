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

  if [ "$CXX_" == "g++-5" ] && [ "$A" == "64" ] ; then DPKG="$DPKG"
elif [ "$CXX_" == "g++-5" ] && [ "$A" == "32" ] ; then DPKG="$DPKG"
elif [ "$CXX_" == "g++-6" ] && [ "$A" == "64" ] ; then DPKG="$DPKG g++-6"
elif [ "$CXX_" == "g++-6" ] && [ "$A" == "32" ] ; then DPKG="$DPKG g++-6 g++-6-multilib"
elif [ "$CXX_" == "g++-7" ] && [ "$A" == "64" ] ; then DPKG="$DPKG g++-7"
elif [ "$CXX_" == "g++-7" ] && [ "$A" == "32" ] ; then DPKG="$DPKG g++-7 g++-7-multilib"
elif [ "$CXX_" == "clang++-3.6" ] ; then DPKG="$DPKG clang-3.6"
elif [ "$CXX_" == "clang++-3.7" ] ; then DPKG="$DPKG clang-3.7"
elif [ "$CXX_" == "clang++-3.8" ] ; then DPKG="$DPKG clang-3.8"
elif [ "$CXX_" == "clang++-3.9" ] ; then DPKG="$DPKG clang-3.9"
elif [ "$CXX_" == "clang++-4.0" ] ; then DPKG="$DPKG clang-4.0"
fi

if [ "$A" == "32" ] ; then
    DPKG="$DPKG libc6-dbg:i386"
    XFLAGS="-m32"
fi

if [ ! -z "$DPKG" ] ; then
    echo "additional packages: $DPKG"
fi

# g++-5 is needed for clang; otherwise it uses libstdc++ from g++4.8
# which is not fully C++11 and does not have some important type traits
# such as std::is_trivially_move_constructible.
sudo -E apt-get install -y --force-yes \
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
cmake -DCMAKE_C_COMPILER=$CC_ -DCMAKE_CXX_COMPILER=$CXX_ \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_FLAGS="$XFLAGS" -DCMAKE_CXX_FLAGS="$XFLAGS" \
      -DCMAKE_INSTALL_PREFIX=`pwd`/../extern_install \
      $C4STL_DIR/extern
cmake --build . --config Release
cd -

exit 0
