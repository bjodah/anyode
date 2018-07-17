#!/bin/bash -xeu
cd tests/eulerfw; python3 -m pytest; cd -
for dir in tests/decomp tests/matrix; do 
    cd $dir; make -B DEFINES=-D_GLIBCXX_DEBUG; cd -
    cd $dir; make -B DEFINES=-DNDEBUG; cd -
    cd $dir; make -B CXX=clang++-6.0 EXTRA_FLAGS="-fsanitize=address"; cd -
    cd $dir; make -B CXX=clang++-6.0 EXTRA_FLAGS="-fsanitize=address" DEFINES=-DNDEBUG; cd -
done
