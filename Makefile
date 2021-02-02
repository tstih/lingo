# Configure.
export RM = rm -f

# Rules.
.PHONY: all
all: lingo

# Just build.
lingo: lingo.cpp lingo.hpp
	$(CXX) -std=c++2a -ggdb -o lingo lingo.cpp 

.PHONY: clean
clean:
	$(RM) lingo