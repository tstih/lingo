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
    lingo::rule identifier = (underscore | letter) + lingo::repeat(alphanum) + lingo::placeholder();

    identifier.print();

    return 0;
}