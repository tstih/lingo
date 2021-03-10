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
<num_list> = <number> (<comma> <digit>)*
~~~

In lingo, we can write this grammar like this.

~~~cpp
lingo::rule comma(',');
lingo::rule digit('0','9'); 
lingo::rule number = lingo::repeat(digit,1); // Min. repetitions is 1.
lingo::rule num_list = number + lingo::repeat(comma + number, 0);
~~~

### Recursive grammar

Following is a typical expression parsing grammar.

~~~ 
<expression> = ["+"|"-"] <term> {("+"|"-") <term>} .
<term> = <factor> {("*"|"/") <factor>} .
<factor> = <number> | "(" <expression> ")" 
~~~

To define it, we need to use a placeholder. And later set it
to the correct rule.

### Parsing a file

### Creating an abstract syntax tree