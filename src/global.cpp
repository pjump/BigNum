#include <cstdint>

/** \file
 * \brief Storage space for globals. 
 * 
 * BigNum has only one global variable, the pten array.*/

namespace BigNum {
/**
__Some lower powers of ten pre-stored for fast runtime lookup.__
Some of the project's routines refer to this array as to an extern, global variable.
 */
//ruby -e  'a="1"; 20.times { print a,"LU, "; a+="0" }'`
//the vals at the end of the array will have overflown on x86, but they're only going to be needed on x86_64
extern const uintmax_t pten[20]= {
     1LU, 10LU, 100LU, 1000LU, 10000LU, 100000LU, 1000000LU, 10000000LU, 100000000LU, 1000000000LU, 10000000000LU, 100000000000LU, 1000000000000LU, 10000000000000LU, 100000000000000LU, 1000000000000000LU, 10000000000000000LU, 100000000000000000LU, 1000000000000000000LU, 10000000000000000000LU
};
}

