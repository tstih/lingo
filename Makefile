# Rules.
.PHONY: all
all: lingo

# Just build.
lingo: lingo.cpp rule.hpp node.hpp 
	$(CXX) -std=c++2a -ggdb -o lingo lingo.cpp rule.cpp 