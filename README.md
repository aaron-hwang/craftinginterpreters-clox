# Overview
This project is a stack based virtual machine interpreter for the 
Lox language outlined in the crafting interpreters book. 

This project has several key components:
A scanner, which takes human readable source code of lox and converts it into 'Tokens'. 

These tokens are then taken by our compiler/parser, which converts said tokens into pieces of bytecode for the virtual machine to interpret.
The compiler is single pass, using pratt parsing to actually comb through the outputted tokens.

Finally, the bytecode outputted by the compiler is interpreted by our VM.

# Differences
This project has the same end goal of the java version, which is to interpret Lox source code
and output its result. The fundamental difference between the two is the design and architecture used to achieve this.

The Java version is simpler conceptually and easier to implement, parsing the source code into an 
AST of Tokens, which was then interpreted by walking said AST recursively. 
This made it conceptually easy to understand, but came with massive performance drawbacks, including but not limited to:
- Garbage Collection is forced in Java, occasional GC pause does add up (although modern GC is so good that this is most likely not the biggest bottleneck, it does matter...)
- AST Overhead: Every node in the AST is an object in Java, so walking the AST under the hood in the jvm involves a massive amount of pointer/reference indirection, making it inherently slower compared to a stack based bytecode approach
- As previously mentioned, a stack based approach is significantly faster; No complicated AST walking required, literally just pop objects off the stack as the program is run. 

# Features:
Features currently implemented that are outlined by the book:
- Functions
- Variables
- Closures/local scope for functions and variables
- Dynamically typed variables
