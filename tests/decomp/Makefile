CXX ?= g++

ifneq ($(ANYODE_NO_LAPACK), 1)
LIBS ?=-llapack -lblas
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

CXXFLAGS ?= -std=c++17 $(WARNINGS) -Werror -pedantic -g -ggdb -DANYODE_NO_LAPACK=$(ANYODE_NO_LAPACK)
CXXFLAGS += $(EXTRA_FLAGS)
INCLUDE ?= -I../../include
DEFINES ?=


.PHONY: test clean

test: test_decomp
	./test_decomp --abortx 1

clean:
	rm catch.hpp
	rm test_decomp

test_%: test_%.cpp ../../include/anyode/anyode_decomposition.hpp catch.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(DEFINES) -o $@ $< $(LIBS)

catch.hpp: ../catch.hpp.bz2
	bzcat $< >$@
