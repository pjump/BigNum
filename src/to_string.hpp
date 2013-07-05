#ifndef TO_STRING_HPP_
#define TO_STRING_HPP_

/**
 * \file
 * \brief Custom implementation of the to_string() functions since *g++ 4.5* fails to provide them.
 */
#include <string>
namespace BigNum
{
using namespace std;

///\brief This should be greater than or equal to the maximum length a native numerical type's string representation can be.
#define MAX_OUT_LEN 200
/** \brief Macro to write the definitions of the to_string() overloads
* @param TYPE the type of the overload
* @param FMT the identifier of the type used in *C* I/O functions
* 	Without the % sign.
*/ 		
#define MAKE_TO_STRING(TYPE,FMT) inline string to_string( TYPE value ){ char buf[MAX_OUT_LEN];\
	std::sprintf(buf, "%"#FMT, value); return string(buf); }
MAKE_TO_STRING(int,d)
MAKE_TO_STRING(long,ld)
MAKE_TO_STRING(long long,lld)
MAKE_TO_STRING(unsigned,u)
MAKE_TO_STRING(unsigned long,lu)
MAKE_TO_STRING(unsigned long long,llu)
MAKE_TO_STRING(float,f)
MAKE_TO_STRING(double,f)
MAKE_TO_STRING(long double,Lf)
#undef MAKE_TO_STRING
#undef MAX_OUT_LEN
}
#endif /* TO_STRING_HPP_ */
