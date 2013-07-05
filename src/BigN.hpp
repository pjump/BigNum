#ifndef BIGN_HPP_
#define BIGN_HPP_



#include <iostream>
#include <string>
#include <iomanip>
#include <cstring>
#include <limits>
#include <vector>
#include <cmath>
#include <stdexcept>


#include "bign_supplemental.hpp"

namespace BigNum {
using namespace std;

/**\brief An Arbitrary Precision Number
 \tparam type of elementary storage units (cells) 
 \sa \ref about
  */
template<typename S=STYPE> 
class BigN {
     static_assert( numeric_limits<S>::is_integer && !(numeric_limits<S>::is_signed), "Must use an unsigned integer storage type." );

public:
     ///Some class-wide invariants
     enum {
          cshift10d = IntLog<10,(Pow<2,sizeof(S)*8/2>::n)-1>::n , 	///<The capacity of a cell in decimal digits (only the lower half is considered)
          cshift=Pow<10,cshift10d>::n 					///<A power of ten multiplying by which is tantamount to shifting by a whole cell to the left
     };
     enum esign { PLUS=0, MINUS=1 };
     typedef typename HalfType<S>::type HalfST;
private:
     ///Vector of cells representing the number
     vector<S> v_;
     ///Index of the first nonzero cell
     long beg_; //index of the first nonzero cell
     ///Number of digits behind the decimal point
     long scale_;
     ///Stores the sign of the BigN
     esign sign_;

     ///\brief A reference to a cell.
     /**Can be used to access (for either reading or writing) individual decimal digits of a cell as if the cell was an array of those digits
     Only the lower digits of a cell are indexed.
     \note E.g.: CellR(&v_[beg_])[1] is the second decimal digit of a cell
     */
     class CellR {
     private:

          S* pcell_;
          S& operator=(const CellR&) {
               return *pcell_;
          };
     public:
          class Digit;
          ///Store the pcell pointer to a storage cell
          CellR(S* pcell) : pcell_(pcell) {}

          ///Return a Digit reference
          Digit operator[](int coord) const {
               return Digit(pcell_,coord);
          }
          ///Return a class that allows to reference for writing a particular decimal digit in a cell
          class Digit {
               S* pcell_;
               short coord_;
               typedef typename HalfType<S>::type htype;
          public:
               Digit(S* pcell, short coord) : pcell_(pcell),coord_(coord) {}
               ///Write to the digit, effectively changing the cell as if that particular decimal digit in its decimal representation was changed.
               Digit& operator=(short n) {
                    S coeff=pten[numeric_limits<htype>::digits10-1-coord_];
                    *pcell_+=coeff*(n-short(*this));
                    return *this;
               }
               operator short() const {
                    return ffront_i_dec<htype>(*pcell_,coord_);
               }
          };

     };
     istream& _read_in_from_a_stream(istream& is);
	 template<typename T>
	 void _read_in_from_a_number(T x);
	 void _read_in_from_a_cstring(const char* istr) {
		   istringstream iss;
		   iss.rdbuf()->pubsetbuf(const_cast<char*>(istr), strlen(istr));
		   _read_in_from_a_stream(iss);
	 }

     ///Convert a character to a number
     unsigned _D(char c) {
          return c-'0';
     }
     /**\name Padding and Alignment Methods
     @{*/

     ///Return how many useful decimal digits an integer has (don't count leading zeros)
     template<typename T>
     static long _c_rel_digs(T x) {
          long n;
          for(n=0; x>=1; x/=10) ++n;
          return n;
     }
     ///Relevant digits: final digits in the integral part count; final digits in the fractional part don't
      size_t _v_rel_digs () const {
           return v_.size()-beg_-_c_beg_padding(beg_)-_v_end_zs();
      }

     long _cells_since_beg() const {
          return v_.size()-beg_;
     }

     ///Return the number of irrelevant zeros at the end; zeros before the decimal point are relevant
     size_t _v_end_zs() const {
          if(scale_<=0)
               return 0;
          size_t pad=0;
          long scale=scale_;

          //BigNs should be trimmed at the end, so hopefully this one won't run a lot
          long i=v_.size()-1;
          for( ; i>=beg_ && v_[i]==0 && scale>=0; --i)
               scale--;
          pad=(v_.size()-1-i)*cshift10d;

          //irrelevant final zeros inside the last nonzero cell
          int j=0;
          if(scale) {
               HalfST last=v_[i];
               for(; last%10==0 && (scale-j)>=0; j++)
                    last/=10;
               if(last%10==0)
                    j--;
          }
          return pad+j;

     }

     ///make sure that beg_ points to the first nonzero cell but don't shift the array; starts at last beg_
          size_t _adjust_beg() {
               for(; beg_< (long)v_.size(); ++beg_) {
                    if(v_[beg_]) return beg_;
               }
               return beg_=v_.size()-1;
          }
          ///Leading zeros in the cell at cindex
          short _c_beg_padding(long cindex) const {
               return cshift10d-_c_rel_digs(v_[cindex]);
          }
          ///Total padding
          size_t _v_padding() const {
               return _c_beg_padding(beg_)+beg_*cshift10d;
          }
          ///Trim before beg_, shifting the vector to the left
          void _hard_trim_b4_beg_() {
               auto it=v_.erase(v_.begin(),v_.begin()+beg_);
               beg_=it-v_.begin();
          }

