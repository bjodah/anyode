#!/bin/bash -xeu

export CFLAGS="-Werror -DNPY_NO_DEPRECATED_API=NPY_1_7_API_VERSION"
export ASAN_OPTIONS=detect_leaks=0

for py_vers in '3.11.*-release' '3.12-apt-deb' '3.12.*-asan' '3.13.*-asan' '3.13.*-release'; do  # from triceratops/test-3.sh
    if [[ $py_vers == *-asan ]]; then
        export CC=clang
        export CXX=clang++
    else
        export CC=gcc
        export CXX=g++
    fi

    for dir in midpoint eulerfw; do
        cd tests/$dir
        if [ -d ./build/ ]; then
            rm -r ./build/
        fi
        BIN_DIR_PY=$(compgen -G "/opt-3/cpython-v${py_vers}/bin")
        if [ -e $BIN_DIR_PY/activate ]; then
            source $BIN_DIR_PY/activate
            PYTHON=python
        else
            PYTHON=$BIN_DIR_PY/python3
        fi
        git clean -xfd
        $PYTHON -m pip install "setuptools==72.1.0"  # temporary work-around, see https://github.com/pypa/setuptools/issues/4748
        $PYTHON -m pytest -v -s
        cd -
    done
done
export ASAN_OPTIONS=""


for dir in tests/decomp tests/matrix; do
    cd $dir
    make -B CXX=g++ DEFINES="-D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2"
    make -B CXX=g++ DEFINES=-DNDEBUG

    LIBCXX_ROOT=$(compgen -G "/opt-2/libcxx??-debug")
    make -B CXX=clang++ EXTRA_FLAGS="-fsanitize=address -nostdinc++ -isystem ${LIBCXX_ROOT}/include/c++/v1" LDFLAGS="-nostdlib++ -Wl,-rpath,${LIBCXX_ROOT}/lib -L${LIBCXX_ROOT}/lib" LDLIBS="-lc++"

    LIBCXX_ROOT=$(compgen -G "/opt-2/libcxx??-asan")
    make -B CXX=clang++ EXTRA_FLAGS="-fsanitize=address -nostdinc++ -isystem ${LIBCXX_ROOT}/include/c++/v1" LDFLAGS="-nostdlib++ -Wl,-rpath,${LIBCXX_ROOT}/lib -L${LIBCXX_ROOT}/lib" LDLIBS="-lc++" DEFINES=-DNDEBUG
    cd -
done
