#include <iostream>

#include <gtest/gtest.h>

#include "../include/lingo.hpp"

TEST(parse_test_cases, and_rule)
{
    lingo::rule and_expression=lingo::rule('(') + lingo::rule('0','9') + lingo::rule(')');
    std::string content="(5)";
    lingo::string_source src(content);
    EXPECT_EQ(and_expression.parse(src), true);
}

TEST(parse_test_cases, or_rule)
{
    lingo::rule or_expression=lingo::rule({'x','y','z'});
    std::string content="y";
    lingo::string_source src(content);
    EXPECT_EQ(or_expression.parse(src), true);
}

TEST(parse_test_cases, expr_rule)
{
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

    EXPECT_EQ(1, 1);
}