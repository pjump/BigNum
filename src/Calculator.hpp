#ifndef CALCULATOR_HPP_
#define CALCULATOR_HPP_

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <limits>
#include <vector>
#include <cassert>
#include <string>
#include <locale>
#include <stdexcept>
#include <typeinfo>

#include "to_string.hpp"
#include "ShrdNum.hpp"
#include "calculator_fns_and_ops.hpp"
#include "bign_supplemental.hpp"

/**
 * \file
 * \brief Header of the Calculator Class
 */

namespace BigNum {
using namespace std;

/**
\brief Class Representing a Calculator
\tparam T numerical type used for storing numbers

The Calculator class is a templated class, parametrized by the type of numbers it works with. The class uses the >> operator or the ReadAndComp function
 to read from an istream and evaluate mathematical expressions one at a time. Expression are separated by semicolons or newlines (the former suppresses output and allows multiple expressions to be on the same line)
 The Calculators uses a modified Shunting Yard algorithm and, thanks to templates, it can deal with common mathematical operators automatically, provided that the number type provided as the template argument to Calculator has such operators working. The class is easily extendable, and the calculator_fns_and_ops.hpp header file provides simple macros for defining new usable functions and operations easily.
  (both fixed-arity and variadic functions are supported.) Storing results in variable and then using those variables in expressions are both supported.

*/

template<typename T>
class Calculator {
private:
	class Tk;
	class ValT;
	class unaryOpT;
	class prOpT;
	class poOpT;
	class infOpT;
	class FnT;

    ///Token types
	enum eTk {
          //concrete tokens
          ENOF, ///< end of file
          ENOL, ///< end of line
          ENOS,	///< end of a semi-colon separated statement
          FN,	///< a function token
          VAR,	///< a variable token
          //vague tokens
          NONE,
          OP,	///< an operator token
          NUM,  ///< a number token
          ALPH	///<an alphanumeric token (VAR or NUM)
     };

	 ///States the parser can be in
     enum eState { HAVE_VAL, WANT_VAL, WANT_LEFTP};

     typedef Stack<Tk*> TokPStackType; 	  ///<Stack of token pointers type
     typedef Stack<ValT> CalcStackType;      ///<Calculation stack type

     typedef unordered_map<string,prOpT> prOpMapType; ///<Container type for prefix operator tokens.
     typedef unordered_map<string,poOpT> poOpMapType; ///<Container type for postfix operator tokens.
     typedef unordered_map<string,infOpT> infOpMapType; ///<Container type for infix operator tokens.
     typedef unordered_map<string,T> VarMapType;	///<Container type for variable tokens.
     typedef unordered_map<string,FnT> FnMapType;	///<Container type for function tokens.
     typedef unordered_set<char> OpCharSet;		///<Container type for the charset of operator tokens.

     VarMapType vars_;			///<Stored variables
     const prOpMapType pr_ops_; 	///<Prefix operator tokens.
     const poOpMapType po_ops_;	 	///<Postfix operator tokens.
     const infOpMapType inf_ops_; 	///<Infix operator tokens.
     const FnMapType fns_;		///< Function tokens.
     const OpCharSet opchars_;		///< Operator charset.

     ///Vector/stack representing a line or semicolon-separated statement expressed in Reverse Polish Notation.
     vector<Tk*> rpn_;

     ///A reference to the stream the Calculator outputs into
     ostream& os_;
     ///Reference to a variable that stores the result of the last evaluated statement
     T& ans_;

     ///\brief Purely Virtual Base of All Token Classes
     class Tk {
     protected:
          string id_; ///< string representation of a token
     public:
          Tk(const string& id) : id_(id) {}
          Tk(string&& id="") : id_(move(id)) {}

          struct exInvalidTk : runtime_error {
               exInvalidTk(const string& arg="") : runtime_error(string("Invalid token: ")+ arg) {}
          };
          virtual string to_str() const {
               return id_;
          }; //overload for ValT if it is a number

          //// evaluate a token within the context of *parent* and in respect to the comp_stack computations stack
          virtual void eval(CalcStackType& comp_stack, Calculator& parent) = 0;
          virtual ~Tk() {};

          ///Output to the *os* ostream
          ostream& print(ostream& os) const {
               return os<<(this->to_str());
          }
          ///Prints the token to ostream os
          friend ostream& operator<<(ostream& os,const Tk& token) {
               return print(os);
          }
     };
     /*END OF Tk*/

     /** @brief Value Token.
      * Either a number or a variable 	*/

