#include <iostream>

#define DIAGNOSTICS // We need this for graphviz support.
#include "lingo.hpp"

int main() {

    lingo::rule digit('0','9');
    lingo::rule integer = lingo::repeat(digit,1,5);
    lingo::rule underscore('_');
    lingo::rule letter = lingo::rule('a','z') | lingo::rule('A','Z');
    lingo::rule alphanum = letter | digit | underscore;
    lingo::rule input("input");
    lingo::rule ws({' ', '\n', '\t', '\r'});
    lingo::rule not_letter=!letter;
    lingo::rule identifier ("<identifier>", (underscore | letter) + lingo::repeat(alphanum, 1, 31) ); 

    // Generate graphviz graph.
    lingo::graphviz_export_node_visitor v;
    identifier.accept(v);
    std::cout << v.str();

    return 0;
}