          ///Trim Left of digit position pos, either really doing a hard trim (if lazy=false), or just adjusting beg
          void _trimL(size_t pos, bool lazy=true) {
               auto indicies=get_indicies(pos);
               for(short i=0; i<indicies.second; ++i)
                    (*this)[indicies.first*cshift10d+i]=0;
               beg_=indicies.first-1;
               _adjust_beg();
               if(!lazy)
                    _hard_trim_b4_beg_();
          }
          ///Adjust beg_ and hard trim left of beg_
          void _hard_trimL() {
               _adjust_beg();
               _hard_trim_b4_beg_();
          }

          ///Get the index of the last nonzero cell
          long _last_nonzero_cell_i() {
               long i=v_.size()-1;
               for(; i>=beg_ && v_[i]==0; i--)
                    ;;
               return i;
          }

          ///Trim Right of digit position pos; make sure there are no zero cells at the end; may lead to negative scales
          void _hard_trimR(size_t pos) {
               auto indicies=get_indicies(pos);
               if(indicies.first>=v_.size())
                    return;
               for(short i=indicies.second+1; i<cshift10d; ++i)
                    (*this)[indicies.first*cshift10d+i]=0;
               size_t start=indicies.first;
               if(v_[start]!=0)
                    start++;
               size_t ncells=v_.size()-start;
               scale_-=ncells*cshift10d;
               v_.erase(v_.begin()+start,v_.end());

          }
          ///Finds the last nonzero cell and trims at the right of it
          void _hard_trimR() {
               _hard_trimR((_last_nonzero_cell_i()+1)*cshift10d);
          }
          ///Trim Left and Right — combines a soft left trim with a hard right trim
          void _trim() {
               _adjust_beg();
               _hard_trimR();
          }
          ///Does a hard right trim at the right as well
          void _hard_trim() {
               _hard_trimR();
               _adjust_beg();
               _hard_trim_b4_beg_();

          }
          ///@}

          void _propagate_carry(size_t pos, bool pos_is_cell_ix=false);
          ostream& _print(ostream& os) const;

          int _abs_compare(const BigN& Y) const;
          static BigN _add(const BigN& X, const BigN& Y);
          static BigN _subtract(const BigN& X, const BigN& Y);
          static BigN _lmultiply(const BigN&x, const BigN& y);


public:
     void printinfo(ostream& os=cerr) const;
     void hard_trim() { _hard_trim(); }

     ///Get the coordinates of a digit ( [cell_coord,digit_coord] )
     static inline pair<size_t,short> get_indicies(size_t pos)  {
          size_t first=pos/cshift10d;
          short second=(pos<cshift10d) ? pos : pos%cshift10d;
          return {first,second};
     }
     ///Index the BigN as if it were an array of decimal digits (0–9) -- writable version
     typename CellR::Digit operator[](unsigned pos) {
          auto indicies=get_indicies(pos);
          return CellR(&v_[indicies.first])[indicies.second];
     };
     ///Index the BigN as if it were an array of decimal digits (0–9) -- const version
     short operator[](unsigned pos) const {
          auto indicies=get_indicies(pos);
          return CellR(const_cast<S*>(&v_[indicies.first]))[indicies.second];
     };

     ///Rounds to n significant digits, used by the templated constructor for reading from floats
     void round2_n_sdigits(size_t n) {
          n--;
          size_t pad=_v_padding();
          size_t nth=beg_+pad+n;
          if((nth+1)>=v_.size())
			return;
		
          if((*this)[nth+1]>=5) {
               (*this)[nth]=(*this)[nth]+1;
               _propagate_carry(pad+n);
          }
          _hard_trimR(pad+n);
     }

     ///Output: emulates std
     friend ostream& operator<<(ostream& os, const BigN<S>& n) {
          return n._print(os);
     }
     ///Input: emulates std
     friend istream& operator>>(istream& is, BigN<S>& n) {
          n._read_in_from_a_stream(is);
          return is;
     }


     ///Read from a stream (read!=BigN since support for delegated constructors does not exist in G++ 4.6
     BigN(istream& is) {
          _read_in_from_a_stream(is);
     }
     ///Read from an input string; makes it into an istringstream buffer and then calls read()
     BigN(const char* istr) {
          _read_in_from_a_cstring(istr);
     }
     ///Construct from native and floating point types; Floating point types will only be read up to their maximum decimal precision and then rounded
     template<typename T=int>
     BigN(T x=0) {
          _read_in_from_a_number<T>(x);
     }
    ///Read in from a boolean (true==1, false==0)
     BigN (bool b) {
          _read_in_from_a_number(int(b));
     }
     