     // only these will point to new memory OpT and FnT pointers will point to const memory so won't need to be deleted
     class ValT
               : public Tk {
          T num_;		//copy of vars_[id_].second if !isnum()

     public:

          struct exInvalidVar : Tk::exInvalidTk {
               exInvalidVar(const string& arg) : Tk::exInvalidTk(string("Invalid variable: ") + arg) {}
          };
           bool isnum() const {
               return Tk::id_.length()==0;
          }
	  string id() const { return Tk::id_; }
          ValT(const T& num) : Tk(),num_(num) {}
          ValT(T&& num) : Tk(), num_(move(num)) {}
          ValT(const string& id) : Tk(id) {}
          ValT(string&& id) : Tk(move(id)) {
               /*cout<<"moving id "<<id_<<endl;*/
          }
          /*virtual*/ string to_str() const {
               if(!isnum()) return Tk::id_;
               return to_string(num_);
          };
          ///Evaluation of value tokens just pushes them onto the calculation stack
          void eval(CalcStackType& comp_stack, Calculator& parent ) {
               comp_stack.push(*this);
          };

          ///Get assignment pointer—returns a pointer that can be used for assigning a value to the value token
          T* get_ass_p(Calculator& parent )  {
               if(isnum()) return &num_; 	 //not a variable; the value must be already inside
               T* p = &(parent.vars_[Tk::id_]);
               return p; //write directly to the variable
          };
          ///Get scratch pointer—returns a pinter to a writable copy of the value (or to the value if this is just an unnamed value)
          T* get_scratch_p(Calculator& parent )  throw (exInvalidVar) {
               if(isnum()) return &num_; 	//not a variable; the value must be already inside
               auto it=parent.vars_.find(Tk::id_);
               if(it==parent.vars_.end()) throw exInvalidVar(Tk::id_);
               Tk::id_.clear(); //substitute value
               num_=it->second;
               return &num_;
          };
     };
     /// \brief Operator Token Base Class
     class OpT
               : public Tk {
     public:
    	 ///Associativity
          enum eAsc {L,
        	  R,
        	  N ///<NONE
          };
          typedef void (*fptr) (T* a1, const T* a2);

          struct exInvalidOp : Tk::exInvalidTk {
               exInvalidOp(const string& arg) : Tk::exInvalidTk(string("Invalid operator: ") + arg) {}
          };

     private:
          unsigned  prec_ : 4;	 ///<operator precedence
          fptr exec_;		///function to be executed upon operator evaluation

     public:
          OpT(const string& id, unsigned prec, fptr exec) : Tk(id),prec_(prec),exec_(exec) {}
          OpT(string&& id, unsigned prec, fptr exec) : Tk(move(id)),prec_(prec),exec_(exec) {}

          virtual void eval(CalcStackType& comp_stack, Calculator& parent) = 0;
          virtual eAsc Assoc() const = 0;
          virtual bool isBin() const = 0;
          bool isPrefix() {
               return !isBin() && Assoc()==R;
          }
          bool isPostfix() {
               return !isBin() && Assoc()==L;
          }
          ///A prec_ getter
          unsigned Prec() const {
               return prec_;
          }

          static const OpT* Find(const string& id, eState& state, const Calculator& parent);

     };	//END OD OpT


     /** \brief Base Class for Unary Operator Tokens
     
     All unary operators are unaryOpT::eval()'ed in the same way inasmuch that the second argument to the OpT::exec_ function pointer is ignored when the pointed-to function is invoked.
      */
     class unaryOpT
               : public OpT {
          typedef typename OpT::eAsc eAsc;
          typedef typename OpT::fptr fptr;
     public:
          unaryOpT(const string& id, unsigned prec, fptr exec) : OpT(id, prec, exec) {}
          unaryOpT(string&& id, unsigned prec, fptr exec) : OpT(move(id), prec, exec) {}
          //exec_ pointers still have the same signature, so the second argument is ignored
          virtual void eval(CalcStackType& comp_stack, Calculator& parent) {
               if(comp_stack.size()<1) throw runtime_error("No argument to \"" + this->to_str() + "\"");
               exec_(comp_stack.top().get_scratch_p(parent), NULL);
          };
          //virtual eAsc Assoc() = 0;
          bool isBin() const {
               return false;
          };

     };

     /// \brief Infix Operator Token
     
