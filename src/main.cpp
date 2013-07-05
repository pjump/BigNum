/*BigNum—Basic functionality of a simple arbitrary precision calculator similar to GNU bc.
Copyright (C) 2013  Petr Skocik

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License, version 2,
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA. 
*/
//#define DEBUG
//#define private public
//#define DEBUG_RPN
/**
\mainpage 

\section about BigNum—An Arbitrary Precision Computation Suite


BigNum is a set of well-performing classes that allow for relatively high-performance basic computation with arbitrary-precision numbers.
The project was partially inspired by GNU *bc*.

The core of the project lies in the BigNum::BigN templated class, which represents a decimal, arbitrary precision number.
The class is parametrized by the type that it will be using as an elementary storage unit, or cell, as it is referred to in the code.
This will most likely be either **unsigned char** or **uintmax_t**. While using **unsigned chars** will generate code that is,
for addition and subtraction, both formally and performance-wise equivalent to that of **bc**, it is rather wasteful in terms of storage efficiency.
Using larger data types makes BigNum more efficient both in terms of storage space economy and performance, especially when it comes to multiplication.
(Addition and subtraction are actually slightly slower, possibly due to somewhat more involved shifting).
On my 64 bit computer , BigNum with 64-bit cells computes products  (even if they're thousands of digits long') about three times faster than *bc*, despite the fact that, unlike bc, BigNum  only uses long multiplication.

The Calculator class exposes its functionality by providing a user interface that accepts standardly formatted arithmetic formulae.
It is again, a templated class, which means that BigNum::BigN<>'s can be seamlessly swapped for any other numerical type, and the Calculator will then work with that type instead. Using native types should lead to virtually no performance loss, as the Calculator class will work with those types directly, without using pointers. Since this is counterproductive for large types such as BigN or std::valarrays, for instance, the ShrdNum class is presented, which represents a thin wrapper around numerical types that exposes their functionality but only stores *std::shared_ptr*'s internally. This speeds up copying of large numerical types significantly.

The Calculator provides standard template code for all basic numerical operators in terms of the C++ operators defined for the underlying type. It accepts infix notation and supports variables, fixed-arity and variadic functions, and custom operators.



*This is the documentation of a semestral project for Programming and Algorithms 2, an undergraduate course at Czech Technical University.*<br>
\date June 2013
\author Petr Skocik
\copyright GNU GPLv2
*/

///@file main.cpp

#include "Calculator.hpp"
#include "ShrdNum.hpp"
#include "BigN.hpp"
#include <iostream>
#include <fstream>

#ifdef USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/io.h>
#endif 

using namespace std;
using namespace BigNum;

/**\namespace BigNum
 * \brief Project's main namespace
 * 		Virtually all of the project's code is in this namespace
 */

#ifndef STYPE
///Which type will be used a storage unit for BigN<>'s
#define STYPE u8
#endif

/// SBigN—a shared BigN—a type that will behave as a number but will internally manage  pointers to shared storage
typedef ShrdNum<BigN<STYPE>> SBigN;
/// Calc—instantiate the Calculator to use SBigN to represent its numbers
typedef Calculator<SBigN> Calc;

///Reads from cin and might use GNU readline if the USE_READLINE macro is defined and the program is linked with -lreadline.
///\param calc A reference to the Calculator to be used
///\return Number of erroneous statements in input
int ReadFromCin(Calc& calc) {
	#ifdef USE_READLINE
	if(isatty(fileno(stdin))){
	  int fails=0; 	
	  char *buf;
	  rl_bind_key('\t',rl_abort);//auto-complete

	  while((buf = readline("\n>> "))!=NULL)
	  {
		  if (strcmp(buf,"quit")==0)
			  break;
		  if (buf[0]!=0)
		      add_history(buf);
		  //Shove a newline at the end—Calc needs it
		  size_t buflen=strlen(buf);
		  buflen+=2;
		  buf=(char*)realloc(buf,sizeof(char)*buflen);
		  buf[buflen-2]='\n'; buf[buflen-1]='\0';

		  istringstream is;
		  is.rdbuf()->pubsetbuf(buf,strlen(buf));
		  fails+=is>>calc;
	 }
	 free(buf);
	 return fails;
	}
	#endif
	return cin>>calc;
}

/** \brief
 * \arg argv The program will attempt to read from a file whose name is the first argument to it. If no argument is specified, it will read from *stdin*.
 * \return The program returns the return value of Calculator::ReadAndComp(). This will be 0 on success or the number of unsuccessfully processed Calculator statements. */
int main( int argc, const char *argv[] )
{
     Calc calc;
     
     if(argc>1) {
		  int fails=0;
          if(argc>2)
               cerr<<"Only one argument is required."<<endl;
          ifstream ifile (argv[1]);
          if(!ifile) {
               cerr << "Unable to open \""<<argv[1]<<"\" : "<<strerror(errno)<<endl;
               return 1;
          }
          Calc calc;
          //Read input from the file given as the first CL argument
          fails=ifile>>calc;
          return fails;
     }
     //If no file is provided, read from stdin
     return ReadFromCin(calc);
}
