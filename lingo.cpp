#include <iostream>

#define DIAGNOSTICS
#include "lingo.hpp"

// goal: 
// expression = ["+"|"-"] term {("+"|"-") term} .
// term = factor {("*"|"/") factor} .
// factor = number | "(" expression ")" .

int main() {

    lingo::rule digit('0','9');
    lingo::rule underscore('_');
    lingo::rule letter = lingo::rule('a','z') | lingo::rule('A','Z');
    lingo::rule alphanum = letter | digit | underscore;
    lingo::rule input("input");
    lingo::rule ws({' ', '\n', '\t', '\r'});
    lingo::rule not_letter=!letter;
    lingo::rule identifier ("<identifier>", (underscore | letter) + lingo::repeat(alphanum) ); 

    /*
    lingo::source test1("!@joo");
    if (identifier.parse(test1))
        std::cout << "Success!" << std::endl;
    else
        std::cout << "Fail!" << std::endl;
    */

    lingo::graphviz_export_node_visitor v;
    identifier.accept(v);
    std::cout << v.str();

    return 0;
}