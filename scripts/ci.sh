#!/bin/bash -xeu

export PATH=:/opt-2/gcc-13/bin:$PATH


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

for py_vers in '3.11-apt-deb' '3.11.*-debug' '3.12.*-debug' '3.12.*-asan' '3.12.*-release' '3.11.*-asan'; do
    for dir in midpoint eulerfw; do
        cd tests/$dir
        if [ -d ./build/ ]; then
            rm -r ./build/
        fi
        ASAN_OPTIONS=detect_leaks=0 $(compgen -G "/opt-3/cpython-v${py_vers}/bin/python3") -m pytest -v
        cd -
    done
done
