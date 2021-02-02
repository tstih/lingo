# Lingo

Lingo is a C++20 framework for creating recursive descent parsers. Followign the
*simplicity before performance* philosophy, Lingo should be used for parsing 
small to medium sized files. 

 > Lingo is currently under development. Please come back later.

## Introduction

With Lingo you can mimic Backus–Naur form or Backus normal form (BNF) for
context free grammars in C++20. Lingo is implemented as a single header
file. To use it, simply include `lingo.hpp`.

### Defining a grammar

~~~
<comma> = ","
<digit> = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
<number> = <digit> [(<digit>)+]
<list> = <number> [(<comma><digit>)+]
~~~

becomes

~~~cpp
lingo::rule comma(',');
lingo::rule digit({'0','1','2','3','4','5','6','7','8','9'}); 
lingo::rule number = digit + lingo::repeat(digit,0); // Min. repetitions is 0.
lingo::rule digit_list = digit + lingo::repeat(comma + digit, 0);
~~~

### Parsing a file

### Creating an abstract syntax tree