CXX ?= g++
LIBS ?=-llapack -lblas
INCLUDE ?= -I../../include
CXXFLAGS ?= -std=c++11 -Wall -Wextra -Werror -pedantic
DEFINES ?=

.PHONY: test clean

test: test_svd test_svd_ndebug
	./test_svd --abortx 1
	./test_svd_ndebug --abortx 1

clean:
	rm catch.hpp
	rm test_svd

test_%: test_%.cpp ../../include/anyode/anyode_decomposition.hpp catch.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(DEFINES) -g -ggdb -O0 -D_GLIBCXX_DEBUG -o $@ $< $(LIBS)

test_%_ndebug: test_%.cpp ../../include/anyode/anyode_decomposition.hpp catch.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(DEFINES) -O1 -DNDEBUG -o $@ $< $(LIBS)

catch.hpp: ../catch.hpp.bz2
	bzcat $< >$@
