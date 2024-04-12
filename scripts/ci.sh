#!/bin/bash -xeu
cd tests/midpoint; if [ -d ./build/ ]; then rm -r ./build/; fi; gdb -ex r -args python3 -m pytest --pdb -s; cd -
cd tests/eulerfw; python3 -m pytest; cd -
for dir in tests/decomp tests/matrix; do 
    cd $dir; make -B DEFINES="-D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2"; cd -
    cd $dir; make -B DEFINES=-DNDEBUG; cd -
    cd $dir; make -B CXX=clang++-12 EXTRA_FLAGS="-fsanitize=address"; cd -
    cd $dir; make -B CXX=clang++-12 EXTRA_FLAGS="-fsanitize=address" DEFINES=-DNDEBUG; cd -
done
