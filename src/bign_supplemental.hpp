//#include "BigN.hpp"

#ifndef BIGN_SUPPLEMENTAL_HPP_
#define BIGN_SUPPLEMENTAL_HPP_
#include <climits>
#include <cmath>

/**
 * \file
 * \brief Some Simple Typedefs and Functions.
 * The functions are mostly for the BigN.hpp header library, but too general to be included in it.
 */

namespace BigNum {
using namespace std;

//An extern declaration of the global pten array, which contains lower powers of ten
extern const uintmax_t pten[];

///\name Some Convenient Fixed-length Integer Typedefs
///@{
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
///@}
}

namespace std {
/** \brief Macro to write floor overloads for integer types.
 *  In effect, these will be no-ops, but are required for generality in the BigN template class, particularly in its templated constructor that constructs BigN's from native numerical types (integers or floats).
This can't be in the BigNum namespace, or else these templates would override the std::floor() function even where it already is defined. Consequently, it is in the std namespace.'*/
#define INTFLOOR_(I) inline BigNum::I floor(BigNum::I x) {return x;}
///@name Floor() Overloads for Integral Types
///@{
INTFLOOR_(u8) INTFLOOR_(u16) INTFLOOR_(u32) INTFLOOR_(u64)
INTFLOOR_(i8) INTFLOOR_(i16) INTFLOOR_(i32) INTFLOOR_(i64)
///@}
#undef INTFLOOR_
}

namespace BigNum {

///\brief A stack type that effectively copies the behavior of *std::stack<T,std::vector>*, but whose internal vector is accessible.
template<typename T>
struct Stack {
     vector<T> v_;
     typename vector<T>::size_type size() const {
          return v_.size();
     }
     bool empty() const {
          return size()==0;
     }
     T& top() {
          return v_[size()-1];
     }
     void push(const T& val) {
          v_.push_back(val);
     }
     void push(T&& val) {
          v_.push_back(move(val));
     }
     void pop () {
          v_.erase(v_.end()-1);
     }
};

///\brief A struct to get a typedef for the type that takes half the size of the argument.
///\tparam T The type to get half-sized type from.
template<typename T>
struct HalfType;
/**@name htype HalfType Specializations
 * @{ */
template<> struct HalfType<u64> {
     typedef u32 type;
};
template<> struct HalfType<u32> {
     typedef u16 type;
};
template<> struct HalfType<u16> {
     typedef u8 type;
};
template<> struct HalfType<u8> {
     typedef u8 type;
};
///@}

///\brief Used by IntLog.
template< unsigned short b, unsigned long N > struct IntLog_ {
     enum { n = 1 + IntLog_< b, N / b > :: n } ;
} ;
///\brief Used by IntLog.
template<unsigned short b> struct IntLog_< b, 0 >  {
     enum { n = 0 } ;
} ;
///\brief Count decimal digits in integer N at compile time.
template< unsigned short b, unsigned long N > struct IntLog {
     enum { n=IntLog_<b,N>::n - 1 };
};

///\brief Raises "b" to the power of "e" at compile time.
template<long long b, char e> struct Pow {
     enum { n = b * Pow< b, e-1 >::n } ;
};
template<long long  b> struct Pow<b, 0>  {
     enum { n = 1 } ;
} ;

///\brief Computes modulos for floating point types. See integer type specializations of this template below.
template<typename T>
inline T mod (T a, T b)
{
     T n=floor(a/b);
     return a-b*n;
}
///Macro to write integer specializations of the mod template function
#define INTMOD_(TYPE) template<> inline TYPE mod<TYPE>(TYPE a, TYPE b) { return a%b; }
///@name Integer specializations of the mod template functions.
///@{
INTMOD_(i8) INTMOD_(i16) INTMOD_(i32) INTMOD_(i64)
INTMOD_(u8) INTMOD_(u16) INTMOD_(u32) INTMOD_(u64)
///@}
#undef INTMOD_

///\brief Computes the number of decimal zeros at the end of x.
///\tparam T The type of the x parameter.
///\param x The number whose ending decimal zeros should be counted.
template<typename T>
T end_zeroes (T x)
{
     T r=0;
     for(; x>0 && x%10==0; x/=10)
          r++;
     return r;
}

/** \brief Indexes a decimal digit inside from the end  of integer n of type T. <br><br>

E.g., back_i_dec(1234,0)==4; back_i_dec(1234,1)==3*
\param n Number to index.
\param index The index at which to get the digit. The final digit in the decimal representation of the number has an index of 0.
\return A decimal digit [0-9]. 
*/

template <typename T>
inline short back_i_dec(T n, int index)
{
     while(n && index--)
          n/=10;
     return n%10;
}

/** \brief __Fixed Front Index__—indexes a decimal digit inside from the front of integer n of type T, taking its size into account. <br><br>
	*E.g., ffront_i_dec(1234,0)==0; ffront_i_dec(1234,numeric_limits<int>::digits10)==4*

\param n Number to index.
\param index The index at which to get the digit. The first digit in the decimal representation of the number has an index of 0.
\return A decimal digit [0-9].
*/
template <typename T>
inline short ffront_i_dec(T n, int index)
{
     return back_i_dec(n,std::numeric_limits<T>::digits10 -1 - index);
}

}//namespace BigNum
#endif /* BIGN_SUPPLEMENTAL_HPP_ */
