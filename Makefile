CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++14 -O3 --param l1-cache-size=32 --param l1-cache-line-size=32 --param l2-cache-size=256 -ftree-vectorize -foptimize-sibling-calls -ffast-math -funsafe-math-optimizations  -floop-interchange -floop-block -ftree-loop-distribution -fassociative-math

all: full_dynamic_prog optimal_dynamic_prog ya_dynamic_prog

full_dynamic_prog: full_dynamic_prog.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

optimal_dynamic_prog: optimal_dynamic_prog.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

ya_dynamic_prog: ya_dynamic_prog.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f full_dynamic_prog optimal_dynamic_prog ya_dynamic_prog
