CXX = clang++
CXXFLAGS = -Wall -Wextra -std=c++14 -O3

all: full_dynamic_prog

full_dynamic_prog: full_dynamic_prog.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f full_dynamic_prog
