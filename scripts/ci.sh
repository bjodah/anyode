#!/bin/bash -xeu
cd tests/eulerfw; python -m pytest; cd -
export ASAN_SYMBOLIZER_PATH=/usr/lib/llvm-3.8/bin/llvm-symbolizer
export ASAN_OPTIONS=symbolize=1
for dir in tests/decomp tests/matrix; do 
    cd $dir; make -B DEFINES=-D_GLIBCXX_DEBUG; cd -
    cd $dir; make -B DEFINES=-DNDEBUG; cd -
    cd $dir; make -B CXX=clang++-3.8 EXTRA_FLAGS="-fsanitize=address"; cd -
done
