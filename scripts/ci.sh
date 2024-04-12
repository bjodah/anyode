#!/bin/bash -xeu

export PATH=$(compgen -G "/opt-3/cpython-v3.11*-apt-deb/bin"):/opt-2/gcc-13/bin:$PATH

cd tests/midpoint; if [ -d ./build/ ]; then rm -r ./build/; fi; python3 -m pytest -v; cd -
cd tests/eulerfw;  if [ -d ./build/ ]; then rm -r ./build/; fi; python3 -m pytest -v; cd -
for dir in tests/decomp tests/matrix; do 
    cd $dir; make -B DEFINES="-D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2"; cd -
    cd $dir; make -B DEFINES=-DNDEBUG; cd -
    cd $dir; make -B CXX=clang++-12 EXTRA_FLAGS="-fsanitize=address"; cd -
    cd $dir; make -B CXX=clang++-12 EXTRA_FLAGS="-fsanitize=address" DEFINES=-DNDEBUG; cd -
done
