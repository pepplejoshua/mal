# set this if not already set
# generates additional files
# CXXFLAGS ?= -g -Wall -Werror -Wextra
CXXFLAGS ?= -std=c++20

build: step5_tco

rm:
	make clean && make

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
	rm -rf step5_tco