#ifndef BIGN__HPP_
#define BIGN__HPP_
//#define DEBUG
#define FILE__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define PVAR(X) cout<<FILE__<<":"<<__LINE__<<"\t"#X"="<<X<<endl
#ifdef DEBUG
#define DB(X) cerr<<FILE__<<":"<<__LINE__<<"\t"<<X<<endl
#define DB_VAR(X) cerr<<FILE__<<":"<<__LINE__<<"\t"<<#X"="<<X<<endl
#else
#define DB_VAR(X)
#define DB(X)
#endif

#include "BigN.hpp"
#include "bign_supplemental.hpp"


namespace BigNum {
using namespace std;

/** \brief A class that allows one-directional browsing through a virtual, properly aligned BigN.
 * The class circumvents the need to physically shift and pad BigN objects before an operations such as comparison, subtraction, or addition (i.e., an operation dependent on
 * proper alignment) can be performed. */

template<typename S>
class BigN<S>::CellIterator {
private:
     const BigN* p_; //ain't gonna change the data, just browse through it
     size_t beg_padding_cells_;  ///< number of virtual padding cells at the beginning
     size_t end_padding_cells_;  ///< number of virtual padding cells at the end
     unsigned short shift10d_;	 ///< shift to the right in decimal digits
     HalfST shift_;		 ///< 10^shift10d_
     HalfST mask_;		 ///< BigN::cshift/shift_
     bool fwd_;			 ///< goes forward?
     long int ix_;		 ///< current index
     HalfST val_;		 ///< current value
     HalfST carry_;		 ///< current carry

public:
     ///Initialize the iterator
     CellIterator(const BigN* p,
                  bool fwd,
                  size_t beg_padding_cells=0,
                  size_t end_padding_cells=0,
                  short shift10d=0)
          : p_(p),
            beg_padding_cells_(beg_padding_cells),
            end_padding_cells_(end_padding_cells),
            shift10d_(shift10d),
            shift_(pten[shift10d]),
            mask_(cshift/shift_),
            fwd_(fwd), val_(0),carry_(0) {
          ix_ = fwd_ ? p_->beg_ : p_->v_.size()-1;
     };
     size_t rescale(long int scale) {
          if(scale > p_->scale_) {
               end_padding_cells_=(scale-p_->scale_)/cshift10d;
               if((scale-p_->scale_)%cshift10d)
                    end_padding_cells_++;
               shift10d_=p_->scale_+end_padding_cells_*cshift10d - scale;
               DB("Rescaling a pseudo-BigN by "<<end_padding_cells_<<" cells; shift10d_="<<shift10d_);
               

               //If it's a backward iterator, then if shifting overflows to the first end_padding cell, then there's one less end_padding cell 
               short endzs=(end_zeroes(p_->v_[p_->v_.size()-1])) ;
               //The whole last cell is zeros
               endzs = endzs ? endzs : cshift10d;
			  
               if(shift10d_ && !fwd_) {
                    end_padding_cells_--;
               }



               return end_padding_cells_;
          }
          return 0;
     }
     ///\return number of relevant cells, i.e. cells since beg_ + padding cells at both the end and the beginning
     long int rel_cells() const {
          return p_->v_.size()-p_->beg_+end_padding_cells_+beg_padding_cells_;
     }
    ///Setter for setting the beg_padding_cells_ value after initialization
     void pad(size_t beg_padding_cells) {
          DB("Padding a pseudo-BigN by "<<beg_padding_cells<<" cells");
          beg_padding_cells_=beg_padding_cells;
     }
     ///Make CellIterator ready for being browsed. Specify that no additional adjustment will be made from now on.
     void ready() {
          shift_=pten[shift10d_];
          mask_=pten[cshift10d-shift10d_];

          //If it's a forward iterator and shift10d == intdigits10d(v_[p->beg_])
          if(fwd_ && ix_==p_->beg_ && _c_rel_digs(p_->v_[p_->beg_])==shift10d_)
               this->operator ()();
     }
     ///Return value and move on to the next value.
     HalfST operator() () {
          if(fwd_) return _getfwd();
          return _getreverse();
     }
private:
     HalfST _getfwd() {
          if (beg_padding_cells_)
               beg_padding_cells_--;
          else {
               if (ix_ != (long) p_->v_.size()) {
                    S cell = p_->v_[ix_];
                    val_ = cell / shift_ + mask_*carry_;
                    carry_ = p_->v_[ix_] % shift_;
                    ix_++;
               } else {
                    if (carry_)	{
                         val_ = carry_;
                         carry_ = 0;
                         return val_;
                    }
                    return 0;
               }
          }
          return val_;
     }
     HalfST _getreverse() {
          if(end_padding_cells_>0) {
               DB_VAR(end_padding_cells_);
               end_padding_cells_--;
          } else {
               if(ix_>=p_->beg_) {
                    if(shift10d_) {
                         val_=(p_->v_[ix_])%shift_*mask_+carry_;
                         //carry this shifted value
                         carry_=p_->v_[ix_]/shift_;
                    } else {
                         val_=p_->v_[ix_];
                    }
                    ix_--;
               } else {
                    if(carry_) {
                         val_=carry_;
                         carry_=0;
                         return val_;
                    }
                    return 0;
               }

          }
          return val_;
     }


};
///A u8 specialization
template<>
class BigN<u8>::CellIterator {
private:
     typedef u8 S;
     const BigN* p_; ///>won't' change the data, just browse through it
     size_t beg_padding_cells_;
     size_t end_padding_cells_;
     bool fwd_;
     long int ix_;

public:
     CellIterator(const BigN* p,
                  bool fwd,
                  size_t beg_padding_cells=0,
                  size_t end_padding_cells=0,
                  short shift10d=0)
          : p_(p),
            beg_padding_cells_(beg_padding_cells),
            end_padding_cells_(end_padding_cells),
            fwd_(fwd) {
          ix_ = fwd_ ? p_->beg_ : p_->v_.size()-1;
     };
     size_t rescale(long int scale) {
          if(scale > p_->scale_) {
               end_padding_cells_=(scale-p_->scale_);
               return end_padding_cells_;
          }
          return 0;

     }

