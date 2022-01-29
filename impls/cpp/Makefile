# set this if not already set
# generates additional files
# CXXFLAGS ?= -g -Wall -Werror -Wextra
CXXFLAGS ?=  -include-pch ../linenoise.hpp.gch -std=c++20 -fuse-ld=/usr/local/opt/llvm/bin/ld64.lld

build: step8_macros

rm:
	make clean && make

step8_macros: step8_macros.cpp reader.cpp printer.cpp mal_types.cpp
	clang++ $(CXXFLAGS) -o $@ $^

step7_quote: step7_quote.cpp reader.cpp printer.cpp mal_types.cpp
	clang++ $(CXXFLAGS) -o $@ $^

step6_file: step6_file.cpp 
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
	clang++ $(CXXFLAGS) -o step0_repl step0_repl.cpp

clean:
	rm -rf step8_macros