# Assignment overview
Read a program in a simple human-oriented programming language from the standard input, and output an equivalent code that can be processed by a stack-based machine.

## Input program
The input program contains two main constructions: Expressions and Programs.

Expressions are simple arithmetic expression with parentheses, numbers, variables, and binary operators. Variables are any group of lowercase letters (i.e. written as [a-z]+ in regex); all numbers are non-negative integers, and the only allowed binary operators are '+', '-' and '*'.

To simplify parsing of expressions, use a simple recursive descent parser (see below), and assume that all operators are right-associative. The usual priorities of operators (multiplicative before additive) should hold.

For example:

1+2*3 must parse as 1+(2*3)
(1+2)*3 must obviously parse as (1+2)*3
1+3*3*3 must parse as 1+(3*(3*3))
operators in 1+1-1+1-1are right-associative without any priority, so they must parse as 1+(1-(1+(1-1))).
a, asd, asdadasdadasdasdadsa are great names of variables
a+b-c*d-5 should parse as a+(b-((c*d)-5))
Programs consist of "statements" that may contain expressions:

assignment statement begins with =, continues with a single variable name, and then with expression. For example, increasing the value of a by 1 can be done as = a a+1
statements can be combined using ;, for example a program =a a+1;=a a+1 increments the variable a twice
groups of statements may be delimited using { and }
variable input is marked by > and output as <, e.g. a program >tmp ; < tmp reads a number and immediately outputs it
while-cycle is written as @ variable statement -- it repeats statement until the variable becomes false. (Note the "condition" only contains a single variable, not expressions in general.) The definition of trueness and falseness does not concern us, but you can imagine that "true" means "non-zero"; in that case the program >a; @a {<a ; =a a-1}; <a would read a number and write out all numbers from it to zero, in decreasing order. Note that statements without braces are acceptable too: @a=a a-1 is a valid program.
conditional is written as ? variable statement, the statement is executed only if the variable is true
negative conditional, written as ! variable statement, executes the statement only if the variable is false.
Note that all whitespace (newlines, spaces, tabs) is ignored and serves only for separating the variable names. The program input is terminated by EOF.

## Output program
The output programming language is a simple list of instructions for a stack-based machine that may resemble assembly, FORTH, or a simple postfix calculator.

During the computation, it maintains a stack (last-in-first-out-style queue) of values. Results of the instructions are added to the top of the stack, parameters of individual instructions are removed from the top of the stack.

Additionally, the machine has a variable lookup table that stores values of individual named variables.

### The supported instructions are as such:

INT xxx pushes a single integer constant to a stack, e.g. INT 1 pushes 1.
LOADVAR xxx finds a variable named xxx in the variable memory and pushes its value to the stack.
STOREVAR xxx pops a value off the stack and stores it in variable xxx in the variable memory .
ADD, SUB, and MULT pop 2 values off the stack and push the result of the corresponding operation
READ reads a number from the user and pushes it to the stack
WRITE pops a number from the stack and outputs it for the user
QUIT terminates the program, and must occur exactly once at the end of each program
JMP xxx makes a relative jump in the program -- the next executed instruction will be the one at position current_position+xxx in the program. For example, JMP -1 will jump to the previous instruction, JMP 1 just skips to the next instruction, JMP 2 skips the next instruction and continues with the one after that, and JMP 0 is guaranteed to cycle forever.
JMPT xxx and JMPF xxx are like JMP xxx, but they additionally pop a number from the stack and only make the jump if the number is true (in case of JMPT) or false (in case of JMPF). Otherwise they "do nothing" and continue with the next instruction.
