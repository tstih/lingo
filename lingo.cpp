#include <iostream>

#define DIAGNOSTICS
#include "lingo.hpp"

int main() {

    // RECURSIVE GRAMMAR
    //  <expression> = ["+"|"-"] <term> {("+"|"-") <term>} .
    //  <term> = <factor> {("*"|"/") <factor>} .
    //  <factor> = <number> | "(" <expression> ")" 

    lingo::rule lparen('(');
    lingo::rule rparen(')');
    lingo::rule plus('+');
    lingo::rule minus('-');
    lingo::rule asterisk('*');
    lingo::rule slash('/');
    lingo::rule digit('0','9');

    lingo::placeholder expression_placeholder; // For recursion.
    lingo::rule number = lingo::repeat(digit, 1);
    lingo::rule factor = number | ( lparen  + expression_placeholder + rparen );
    lingo::rule term = factor + lingo::repeat( (asterisk | slash) + factor, 0);
    lingo::rule expression = lingo::repeat(plus|minus,0,1) + term + lingo::repeat ( (plus | minus) + term, 0);
    expression_placeholder.set(expression); // Wire the recursion.

    // Generate graphviz graph.
    lingo::graphviz_export_node_visitor v;
    expression.accept(v);
    std::cout << v.str();

    return 0;
}