     /// Binary, with specified associativity. May or may not assign to variables.
     class infOpT
               : public OpT {

          typedef typename OpT::eAsc eAsc;
          typedef typename OpT::fptr fptr;

          eAsc assoc_;
          bool assigns_;

     public:

          infOpT(const string& id, unsigned prec, fptr exec, eAsc assoc, bool assigns ) :
               OpT(id,prec,exec),assoc_(assoc),assigns_(assigns) {}
          infOpT(string&& id, unsigned prec, fptr exec, eAsc assoc, bool assigns ) :
               OpT(move(id),prec,exec),assoc_(assoc),assigns_(assigns) {}
	  ///\brief Evaluation combines last two values on the stack according to the OpT::exec_ function.
	  ///Depending on whether the value is a variable and whether or not the operator assigns, the numerical value of the variable may get fetched prior to executing OpT::exec_.  Assignment to nonvariables works.
          virtual void eval(CalcStackType& comp_stack, Calculator& parent) {
               if(comp_stack.size()<2) throw runtime_error("Invalid input. Infix operator \"" + this->to_str() + "\" expects a second argument.");
               ValT* pval2=&comp_stack.top();
               ValT* pval1=pval2-1;
               const T * p2 = pval2->get_scratch_p(parent);
               T * p1;
               if(assigns_) {
                    p1 = pval1->get_ass_p(parent);
               } else {
                    p1 = pval1->get_scratch_p(parent);
               }
               exec_(p1,p2);
               /*if(assigns_)
            	   Trim(*p1);*/
               comp_stack.pop();
          };
          eAsc Assoc() const {
               return assoc_;
          };
          virtual bool isBin() const {
               return true;
          };

     };
     /** \brief Prefix Operator Token
      *
      * A unary operator with associativity R*/
     struct prOpT
               : public unaryOpT {
          typedef typename OpT::eAsc eAsc;
          typedef typename OpT::fptr fptr;
          prOpT(const string& id, unsigned prec, fptr exec) : unaryOpT(id, prec, exec) {}
          prOpT(string&& id, unsigned prec, fptr exec) : unaryOpT(move(id), prec, exec) {}
          eAsc Assoc() const {
               return OpT::R;
          };
     };
     /** \brief Prefix Operator Token
      *
      * A unary operator with associativity L*/
     struct poOpT
               : public unaryOpT {

          typedef typename OpT::eAsc eAsc;
          typedef typename OpT::fptr fptr;
          poOpT(const string& id, unsigned prec, fptr exec) : unaryOpT(id, prec, exec) {}
          poOpT(string&& id, unsigned prec, fptr exec) : unaryOpT(move(id), prec, exec) {}
          eAsc Assoc() const {
               return OpT::L;
          };

     };
     /// \brief A Function token
     class FnT : public Tk {
     public:
          typedef signed char U;
          typedef void (*fptr) (int n, T* args []);
          //maximum number of function arguments is N = numeric_limits<U>::max() == 127
          enum { NARG_MAX = 127 /**< Maximum number of function arguments; equal to numeric_limits<U>::max() */ };
     private:
          U n_;
          fptr exec_;	 ///function to be executed upon function evaluation
     public:
          FnT(const string& id, U n, fptr exec ) : Tk(id),n_(n),exec_(exec) {}
          FnT(string&& id, U n, fptr exec ) : Tk(move(id)),n_(n),exec_(exec) {}
          ///Evaluation makes use of the fact the vector in our CalcStackType is directly accessible.
          
          void eval(CalcStackType& comp_stack, Calculator& parent) {
               // arg1 arg2 arg3 argn
               auto p_last_valt=comp_stack.v_.end()-1;
               int argsn = *(p_last_valt->get_scratch_p(parent));
                              
               vector<T*> args;
               args.reserve(argsn);
               args.resize(argsn);
               auto p_first_arg = args.begin();
               auto p_valt=p_last_valt-argsn;

               while( p_valt!=p_last_valt)
                    *(p_first_arg++) = (p_valt++)->get_scratch_p(parent);

#ifdef DEBUG
               cout<<this->to_str()<<"(narg="<<argsn;
               for(auto it=args.begin(); it!=args.end(); ++it)
                    cout<<" "<<(**it);
               cout<<")"<<endl;
#endif

               exec_(argsn, args.data());

               //pop all args from the computation stack
               for(int i=0; i<argsn; i++)
                    comp_stack.pop();

          }
          static const FnT* Find(const string& id, eState& state, const Calculator& parent);
          int nargs() const {
               return n_;
          }

     };

