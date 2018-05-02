CXX ?= g++
LIBS ?=-llapack -lblas
CXXFLAGS ?= -std=c++11 -Wall -Wextra -Werror -pedantic -g -ggdb
CXXFLAGS += $(EXTRA_FLAGS)
INCLUDE ?= -I../../include
DEFINES ?=


.PHONY: test clean

test: test_matrix
	./test_matrix --abortx 1

clean:
	rm catch.hpp
	rm test_decomp

test_%: test_%.cpp ../../include/anyode/anyode_matrix.hpp catch.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(DEFINES) -o $@ $< $(LIBS)

catch.hpp: ../catch.hpp.bz2
	bzcat $< >$@
