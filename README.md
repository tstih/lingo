# Lingo

Lingo is a C++20 framework for creating recursive descent parsers. Followign the
*simplicity before performance* philosophy, Lingo should be used for parsing 
small to medium sized files. 

 > Lingo is currently under development. Expect strange things.

## Introduction

With Lingo you can mimic Extended Backus–Naur form or Extended Backus 
normal form (EBNF) for context free grammars in C++20. Lingo is 
implemented as a single header file. To use it, simply include `lingo.hpp`.

### Defining a grammar

Let us first define a simple grammar to parse comma delimited non-floating 
point numbers.

~~~
<comma> = ","
<digit> = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
<number> = <digit>+
<num_list> = <number> (<comma> <number>)*
~~~

In lingo, we can write this grammar like this.

~~~cpp
lingo::rule comma(',');
lingo::rule digit('0','9'); 
lingo::rule number = lingo::repeat(digit,1); // Min. repetitions is 1.
lingo::rule num_list = number + lingo::repeat(comma + number, 0);
~~~

## Grammars

To define a grammar you use the `rule` class, and the  `placeholder` 
class. Placeholders are used to implement recursion. A placeholder
is a forward definition of a not-yet-defined rule. When you define it, 
you can attach it to the placeholder.

Following is a typical recursive grammar for numerical expressions.

~~~ 
<expression> = ["+"|"-"] <term> {("+"|"-") <term>} .
<term> = <factor> {("*"|"/") <factor>} .
<factor> = <number> | "(" <expression> ")" 
~~~

Here is its' implementation in lingo.

~~~cpp
lingo::rule lparen('(');
lingo::rule rparen(')');
lingo::rule plus('+');
lingo::rule minus('-');
lingo::rule asterisk('*');
lingo::rule slash('/');
lingo::rule digit('0','9');
lingo::rule number = lingo::repeat(digit, 1); // At least 1 digit.

lingo::placeholder expression_placeholder; // For recursion.
lingo::rule factor = number | ( lparen + expression_placeholder + rparen );
lingo::rule term = factor + lingo::repeat( ( asterisk | slash ) + factor, 0);
lingo::rule expression = lingo::repeat(plus | minus,0,1) + term + lingo::repeat ( (plus | minus) + term, 0);
expression_placeholder.set(expression); // Wire the recursion.
~~~

## Named rules

To create a meaningful message when a parse error occurs (such as:
'identifier expected at line 12 column 3') you need to assign names
to rules. 

Rule constructors have their twin which accepts rule name as the first
parameter.

~~~cpp
lingo::rule anon_digit('0','9'); // Anonymous rule.
lingo::rule named_digit("digit",'0','9'); // Named rule.
lingo::rule anon_number = lingo::repeat(digit, 1);
lingo::rule named_number("number", lingo::repeat(digit, 1));
~~~

You don't have to assign the name to every rule. When an error happens it will find
the closest parent rule with a name and use it to generate a message.

## Parsing

### Providing a source class

The `lingo::source` is used to abstract access to source code and provide basic
operations such as: tracking row and column in source file, reading symbols from
stream and returning them into the stream if parser requries a retry. 

This class already implement functionality, required to parse source code 
as `std::string`. To provide more sophisticated (optimized!) logic, derive 
your class from `lingo::source` and implement it yourself. 

### Parse 

(...coming soon...)

### Errors

When parsing fails, lingo throws a `lingo::parse_exception`, which holds human
readable error message, and the position of error (row and column!). 

## Creating an abstract syntax tree

(...coming soon...)

## Diagnostics

### Exporting your grammar to graphviz

You can pass the `lingo::graphviz_export_node_visitor` class to any lingo rule
and it will generate [graphviz](https://graphviz.org/) code which you can then pipe into 
the **graphviz** to create a diagram of that rule's grammar.

To use this class you need to define `DIAGNOSTICS` before including the lingo header.
You create the visitor and pass it to any rule by calling the `accept()` function.
After visiting the rule you can retreive **graphviz** code with a help of the `str()`
function. 

~~~cpp

// Define DIAGNOSTICS.
#define DIAGNOSTICS
#include "lingo.hpp"

// Define your rule: lingo::rule myrule = ...

// Now export to graphviz.
lingo::graphviz_export_node_visitor v;
myrule.accept(v);
std::cout << v.str();
~~~

### Exporting your grammar to EBNF

(...coming soon...)

## Internals

### Node tree visitors

(...coming soon...)

## Join the lingo team!

(...coming soon...)