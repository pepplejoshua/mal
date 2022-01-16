# set this if not already set
# generates additional files
# CXXFLAGS ?= -g -Wall -Werror -Wextra
CXXFLAGS ?= -std=c++20

build: step2_eval

step2_eval: step2_eval.cpp reader.cpp printer.cpp
	clang++ $(CXXFLAGS) -o $@ $^

step1_read_print: step1_read_print.cpp reader.cpp printer.cpp
	clang++ $(CXXFLAGS) -o $@ $^
# is equivalent to
# clang++ $(CXXFLAGS) -o step1_read_print step1_read_print.cpp reader.cpp printer.cpp
# where $@ is step1_read_print and
# $^ is step1_read_print.cpp reader.cpp printer.cpp

step0_repl: step0_repl.cpp
# CXX is the c++ compiler 
	clang++ $(CXXFLAGS) -o step0_repl step0_repl.cpp

clean:
	rm -rf step2_eval