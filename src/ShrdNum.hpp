#ifndef SHRDNUM_HPP_
#define SHRDNUM_HPP_

#include <string>
//#include <valarray>
#include <memory>

/** \file ShrdNum.hpp
 \brief This file contains ShrdNum templated class and some related functions. */

namespace BigNum {
using namespace std;

/** \brief A thin wrapper around numeric type *T*.
Makes copy-constructed copies share the same underlying resource.
This especially affects function calls—all functions down in the call stack will have access to the originally passed argument,
regardless of whether ShrdNums were further down passed by value or by const references (!).

This is speeds up copying of large types, such as BigN or std::vallaray, and is useful, since the Calculator class copies values around a lot. If the Calculator class had used references itself, it would have been a bit slower for small, native template arguments	, and Calculator would have been a bit more prone to memory leakage.

__Example__
\code
typedef ShrdNum<int> Shared
	Shared a=0;
	const Shared& ar=a;
	Shared b(ar);
	b=1;
	cout<<a	<<endl;
\endcode
> Output: 1
*/

template<typename T> class ShrdNum {
     shared_ptr<T> p_;
public:

     ///The default constructor; initializes ShrdNum with T(0)
     ShrdNum() : p_(new T(0)) {}
     /**!!!COPY CONSTRUCTOR—all copy-constructed copies (this especially affects function arguments,
     	regardless of whether or not they may be passed by value or by const references) will be effectively REFERENCES with write access to the original resource.
     */
     ShrdNum(const ShrdNum& x) : p_(x.p_) {}
     template<typename U> 	ShrdNum(const U& val) : p_(new T(val)) {} 		///<Copy-constructs from whatever T can be copy-constructed from. A move version is not available as *val* types will always be small. 
     ShrdNum(ShrdNum&& x) : p_(std::move(x.p_)) {}					///<Move constructor.


     ///Assign value to the resource managed by ShrdNum from the resource managed by x.
     ShrdNum& operator=(const ShrdNum& x) {
          if(this!=&x) {
               *p_=*x.p_;
          };
          return *this;
     }
     /// \brief Assign from whatever T could be assigned from.
     template<typename U>
     ShrdNum& operator=(const U& val) {
          *p_=val;
          return *this;
     }
     ///Move-assign from T.
     ShrdNum& operator=(T&& val) {
          *p_=move(val);
          return *this;
     }
     ///Get T's string representation.
     string to_str() const {
          return to_string(*p_);
     }

     ///Share the same resource with another ShrdNum, effectively relinking the reference that a ShrdNum actually represents
     ShrdNum& assign(const ShrdNum& x) {
          if(this==&x) return *this;
          p_=x.p_;
          return *this;
     }
     ///Detach by assigning a value-derived copy (i.e. new resource) of self
     ShrdNum& detach() {
          return assign(ShrdNum(*p_));
     }
     ///Expose shared_ptr::use_count()
     size_t use_count() const {
          return p_.use_count();
     }
     friend ostream& operator<<(ostream& os, const ShrdNum& x) {
          return os<<*x.p_;
     }
     friend istream& operator>>(istream& is, 		 ShrdNum& x) {
          return is>>*x.p_;
     }

     /** Very simple wrappers to the operators and conversions defined for the type specified as the template parameter
      * They're mostly written by macros and they make use of of move semantics to speed things up where possible*/

///\name Common operators and conversions

///@{

     ///Macro to write the definitions of the =[+-%/ *] operators
#define BIN_A_(X) friend ShrdNum& operator X##= (ShrdNum& x, const ShrdNum& y) { (*x.p_) X##= *y.p_; return x; }
     BIN_A_(+);
     BIN_A_(-);
     BIN_A_(%);
     BIN_A_(/);
     BIN_A_(*);
#undef BIN_A_
///Macro to write the definitions of operator < > >= <= == !=
#define BIN_C_(X) friend bool operator X (const ShrdNum& x, const ShrdNum& y) { return (*x.p_ X *y.p_);  }
     BIN_C_(<);
     BIN_C_(>);
     BIN_C_(>=);
     BIN_C_(<=);
     BIN_C_(==);
     BIN_C_(!=);
#undef BIN_C_
///Macro to write the definitions of [+-%/ *] operators  when BOTH sides are LVALUES
#define BIN_(X) friend ShrdNum operator X (const ShrdNum& x, const ShrdNum& y) { return (*x.p_ X *y.p_);  }
     BIN_(+);
     BIN_(-);
     BIN_(%);
     BIN_(/);
     BIN_(*);
#undef BIN_
///Macro to write the definitions of [+-%/ *] operators when the LEFT operand is an RVALUE. Will use move semantics to avoid unnecessary copies.
#define RBIN_(X) friend ShrdNum&& operator X (ShrdNum&& x, const ShrdNum& y) {  x X##= y; return std::move(x);  }
     RBIN_(+);
     RBIN_(-);
     RBIN_(%);
     RBIN_(/);
     RBIN_(*);
#undef RBIN_

///Macro to write the definitions of [+*] operators (the commutative ones)  when the RIGHT operand is an RVALUE
#define OP_(X) friend ShrdNum&& operator X (const ShrdNum& x, ShrdNum&& y) {  y X##= x; return std::move(y);  }
     OP_(+);
     OP_(*);
#undef OP_
///Binary minus if the right side is an RVALUE
     friend ShrdNum&& operator- (const ShrdNum& x, ShrdNum&& y) {
          -std::move(y) += x;
          return std::move(y);
     }

///unary minus when the LEFT side is an LVALUE
     friend ShrdNum operator-(const ShrdNum& x) {
          return ShrdNum(-(*x.p_) );
     }
///unary minus when the LEFT side is an RVALUE
     friend ShrdNum&& operator-(ShrdNum&& x) {
          *x.p_ = -(*x.p_);
          return std::move(x);
     }
///unary plus
     ShrdNum& operator+() {
          return *this;
     }

     ShrdNum& operator++() {
          ++(*p_);
          return *this;
     }
     ShrdNum& operator--() {
          --(*p_);
          return *this;
     }

     operator bool() const {
          return bool(*p_);
     }
     operator int() const {
          return int(*p_);
     }
     operator double() const {
          return int(*p_);
     }

///@}
};

/** \name External Templated Resource Management Functions and Their ShrdNum Specialization
These functions are no-ops for anything other than ShrdNum's. In ShrdNum's, they call their respective in-class public method counterparts.
*/
/// @{
template<typename T, typename U> T& assign(T& op,const U& val)
{
     op=val;
     return op;
}
template<typename T,typename U> ShrdNum<T>& assign(ShrdNum<T>& op,const U& val)
{
     op.assign(val);
     return op;
}

template<typename T> T& detach(T& op)
{
     /* no-op unless explicitly specified*/ return op;
}
template<typename T> ShrdNum<T>& detach(ShrdNum<T>& op)
{
     return op.detach();
}
template<typename T> long int use_count(T& op)
{
     return -1;
}
template<typename T> long int use_count(ShrdNum<T>& op)
{
     return op.use_count();
}
template<typename T> string to_string(const ShrdNum<T>& op)
{
     return op.to_str();
}
/// @}

}
#endif /* SHAREDNUM_HPP_ */
