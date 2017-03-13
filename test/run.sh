#!/usr/bin/env bash

set -e

if [ $# -lt 2 ] ; then
    echo "USAGE: $0 <build-dir> <test-name> [<test-name2>] [<test-name3>] ..."
    exit 1
fi

bdir=$1
np=$(nproc --all)
targets=${@:2}  # http://stackoverflow.com/a/7762555/5875572

if [ ! -d $bdir ] ; then
    echo "ERROR: build dir $bdir does not exist"
    exit 1
fi

( cd $bdir && cmake . )
for t in $targets ; do
    if [ -z "$(cd $bdir ; make help | grep $t-build\$)" ] ; then
        ( cd $bdir && make -j $np $t )
    else
        ( cd $bdir && make -j $np $t-build )
    fi
    if [ "$t" == "unit_tests" ] ; then
        ( cd $bdir && make unit_tests )
    else
        ( cd $bdir && ctest -V -R $t )
    fi
done

exit 0
