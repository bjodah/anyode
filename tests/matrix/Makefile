CXX ?= g++

ifneq ($(ANYODE_NO_LAPACK), 1)
EXTRA_LIBS = -llapack -lblas
endif

WARNINGS ?= \
-Wall \
-Wextra \
-Wredundant-decls \
-Wcast-align \
-Wmissing-include-dirs \
-Wswitch-enum \
-Wswitch-default \
-Winvalid-pch \
-Wredundant-decls \
-Wformat=2 \
-Wmissing-format-attribute \
-Wformat-nonliteral \
-Wodr
# -Wmissing-declarations

CXXFLAGS ?= -std=c++20 $(WARNINGS) -Werror -pedantic -g -ggdb -DANYODE_NO_LAPACK=$(ANYODE_NO_LAPACK)
CXXFLAGS += $(EXTRA_FLAGS)
INCLUDE ?= -I../../include
DEFINES ?=


.PHONY: test clean

test: test_matrix
	./test_matrix --abortx 1

clean:
	rm doctest.h
	rm test_decomp

test_%: test_%.cpp ../../include/anyode/anyode_matrix.hpp doctest.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(INCLUDE) $(DEFINES) -o $@ $< $(LDLIBS) $(EXTRA_LIBS)

doctest.h: ../doctest.h.bz2
	bzcat $< >$@