     ///Returns a value >=1 if *this>Y, 0 if *this==Y, and a value <=-1 if *this<Y
     int compare(const BigN& Y) const {
          //If signs differ, we already know which is one greater than the other
          if(sign_!=Y.sign_)
               return sign_==MINUS ? -1 : 1;

          int coeff= sign_==PLUS ? 1 : -1;

          return (_abs_compare(Y)*coeff);
     }
/** \name Self-Descriptive Operators
 * @{ */
     /*ASSIGNMENT OPERATORS*/
     ///Default assignment from BigN
     template<typename T>
     BigN& operator=(T x) {
          _read_in_from_a_number(x);
          return *this;
     }
     BigN& operator=(bool b) {
          _read_in_from_a_number(int(b));
          return *this;
     }
     BigN& operator=(const char* istr) {
          _read_in_from_a_cstring(istr);
          return *this;
     }

     //Default copy constructor
     //Default move constructor
     
     operator bool() const {
          for(size_t i=beg_; i<v_.size(); i++)
               if(v_[i]) return true;
          return false;
     }
     //TODO This is a hack. Works for argument counting, but should be redone ASAP.
     /*operator int() const {
               return v_[v_.size()-1]/pten[scale_];
      }*/
     BigN& operator-() {
          sign_= sign_==PLUS ? MINUS : PLUS;
          return *this;
     }
     BigN& operator+() {
          return *this;
     }

     BigN& operator=(BigN&& y) {
          v_=(move(y.v_));
          scale_=y.scale_;
          beg_=y.beg_;
          sign_=y.sign_;
          return *this;
     }
     BigN& operator=(const BigN& y) {
          if(this==&y) return *this;
          v_=((y.v_));
          scale_=y.scale_;
          beg_=y.beg_;
          sign_=y.sign_;
          return *this;
     }
     class CellIterator;


     //Add the absolute value of Y to the absolute value of (*this) without changing sign_

     //Ops combined with assignment
     BigN& operator+=(const BigN& Y) {
          if(sign_!=Y.sign_)
               return Y.sign_==MINUS ?  (*this=_subtract(*this,Y)) : (*this=_subtract(Y,*this));
          return (*this=_add(*this,Y));
     }
     BigN& operator-=(const BigN& Y) {
          if(sign_!=Y.sign_)
				return *this=_add(*this,Y);
          bool flipsign = sign_==MINUS ? true : false;
          *this=_subtract(*this,Y);
          if (flipsign)
			sign_ = sign_==PLUS ? MINUS : PLUS;
		  return *this;
     }
     
     BigN& operator--() {
          (*this)-=1;
          return *this;
     }
     BigN& operator++() {
          (*this)+=1;
          return *this;
     }

     friend bool operator==(const BigN& x,const BigN& y) {
          return x.compare(y)==0;
     }
     friend bool operator<(const BigN& x, const BigN& y) {
          return x.compare(y)<0;
     }
     friend bool operator>(const BigN& x, const BigN& y) {
          return x.compare(y)>0;
     }
     BigN& operator*=(const BigN& y) {
          return *this=_lmultiply(*this,y);
     };
     BigN& operator/=(const BigN& y) {
          return *this;
     }
///A non-assigning binary operator identified by *X*
#define NONASS_BINOP_(X) inline BigN operator X (const BigN& y) const 	{ 	BigN mx=(*this); mx X##= y; return mx;  	}
///A non-assigning binary operator identified by *X*; auto-converts the *y* parameter to BigN
#define NONASS_BINOP2_(X) template<typename T> inline BigN operator X (T y) const	{ 	BigN mx=(*this); mx X##= BigN(y); return mx;  	}
     NONASS_BINOP_(+)
     NONASS_BINOP2_(+)
     NONASS_BINOP_(-)
     NONASS_BINOP2_(-)
     NONASS_BINOP_(*)
     NONASS_BINOP2_(*)
     NONASS_BINOP_(/)
     NONASS_BINOP2_(/)
#undef NONASS_BINOP_r
     //Ops with temporaries; made faster by means of move semantics
///A binary operator *X* applied to an rvalue x and a const y
#define MOVE_OP_(X) inline friend BigN&& operator X (BigN&& x,const BigN& y) { x X##= y; return x; }
     MOVE_OP_(+)
     MOVE_OP_(-)
     MOVE_OP_(*)
     MOVE_OP_(/);
#undef MOVE_OP_
///A commutative binary operator *X* applied to a const x and an rvalue y
#define MOVE_OP2_(X) inline friend BigN&& operator X (const BigN& x,BigN&& y) { y X##= x; return y; }
     MOVE_OP2_(+)
     MOVE_OP2_(*)
///If y is n rvalue and x isn't, turn it around and flip the sign
inline friend BigN&& operator - (const BigN& x,BigN&& y) { y -= x; return -y; }
    
     friend string to_string(const BigN& x) {
          stringstream ss;
          ss<<x;
          return ss.str();
     }
   ///@}


};


}

#include "bign_.hpp"


#endif /*BIGN_HPP_ */
