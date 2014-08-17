BigNum
======

*An arbitrary precision suite similar to GNU bc.*

Stores numbers decimally, but unlike bc, it can use larger integer types to store higher powers of ten in a single storage cell. This makes multiplication about 3 times faster than on GNU bc. (Additiona and subtractions are as fast as on GNU bc). 

Fully supports infix notation, custom operators, and variadic and fixed-arity functions.
Right now, division is not implemented.

Includes documentation written in doxygen. 
Generate it by running:

	make doc
The program can be compiled by running

	make compile

Running `make` will both compile the program and create the documentation.

*Prerequisites*
	Last tested on g++-4.8 and doxygen 1.8.6.

The code was created as part of a school project and is now published under GNU GPLv2.

