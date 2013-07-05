#ifndef CALCULATOR_FNS_AND_OPS_HPP_
#define CALCULATOR_FNS_AND_OPS_HPP_
#include <string>
#include <vector>
#include <cassert>
#include <cassert>
#include <stdexcept>

/**\file
 Definitions of functions that handle operators and functions in an expression. The signature of an operator handling function and the signature of a function-handling function remain the same.
*/
namespace BigNum {
using namespace std;

/** \namespace Op
 * \brief A namespace for the operator functions that will be used by Calculator.
 */
namespace Op {

/**Writes the signature of an operator function used by the Calculator class.
All operator functions assume that the first argument is writable and disposable and therefore can be used as scratch space for the operation.
\param F_NAME Function name. Must be unique within the namespace.
\note x Disposable first param to the binary infix operation.
\note y Second param to the binary infix oration.
*/
#define OP_SIG_(F_NAME) template<typename T> inline void F_NAME	(T* x, const T* y)

/**Defines an operator function for a common binary operation.
 The binary operator must have it's assignment-form counterpart defined (e.g., += for +)
 \param X The symbol used to execute the operation (e.g., +,-,*).
 \param F_NAME Name of the created function.
 */
#define OP_FUNC_(X,F_NAME) 	OP_SIG_(F_NAME)	{*x X##= *y;}

/**Defines an operator function for a common binary assignment function operation.
 \param X The symbol used to execute the operation (e.g., +=,-=,*=).
 \param F_NAME Name of the created function.
 */
#define OP_AFUNC_(X,F_NAME) OP_SIG_(F_NAME) 	{*x X *y;}

/**Defines an operator function for a common comparison function or a function that results in a boolean value.
 \param X The symbol used to execute the operation (e.g., >,<,&&).
 \param F_NAME Name of the created function.
 */
#define OP_CFUNC_(X,F_NAME) OP_SIG_(F_NAME) 	{ *x = ( *x X *y);}
/**Defines an operator function for a common binary operation.
 The binary operator must have it's assignment-form counterpart defined (e.g., -= for -)
 \param X The symbol used to execute the operation (e.g., +,-,!).
 \param F_NAME Name of the created function.
 */
#define OP_UNFUNC_(X,F_NAME)	OP_SIG_(F_NAME) 	{*x = X(*x);}
OP_FUNC_(*,times) 	///< Multiplication
/*OP_FUNC_(%,mod)*/	///< Modulo
OP_FUNC_(/,div)		///< Division
OP_FUNC_(-,minus)	///< Subtraction	
OP_FUNC_(+,plus)	///< Addition
OP_CFUNC_(<,lt)		///< Less than
OP_CFUNC_(<=,lte)	///< Less than or Equal
OP_CFUNC_(>,gt)		///< Greater than
OP_CFUNC_(>=,gte)	///< Greater than or Equal	
OP_CFUNC_(==,eq)	///< Equal
OP_CFUNC_(!=,neq)	///< Not Equal
OP_CFUNC_(&&,land)	///< Logical and
OP_CFUNC_(||,lor)	///< Logical or
OP_AFUNC_(=,assign)	
OP_AFUNC_(+=,aplus)	///< +=
OP_AFUNC_(-=,aminus)	///< -=
OP_AFUNC_(*=,atimes)	///< *=
OP_AFUNC_(/=,adiv)	///< /=
//OP_AFUNC_(%=,amod)	///< %u
OP_UNFUNC_(+,uplus)	///< Unary Plus
OP_UNFUNC_(-,uminus)	///< Unary Minus
OP_UNFUNC_(!,ulnot)	///< (Unary ) Logical Negation

///A no-op function
OP_SIG_(noop) {}

///^, Raise x to the power of y
/// only works with integral exponents
/*^*/ OP_SIG_(raise)
{

     T b=1;
     T c=*x;
     for(int i=*y; i>0; --i)
          b*=c;
     *x=b;
}
///Calculate the factorial of x
OP_SIG_(ofact)
{
     T tmp=1;
     T one=1;
     for(; *x>one; --(*x))
          tmp*=*x;
     *x=tmp;
}
#undef OP_SIG_
#undef OP_FUNC_
#undef OP_AFUNC_
#undef OP_CFUNC_
#undef OP_UNFUNC_
}  //namespace Op


namespace Fn {
/// \brief Exception to be thrown if a function is called with a wrong number of arguments.
struct exBadArgN: runtime_error {
     exBadArgN(int received_argn, int wanted_argn, const string& name) :
          runtime_error(string("Function \"") + name + "\" takes " + to_string(wanted_argn) + " arguments, but " + to_string(received_argn) + " arguments have been received." ) {}
};

//FIXED SIZE FUNCTIONS
/**Macro to start the definition of a fixed-arity function
\note Functions have been programmed in just for fun. It's not really part of the assignment.
\param NAME The name of the defined function. Must be unique within the Fn namespace.
\param NARG Number of required arguments. An exBadArgN will be thrown if this specification is not satisfied.
*/
#define FDEFBEG_(NAME,NARG)  template<typename T> void NAME(int n, T* args []) { if(n!=NARG) throw exBadArgN(n,NARG,#NAME); //make sure that the right number of args have been loaded
///Macro to end the definition of a fixed sized function
#define FDEFEND_() 	}
///Sum 2 numbers
FDEFBEG_(sum2,2)
*args[0]+=*args[1];
FDEFEND_()
///Negate 1 number
FDEFBEG_(neg,1)
*args[0]*=-1;
FDEFEND_()
///Compute the factorial of an argument
FDEFBEG_(fact,1)
BigNum::Op::ofact<T>(args[0],NULL);
FDEFEND_()
///Output help text
FDEFBEG_(help,0)
cout<<"Welcome to my calculator!";
FDEFEND_()
#undef FDEFBEG_
#undef FDEFEND

//VARIADIC FUNCTIONS
/**Macro to start the definition of a variadic function function
\note Functions have been programmed in just for fun. It's not really part of the assignment.
\param NAME The name of the defined function. Must be unique within the Fn namespace.
*/
#define VDEFBEG(NAME) template<typename T> void NAME(int n, T* args []) {
///Macro to end the definition of a fixed sized function
#define VDEFEND() }
///Return the maximum of its arguments
VDEFBEG(max)
T mx=*args[0];
for(int i=1; i<n; i++)
     if(*args[i]>mx) mx=*args[i];
*args[0]=mx;
VDEFEND()
///Return the minimum of its arguments
VDEFBEG(min)
T mi=*args[0];
for(int i=1; i<n; i++)
     if(*args[i]<mi) mi=*args[i];
*args[0]=mi;
VDEFEND()
///Sum the arguments
VDEFBEG(sum)
T sm=*args[0];
for(int i=1; i<n; i++)
     sm+=*args[i];
*args[0]=sm;
VDEFEND()
///Return the average of the arguments
VDEFBEG(avg)
sum(n,args);
*args[0]/=n;
VDEFEND()
#undef VDEFBEG
#undef VDEFEND

} //namespace Fn

} //namespace BigN
#endif /* CALCULATOR_FNS_AND_OPS_HPP_ */
