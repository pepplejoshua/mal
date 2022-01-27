# set this if not already set
# generates additional files
# CXXFLAGS ?= -g -Wall -Werror -Wextra
CXXFLAGS ?=  -include-pch ../linenoise.hpp.gch -std=c++20 -fuse-ld=/usr/local/opt/llvm/bin/ld64.lld

build: step6_file

rm:
	make clean && make

step6_file: step6_file.cpp reader.cpp printer.cpp mal_types.cpp
	clang++ $(CXXFLAGS) -o $@ $^

step5_tco: step5_tco.cpp reader.cpp printer.cpp mal_types.cpp
	clang++ $(CXXFLAGS) -o $@ $^

step4_if_fn_do: step4_if_fn_do.cpp reader.cpp printer.cpp mal_types.cpp
	clang++ $(CXXFLAGS) -o $@ $^

step3_env: step3_env.cpp reader.cpp printer.cpp mal_types.cpp
	clang++ $(CXXFLAGS) -o $@ $^

step2_eval: step2_eval.cpp reader.cpp printer.cpp mal_types.cpp
	clang++ $(CXXFLAGS) -o $@ $^

step1_read_print: step1_read_print.cpp reader.cpp printer.cpp  mal_types.cpp
	clang++ $(CXXFLAGS) -o $@ $^
# is equivalent to
# clang++ $(CXXFLAGS) -o step1_read_print step1_read_print.cpp reader.cpp printer.cpp
# where $@ is step1_read_print and
# $^ is step1_read_print.cpp reader.cpp printer.cpp

step0_repl: step0_repl.cpp
# CXX is the c++ compiler 
	clang++ $(CXXFLAGS) -o step0_repl step0_repl.cpp

clean:
	rm -rf step6_file