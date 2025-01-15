#!/bin/bash -xeu

export CC=gcc
export CXX=g++
export CFLAGS="-Werror -DNPY_NO_DEPRECATED_API=NPY_1_7_API_VERSION"


for py_vers in '3.11.*-release' '3.12-apt-deb' '3.12.*-asan' '3.13.*-asan' '3.13.*-release'; do  # from triceratops/test-3.sh
    for dir in midpoint eulerfw; do
        cd tests/$dir
        if [ -d ./build/ ]; then
            rm -r ./build/
        fi
        ASAN_OPTIONS=detect_leaks=0 $(compgen -G "/opt-3/cpython-v${py_vers}/bin/python3") -m pytest -v
        cd -
    done
done

for dir in tests/decomp tests/matrix; do
    cd $dir
    make -B DEFINES="-D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2"
    make -B DEFINES=-DNDEBUG
    LIBCXX_ROOT=$(compgen -G "/opt-2/libcxx??-debug")
    make -B CXX=clang++ EXTRA_FLAGS="-fsanitize=address -nostdinc++ -isystem ${LIBCXX_ROOT}/include/c++/v1" LDFLAGS="-nostdlib++ -Wl,-rpath,${LIBCXX_ROOT}/lib -L${LIBCXX_ROOT}/lib" LDLIBS="-lc++"
    LIBCXX_ROOT=$(compgen -G "/opt-2/libcxx??-asan")
    make -B CXX=clang++ EXTRA_FLAGS="-fsanitize=address -nostdinc++ -isystem ${LIBCXX_ROOT}/include/c++/v1" LDFLAGS="-nostdlib++ -Wl,-rpath,${LIBCXX_ROOT}/lib -L${LIBCXX_ROOT}/lib" LDLIBS="-lc++" DEFINES=-DNDEBUG
    cd -
done