     void pad(size_t beg_padding_cells) {
          DB("Padding a pseudo-BigN by "<<beg_padding_cells<<" cells");
          beg_padding_cells_=beg_padding_cells;
     }
     long int rel_cells() const {
          return p_->v_.size()+end_padding_cells_+beg_padding_cells_-p_->beg_;
     }
     void ready() {
     }
     HalfST operator() () {
          if(fwd_) return _getfwd();
          return _getreverse();
     }
private:
     HalfST _getfwd() {
          if (beg_padding_cells_) {
               beg_padding_cells_--;
               return 0;
          }
          if (ix_ != (long) p_->v_.size()) {
               S val= p_->v_[ix_++];
               return val;
          }
          return 0;
     }
     HalfST _getreverse() {
          if(end_padding_cells_) {
               DB_VAR(end_padding_cells_);
               end_padding_cells_--;
               return 0;
          }
          if(ix_>=p_->beg_) {
               S val= p_->v_[ix_--];
               return val;
          }

          return 0;
     }


};



template<typename S>
/** \brief Templated constructor for reading in BigN's from either floats or integers.

BigN's read in this way are always left-aligned (padding()==0) and in the case of floats, the type's  precision is respected.
\param x Number to read from.
*/
template<typename T>
void BigN<S>::_read_in_from_a_number(T x)
{
     //initialize the BigN
     beg_=0;
     scale_=0;
     v_.resize(0);
     sign_=PLUS;

     if(x==0) {
          v_.push_back(0);
          return;
     };

     if(x<0) {
          x=-x;
          sign_=MINUS;
     }

     /*Make sure we don't read in digits that are within the type's error range.
       This affects floating point types in particular */
     unsigned precision=numeric_limits<T>::digits10+1;


     /*Reserve cells for the integral part of the number and fill them up.
     If the integral part is zero, then only start push'ing_back the fractional part cells after the first nonzero cell is found.
     */
     long int intreserve=0;
     for(T xint_cp=floor(x); xint_cp; ++intreserve)
          xint_cp=floor(xint_cp/cshift);

     v_.resize(intreserve);


     //Align to the left

     if(intreserve) {
          long int i=intreserve-1;

          T intx=floor(x);
          int x10d=_c_rel_digs(intx);
          //How many digits to add to the first cell to make it left-aligned
          int shift10d = (x10d%cshift10d)? cshift10d-(x10d%cshift10d) : 0;
          S shift=pten[shift10d];
          scale_+=shift10d;

          //Start at the end of the integral part, and make sure it's properly shifted to the left
          i = intreserve-1;
          v_[i]=mod<T>(intx,static_cast<T>(cshift/shift))*shift;
          intx/=(cshift/shift);
          //Continue to the left
          for(--i; i>=0 ; --i) {
               v_[i]=mod(intx,static_cast<T>(cshift));
               intx/=cshift;
          }
          //Erase the integral part from x
          x=x-floor(x);
          //If x has a fractional part and there was a shift, then the last digits of v_[v_.size()] should not be zeros, but come from the fractional part
          if(x) {
               x*=shift;
               v_[v_.size()-1]+=floor(x);
               x-=floor(x);
          }
     } else { //If there was no integral part, we need to zero in on the first nonzero cell and ensure proper alignment
          for(; floor(x)==0; scale_+=cshift10d)
               x*=cshift;
          scale_+=cshift10d;

          int x10d=_c_rel_digs(static_cast<S>(x));
          int shift10d = x10d%cshift10d ? cshift10d-(x10d%cshift10d) : 0;
          S shift=pten[shift10d];
          x*=shift;
          scale_+=shift10d;

          S lastcell=static_cast<S>(x);
          v_.push_back(lastcell);
          x-=lastcell;


     }
     //This won't run if there was no fractional part
     for(x*=cshift; x && v_.size()*cshift10d<precision; scale_+=cshift10d) {
          if(v_.size()*cshift10d >= (precision+1))
               break;

          S lastcell=static_cast<S>(x);
          v_.push_back(lastcell);
          x-=lastcell;
          x*=cshift;
     }

     //Round to precision
     round2_n_sdigits(precision);

}
///Function for reading in BigN's from an istream.
template<typename S>
std::istream& BigNum::BigN<S>::_read_in_from_a_stream(std::istream& is) //: len_(0),scale_(0),sign_(PLUS)
{
     beg_=0;
     scale_=0;
     v_.resize(0);
     sign_=PLUS;

     size_t ix=0,len=0;

     bool after_dp=false;
     bool zero;

     noskipws(is);
     char c;
     is>>c;
     if(c=='0');
     zero=true;
     //ignore zeros before the decimal point
     while(c=='0' && is>>c)
          ;;

     switch(c) {
     case '-':
          sign_=MINUS; //fall through
     case '+':
          break;
     case '.':
          after_dp=true;
          break;
     default:
          if(!isdigit(c)) {
               if(zero) {
                    v_.push_back(0);
                    is.putback(c);
                    return is;
               }
               is.setstate(ios::failbit);
          }
          is.putback(c);
          break;
     };


     //before decimal point
     while(!after_dp && is>>c) {
          if(!isdigit(c)) {
               if(c!='.') {
                    is.putback(c);
                    break;
               }
               after_dp=true;
               break;
          }
          if(!(len%cshift10d)) {
               v_.push_back(0);
          }
          ix=len++/cshift10d;
          v_[ix]*=10;
          v_[ix]+=_D(c);
     }

     while(after_dp && is>>c) {
          if(!isdigit(c)) {
               is.putback(c);
               break;
          }
          if(!(len%cshift10d)) v_.push_back(0);
          ix=len++/cshift10d;
          v_[ix]*=10;
          v_[ix]+=_D(c);
          ++scale_;

     }
     ///To behave the same as native number types
     if(len==0) {
          v_.push_back(0);
          is.setstate(ios_base::failbit);
          return is;
     }

     ///read to fill up the final cell
     while((len-1)%cshift10d !=cshift10d-1) {
          v_[ix]*=10;
          ++scale_;
          ++len;
     }

     _adjust_beg();

     return is;

}
///Propagate carries (cell overflow in respect to cshift-1) from position pos to the left
///\arg pos position, either a digit position or a cell position
///\arg pos_is_cell_ix specifies, wheter pos is a cell index, otherwise it's considered to be a digit index
template<typename S>
void BigN<S>::_propagate_carry(size_t pos, bool pos_is_cell_ix)
{
     long marker=pos;

     if(pos_is_cell_ix==false) { //If we're dealing with real position here convert it to a cell index;
          auto indicies = get_indicies(pos);
          marker=indicies.first;
     }

     HalfST carry=v_[marker]/cshift;
     v_[marker]%=cshift;

     for(; carry && marker>=beg_; --marker) {
          v_[marker]+=carry;
          carry=v_[marker]/cshift;
          v_[marker]%=cshift;

     }
     if(carry) {
          if(beg_) {
               beg_--;
               v_[beg_]=carry;
          } else {
               beg_--;
               v_.insert(v_.begin(),carry);
          }
     }
     return;
}
///Print *this to ostream os
template<typename S>
ostream& BigN<S>::_print(ostream& os) const
{
	  ///digits in cells
     long ndigs = _cells_since_beg()*cshift10d;
    if(v_.size()==0 || ((v_.size()-beg_)==1 && v_[beg_]==0 ))
          return os<<'0';
     if(sign_==MINUS)
          os<<'-';
     long addz=0;
     if(scale_>=ndigs) { ///all is behind the decimal point; might need to add additional leading zero cells
          addz=scale_-ndigs;
      
#ifdef ZEROPOINTX_
          os<<'0'; //A zero before the decimal point is not printed, unless compiled with -DZEROPOINTX_
#endif
          long irrel_zs=_v_end_zs();
          long end=(ndigs-irrel_zs)+beg_*cshift10d;
          long i=beg_*cshift10d;
          if(i>=end)
               return os<<'0';
          os<<'.';
          for(long j=0; j<addz; ++j)
               os<<'0';
          for(; i<end; i++)
               os<<(*this)[i];
          return os;
     } else if(scale_<=ndigs) { //there will be a dp inside the number
          //strip leading zeros
           long lzs=_c_beg_padding(beg_);
          //first end at the decimal point
          if(scale_>=0) {
               long end=beg_*cshift10d+ndigs-scale_;
               long i=beg_*cshift10d+lzs;
               for(; i<end; ++i)
                    os<<(*this)[i];
               i=end; //we want to skip leading zeros, but not the ones after the decimal point and before the first significant digit
               end+=scale_-_v_end_zs();
               if(i>=end)
                    return os;
               os<<'.';
               //f
               for(; i<end; ++i)
                    os<<(*this)[i];
          } else {
               long end=beg_*cshift10d+ndigs;
               long i=beg_*cshift10d+lzs;
               for(; i<end; ++i)
                    os<<(*this)[i];
               for(long i=0; i>scale_; --i)
                    os<<'0';
          }

     }
     return os<<flush;

}
template<>
ostream& BigN<u8>::_print(ostream& os) const
{
     bool print_padding=false;
     size_t end_pad=_v_end_zs();
     size_t len=(v_.size()-beg_)*cshift10d;//-beg_padding()-end_pad;
     if(len==0)
          return os<<0;
     long i=beg_*cshift10d;//+beg_padding();
     long end=i+len;
     long scale=scale_;//-end_pad;
     if(!print_padding) {
          len-=_c_beg_padding(beg_)+end_pad;
          i+=_c_beg_padding(beg_);
          scale-=end_pad;
          end=i+len;
     }
     if(scale>=0) {
          end-=scale;
          //!!UNCOMMENT THIS TO PRINT A ZERO BEFORE THE DECIMAL POINT IF THERE IS NOT INTEGRAL PART
          //if(i>=end) 	os<<'0';
          for(; i<end; i++) {
               os.put('0'+v_[i]);
          }
          end+=scale;
          if(i<end)
               os<<'.';
          for(; i<end; i++)
               os.put('0'+v_[i]);
     } else {
          for(; i<end; i++)
               os.put('0'+v_[i]);
          for(i=0; i<-scale; i++)
               os.put('0');
     }
     return os;
}
///Equivalent to BigN::compare, but ignores signs
template<typename S>
int BigN<S>::_abs_compare(const BigN& Y) const {
                   #ifdef DEBUG
                   cerr<<"COMPARING (abs)"<<endl;
                   printinfo(cerr); cerr<<endl;
                   Y.printinfo(cerr); cerr<<endl;                   
				   #endif
                                   
                   CellIterator Xit(this,true);
                   CellIterator Yit(&Y,true);

                   size_t scale=max(scale_,Y.scale_);        

                   Xit.rescale(scale);
                   Yit.rescale(scale);
                    
                   size_t len=max(Xit.rel_cells(), Yit.rel_cells()) +1;
                   DB_VAR(len);
                   DB_VAR(Yit.end_padding_cells_);
                   DB_VAR(Xit.end_padding_cells_);
                   Xit.pad(len-Xit.rel_cells());
	               Yit.pad(len-Yit.rel_cells());
  
                   Yit.ready();
                   Xit.ready();
  
  
                   //INTEGER DIGITS
                   long intdigs1=(v_.size()-beg_)*cshift10d -_c_beg_padding(beg_)-scale_;
                   long intdigs2=(Y.v_.size()-Y.beg_)*cshift10d -Y._c_beg_padding(Y.beg_)-Y.scale_;
                   DB_VAR(intdigs1);
                   DB_VAR(intdigs2);
                   
                   if(intdigs1-intdigs2) return (intdigs1-intdigs2);
                   //Make sure beginnings are aligned
                   
                   int ret;
                   long end =beg_+len;
                   for(long i=beg_; i<end; ++i) {
					   S xcell=Xit();
					   S ycell=Yit();
					   DB_VAR(xcell);
					   DB_VAR(ycell);
                        if((ret=xcell-ycell))
                             return ret;
                   }
                   return 0;
              }
	///Does a virtual alignment of two numbers by means of CellIterators and then adds them together, returning a third BigN/
	///The sign will be that of the first operand
	template<typename S>
           BigN<S> BigN<S>::_add(const BigN<S>& X, const BigN<S>& Y) {
          #ifdef DEBUG
                    cerr<<"ADDING"<<endl;
					cerr<<"\t"<<X<<" + "<<Y<<endl;
                    X.printinfo(cerr);
                    cerr<<endl;
                    Y.printinfo(cerr);
                    cerr<<endl;
          #endif
                    CellIterator Yit(&Y,false);
                    CellIterator Xit(&X,false);

                    size_t scale=max(X.scale_,Y.scale_);        

                    Xit.rescale(scale);
                    Yit.rescale(scale);
                    
                    size_t len=max(Xit.rel_cells(), Yit.rel_cells())+1;

                    
                    BigN r;
                    r.v_.resize(len,0);
                    r.scale_=scale;

                    r.sign_=X.sign_;

                    Yit.ready();
                    Xit.ready();
                    long i=len-1;
                    S carry=0;

                    for(; i>=0; i--) {
                         if((r.v_[i]=Xit()+Yit()+carry)>=cshift) {
                              carry=r.v_[i]/cshift;
                              r.v_[i]%=cshift;
                         } else
                              carry=0;

                    }
                    r._trim();
                    #ifdef DEBUG
					cerr<<"RESULT"<<r<<endl;
					#endif
                    return r;

               }
	///Does a virtual alignment of two numbers by means of CellIterators and then subtracts
	///the lesser of Y and *this from the greater one of the two; the sign will be plus if the first one had a greater (absolute) magnitude, or minus if not
	template<typename S>
             BigN<S> BigN<S>::_subtract(const BigN<S>& X, const BigN<S>& Y) {
        #ifdef DEBUG
                  X.printinfo(cerr);
                  Y.printinfo(cerr);
                  cerr<<"SUBTRACTING"<<endl;
                  cerr<<"\t"<<X<<" - "<<Y<<endl;
        #endif

                  //Assume X.sign_=Y.sign_
                  BigN r;
                  const BigN *px, *py;
                  px=&X;
                  py=&Y;

                  r.sign_=PLUS;

                  if(X._abs_compare(Y)<0) {
                       swap(px,py);
                       r.sign_ = MINUS;
                  }

                  //PVAR(r.sign_);
                  //if(px!=&X) cout<<"SWAPPED"<<endl;
                  CellIterator Xit(px,false);
                  CellIterator Yit(py,false);

                  size_t scale=max(px->scale_,py->scale_);
           

                  Xit.rescale(scale);
                  Yit.rescale(scale);
                    
                  size_t len=max(Xit.rel_cells(), Yit.rel_cells())+1;

                  r.v_.resize(len,0);
                  r.scale_=scale;

                  Yit.ready();
                  Xit.ready();
                  long i=len-1;

                  S borrow=0;

                  for(; i>=0; i--) {
                       S xx=Xit();
                       S sub=Yit()+borrow;
                       if(sub>xx) {
                            r.v_[i]=xx+cshift-sub;
                            borrow=1;
                       } else {
                            r.v_[i]=xx-sub;
                            borrow=0;
                       }
                  }
                  r._trim();
        #ifdef DEBUG
                  r.printinfo(cerr);
                  cerr<<"RESULT"<<r<<endl;
        #endif
                  return r;
             }
		///Does long multiplication of x and y. If sizeof(S) > sizeof(unsigned char), this will be very fast.
		template<typename S>
              BigN<S> BigN<S>::_lmultiply(const BigN<S>&x, const BigN<S>& y) {
                  BigN r;
                  // Note: Assignment multiplication provides no speed benefits; have to create a temporary in either case
                  #ifdef DEBUG
                  x.printinfo(cerr);
                  y.printinfo(cerr);
                  cerr<<"MULTIPLYING"<<endl;
				  cerr<<"\t"<<x<<" * "<<y<<endl;
					#endif
                  if(x.sign_!=y.sign_)
                       r.sign_=MINUS;
                  else {
                       if(x.sign_==MINUS)
                            r.sign_=PLUS;
                       else
                            r.sign_=x.sign_;
                  }

                  long xlen=x._cells_since_beg(), ylen=y._cells_since_beg();
                  r.v_.resize(xlen+ylen,0);
                  r.scale_=x.scale_+y.scale_;
                  const BigN *pa=&x, *pb=&y;
                  long alen=xlen;
                  if(xlen<ylen) {
                       swap(pa,pb);
                       alen=ylen;
                  }
                  const BigN /*&a=*pa,*/ &b=*pb;

                  CellIterator aIt(pa,0);
                  long skip_cells=0;
                  S carry=0;
                  for(long i=long(b.v_.size())-1; i>=b.beg_; --i) {
                       CellIterator aIt(pa,0);
                       aIt.ready();
                       S B=b.v_[i];
                       long ri=r.v_.size()-1-skip_cells;
                       long ri_stop=ri-alen-1; //one more than alen to allow for the carry to carry over
                       //Cap(S)>Cap10(HalfS)^2+Cap10(HalfS) ... i.e., there will always be plenty of space inside each cell

                       for(; ri>ri_stop; --ri) {
                            r.v_[ri]+=aIt()*B+carry;
                            if(r.v_[ri]>=cshift) {
                                 carry=r.v_[ri]/cshift;
                                 r.v_[ri]%=cshift;
                            } else
                                 carry=0;
                       }
                       skip_cells++;
                  }
                  r._trim();
                  #ifdef DEBUG
                  cerr<<"RESULT"<<r<<endl;
				  #endif
                  return r;
             }



template<>
inline short BigN<u8>::operator[](unsigned pos) const
{
     return short(v_[pos]);
}

///Prints debugging information about a BigN
template<typename S>
void BigN<S>::printinfo(ostream& os) const
         	 {
         		  if(sign_==MINUS)
         			   os<<"-"<<endl;
         		  for(auto it=v_.begin(); it!=v_.end(); ++it) {
         			   os<<setw(cshift10d)<<setfill('0')<<*it<<endl;
         		  }
         		  os<<"beg_="<<beg_<<endl;
         		  os<<"scale_="<<scale_<<endl;
         		  os<<"length="<<v_.size()<<endl;

         	 }

template<>
void BigN<u8>::printinfo(ostream& os) const
{
     if(sign_==MINUS)
          os<<"-"<<endl;
     for(auto it=v_.begin(); it!=v_.end(); ++it) {
          os.put('0'+*it)<<endl;
     }
     os<<"beg_="<<beg_<<endl;
     os<<"scale_="<<scale_<<endl;
     os<<"length="<<v_.size()<<endl;

}


}
#endif /* BIGN__HPP_ */