     /// \brief Scans through operator containers to determine the operator charset.
     OpCharSet _opchars_init() {
          OpCharSet tmp;
          for(auto it=pr_ops_.begin(); it!=pr_ops_.end(); ++it) {
               string op=it->first;
               for(auto it1=op.begin(); it1!=op.end(); ++it1)
                    tmp.insert(*it1);
          }
          for(auto it=po_ops_.begin(); it!=po_ops_.end(); ++it) {
               string op=it->first;
               for(auto it1=op.begin(); it1!=op.end(); ++it1)
                    tmp.insert(*it1);
          }
          for(auto it=inf_ops_.begin(); it!=inf_ops_.end(); ++it) {
               string op=it->first;
               if(po_ops_.count(op))
                    throw logic_error(op + " cannot be defined as both a prefix and an infix operator\n");
               for(auto it1=op.begin(); it1!=op.end(); ++it1)
                    tmp.insert(*it1);
          }

          return tmp;
     }

     bool _isopchar(char c) const {
          return opchars_.count(c);
     }

     ///\brief For an emergency cleanup of token vectors/stacks.
     ///Only value tokens are deleted. Other token pointers are presumed to point either to a const function from Calculator::fns_ or to a const operator in Calculator::pr_ops_,Calculator::po_ops_, or Calculator::inf_ops_.
     template<typename U>
     void _clear_tp_vec(vector<U>& tps) {
          for(auto it=tps.begin(); it!=tps.end(); ++it )
               delete dynamic_cast<const ValT*>(*it);
          tps.clear();
     }

     int _run(bool should_print=true) ;
     eTk _getTkStr(istream& is, string& out_tok);

public:

     /**\brief The Main Function of the Calculator.
     Reads input from an input stream
     \return number of erroneous statements in the input */
     int ReadAndComp(istream& is);
     friend int operator>>(istream& is, Calculator& calc ) {
          return  calc.ReadAndComp(is);
     }
     
     ~Calculator() {
          _clear_tp_vec(rpn_);
     }

public:

//Macros to simplify writing, ignored in Doxyfile
#define OP_PR_(OPER_,PREC_,FUNC_) { OPER_, { OPER_, PREC_, Op::FUNC_ }}
#define OP_PO_(OPER_,PREC_,FUNC_) { OPER_, { OPER_, PREC_, Op::FUNC_ }}
#define OP_INF_(OPER_,PREC_,FUNC_,ASSOC_,ASSIGN_) { OPER_, { OPER_, PREC_, Op::FUNC_,OpT::ASSOC_,ASSIGN_ }}
#define FN_(FNAME_,ARGSN_) { #FNAME_, { #FNAME_, ARGSN_, Fn::FNAME_ }}

///The constructor specifies the functions operators, and variables that the Calculator instance is going to keep
     Calculator(ostream& os=cout) :
          vars_ {
          {"Pi",3.14},
          {"Ga",9.81},
          {"E",2.41}
     },
     pr_ops_ {
          //RIGHT ASSOCIATIVE
          OP_PR_("(", 12, noop),
          OP_PR_("-", 2,uminus),
          OP_PR_("+", 2,uplus),
          OP_PR_("!", 2,ulnot)
     },
     po_ops_ {
          //LEFT ASSOCIATIVE
          OP_PO_(")",1, noop),
          OP_PO_("!",0, ofact)
     },
     inf_ops_ {
          OP_INF_("^",1,raise,R,0),
          OP_INF_("*",3,times,L,0),
          OP_INF_("/",3,div,L,0),
          //OP_INF_("%",3,times,L,0),
          OP_INF_("-",4,minus,L,0),
          OP_INF_("+",4,plus,L,0),
          OP_INF_("<",5,lt,L,0),
          OP_INF_("<=",5,lte,L,0),
          OP_INF_(">",6,gt,L,0),
          OP_INF_(">=",6,gte,L,0),
          OP_INF_("==",7,eq,L,0),
          OP_INF_("!=",7,neq,L,0),
          OP_INF_("&&",8,land,L,0),
          OP_INF_("||",9,lor,L,0),
          OP_INF_("=",10,assign,L,1),
          OP_INF_("+=",10,aplus,L,1),
          OP_INF_("-=",10,aminus,L,1),
          OP_INF_("*=",10,atimes,L,1),
          OP_INF_("/=",10,adiv,L,1),
          OP_INF_(",",12,noop,L,0)
     },
     fns_ {
          FN_(neg,1),
          FN_(fact,1),
          FN_(max,-1),
          FN_(avg,-1),
          FN_(min,-1),
          FN_(sum2,2),
          FN_(help,0)

     },
     opchars_(_opchars_init()),
              os_(os),
              ans_(vars_["ans"])

     {
     }

};

} //namespace BigN


#include "calculator_parsing.hpp"
#endif /* CALCULATOR_HPP_ */
