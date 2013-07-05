#ifndef CALCULATOR_PARSING_HPP_
#define CALCULATOR_PARSING_HPP_
#include "Calculator.hpp"

/*
#ifdef DEBUG
	#ifndef DB
		#define DB(X) DB(X)
	#endif
#else
	#define DB(X)
#endif
*/
//#define DEBUG_TOKENIZER
#ifdef DEBUG_TOKENIZER
#define DBT(X) DB(X)
#else
#define DBT(X)
#endif

#ifdef DEBUG_PARSER
#define DBP(X) DB(X)
#else
#define DBP(X)
#endif

/**\file
 * \brief This file contains the implementation of some longer functions of the Calculator class.
 */

namespace BigNum {

/**
 * \brief Searches operator containers in `parent` for the right operator in respect to `state`
 * 
   \return NULL if no suitable operator is found, otherwise a const* Calculator::OpT pointing to a member in a Calculator member container.
 */
template<typename T>
const typename Calculator<T>::OpT * Calculator<T>::OpT::Find(const string& id,
          typename Calculator<T>::eState& state, const Calculator<T>& parent)
{


     if(state==WANT_VAL || state==WANT_LEFTP) {
          //PREFIX
          auto cit=parent.pr_ops_.find(id);
          if(cit != parent.pr_ops_.end()) {
               if(state==WANT_LEFTP && cit->first != "(")
                    return NULL;
               return &(cit->second);
          }
          return NULL;
     };
     //state==HAVE_VAL --- needs to be either a inf_ops poOpT or a infOpT
     {
          //INFIX
          auto cit=parent.inf_ops_.find(id);
          if(cit != parent.inf_ops_.end()) {
               state=WANT_VAL;
               return &(cit->second);
          }
     }
     {
          //POSTFIX
          auto cit=parent.po_ops_.find(id);
          if(cit != parent.po_ops_.end()) {
               if(cit->first == ")")
                    DBT("HAVE_VAL"<<endl);
               return &(cit->second);

          }
     }
     return NULL;
}

template<typename T>
/**
 * \brief Searches the function container in `parent` for the right operator in respect to `state`
 * 
   \return NULL if no suitable operator is found, otherwise a const* Calculator::FnT pointing to a member in the function container in Calculator
 */
const typename Calculator<T>::FnT * Calculator<T>::FnT::Find(const string& id,
          typename Calculator<T>::eState& state, const Calculator<T>& parent)
{
     //Search operator containers in `parent` for the right operator in respect to `state`
     //Return NULL if not found

     if(state==WANT_VAL) {
          //POSTFIX
          auto cit=parent.fns_.find(id);
          if(cit != parent.fns_.end()) {
               return &(cit->second);
          }
     }
     return NULL;
}




template<typename T>
/** \brief Get token string.
  
Gets a token string and its type from *istream is*. If *NUM* is returned, *out_tok* won't be set—the caller must read it itself.
\param [in] is input stream
\param [out] out_tok string to write to
\return token type
*/
typename Calculator<T>::eTk
Calculator<T>::_getTkStr(istream& is, string& out_tok)
{

     eTk type=NONE;

     out_tok.clear();
     int c;
     while((c=is.get())!=EOF) if(!isblank(c)) break;
     if(c=='\n')  return ENOL;
     if(c==EOF) return ENOF;
     if(c==';') return ENOS;


     //check the first character
     if (isdigit(c) || c=='.') {
          is.putback(char(c));
          type=NUM;
          return type;
     } else {
          out_tok+=char(c);
          if ( isalpha(c) )
               type=ALPH;
          else {
               type=OP;
               if(c=='('||c==')')
                    return type;
               else if(!_isopchar(c))
                    throw typename Tk::exInvalidTk(string()+char(c));
          }
     }
     //c is in out, type== ALPH || type==OP

     while((c=is.get())!=EOF) {

          //check if we need to get out_tok
          if(isblank(c) || c=='(' || c==')' ||c==';' )
               break;

          if ( isdigit(c) || isalpha(c) ) {
               if(type!=ALPH)
                    break;
          } else if(_isopchar(c)) {
               if(type!=OP)
                    break;
          } else
               break;


          out_tok+=char(c);
     }
     is.putback(char(c));
     return type;
}

/** \brief Run the evaluation of the rpn_ vector, printing out the result if *should_print==true*
    
    Whether successful or not,this will wipe rpn_ vector completely. If an exception is thrown inside Calculator::_run, 
    then Calculator::_run will do the clean up and write the appropriate error message to *cerr*.
 */
template<typename T>
int Calculator<T>::_run(bool should_print)  
{
    //#define DEBUG_RPN
    #ifdef DEBUG_RPN
     cout<<"RPN"<<endl;
     cout<<"rpn_.size()="<<rpn_.size()<<endl;
     for(auto it=rpn_.begin(); it!=rpn_.end(); ++it) {
          OpT *p=dynamic_cast<OpT*>(*it);
          if(p) {
               cout<<"inf="<<p->isBin()<<"\t"<<p->to_str()<<"\t"<<p->Assoc()<<endl;

          } else{
			    cout<<(*it)->to_str()<<endl;
		}
            
     }
     cout<<"END"<<endl;
    #endif
  
     enum {SUCCESS=0, FAILURE=1};

     if(rpn_.size()==0) {
          os_<<endl;
          return FAILURE;
     }

     CalcStackType comp_stack;

     auto it=rpn_.begin();
     try {
          for( ; it!=rpn_.end(); ++it ) {
               (*it)->eval(comp_stack, *this);
                delete dynamic_cast<ValT*>(*it); *it=NULL;
          }
      if(comp_stack.size()>1) {
		  cout<<"stack size="<<comp_stack.size()<<endl;
		  cout<<(comp_stack.top().to_str())<<endl;
		throw runtime_error("Error while computing the expression.");
	  }
	  ValT& valref=comp_stack.top(); 
	  string id=valref.id();
	  T *p=valref.get_scratch_p(*this); 
	  assign(ans_,*p);
	  
	  if(should_print){
	    if(id.length())
	      os_<<id<<"=="<<ans_<<"\n";
	    else
	      os_<<ans_<<"\n";
	  }
	  
	  comp_stack.pop();
          rpn_.clear();

       
    } catch(const runtime_error& e) {
          cerr<<"Error: "<<e.what()<<endl;
          for( ; it!=rpn_.end(); ++it ) {
               delete dynamic_cast<ValT*>(*it); *it=NULL;
          }
          rpn_.clear();
          return FAILURE;
     }

    return SUCCESS;
}

/** \brief Read a Math Expression and Compute its Value
\arg is istream to read expressions from
\return int number of failures encountered
Reads an expression from *istream is* and internally transforms it into a Reverse Polish Notation expression, which it then evaluates.
If the expression ends with a semicolon, the result is not outputted; if it ends with a newline, it gets printed to Calculator::os_.
The function throws no exceptions other than a possible std::bad_alloc, and it should be resilient to failure (No input should crash it).
*/
template<typename T>
int Calculator<T>::ReadAndComp(istream& is)
{

     Stack<const Tk*> opstack; //operator stack
     //output (rpn line)

     eState state=WANT_VAL;

     int Failures=0;

     const Tk *tkp1;
     const OpT *op1,*op2;
     int argsn=-1;
     int argsn_tmp;


     while(is) {
          eTk tt=NONE;
          string id;

          try {

               tt=_getTkStr(is,id);

               if(id!=")" && argsn==0)
                    argsn=1;



               switch(tt) {
               case NUM: {
                    T num;
                    is>>num;
                    if(!is) {
                         is.clear();
                         throw typename Tk::exInvalidTk("");
                    }
                    DBT(to_string(num)<<endl);
                    DBT("NUM\t"<<num<<endl);
                    DBP("NUM\t"<<num<<endl);
                    DBT("HAVE_VAL"<<endl);
                    rpn_.push_back(new ValT(move(num)));
                    state=HAVE_VAL;

                    break;
               }
               case ALPH: {
                    //A FUNCTION TOKEN?
                    tkp1=FnT::Find(id,state,*this);
                    if(tkp1) {
                         DBT("FN\t"<<id<<endl);
                         DBP("FN\t"<<id<<endl);
                         opstack.push(tkp1);
                         state=WANT_LEFTP;

                         break;
                    }

                    //VAR TOKEN (possibly uninitialized)
                    DBT("\tVARIABLE\t"<<id<<endl);
                    rpn_.push_back(new ValT(id));
                    state=HAVE_VAL;
                    break;
               }
               case OP: {
                    DBT("OP\t"<<id<<endl);
                    if(id==",") {
                         if(state!=HAVE_VAL)
                              throw typename OpT::exInvalidOp(id);
                         argsn++;
                         //POP until opstack.top()->to_str()=="(" or opstack.empty()
                         while(!opstack.empty() && opstack.top()->to_str()!="(") {
                              opstack.pop();
                         }
                         state=WANT_VAL;

                    } else if(id=="(") {
                         if(state==WANT_LEFTP)
                              state=WANT_VAL;
                         if(state==HAVE_VAL)
                              opstack.push(const_cast<OpT*>(OpT::Find("*",state,*this)));

                         opstack.push(new ValT(argsn));
                         opstack.push(const_cast<OpT*>(OpT::Find(id,state,*this)));
                         DBP("("<<argsn<<endl);
                         argsn=0;

                    } else if(id==")") {
                         if(state==WANT_VAL)
                              state=HAVE_VAL;
                         argsn_tmp=argsn;
                         DBP(")"<<endl);
                         DBT(")"<<endl);

                         while(!opstack.empty() && opstack.top()->to_str()!="(") {
                              rpn_.push_back(const_cast<Tk*>(static_cast<const Tk*>(opstack.top())));
                              opstack.pop();
                         }
                         if(opstack.empty())
                              throw typename Tk::exInvalidTk(")"); //!! no matching "("
                         opstack.pop(); //popping the "("
                         argsn_tmp=argsn;
                         DBP("("<<argsn<<endl);
                         argsn=int(*(((ValT*)(opstack.top()))->get_scratch_p(*this)));
                         delete (ValT*)(opstack.top());
                         opstack.pop();
                         if(opstack.empty())
                              break;
                         const FnT* fnp = dynamic_cast<const FnT*>(opstack.top());
                         if(fnp) {
                              rpn_.push_back(new ValT(argsn_tmp));
                              rpn_.push_back(const_cast<Tk*>(static_cast<const Tk*>(fnp)));
                              opstack.pop();
                         }



                    } else {
                         //OTHER OPERATOR
                         tkp1=OpT::Find(id,state,*this);

                         if(!tkp1)
                              throw typename OpT::exInvalidOp(id);

                         op1=static_cast<const OpT*>(tkp1);


                         while(!opstack.empty() && (op2=dynamic_cast<const OpT*>(opstack.top())) && (
                                        (op1->Assoc()==OpT::L && op1->Prec()==op2->Prec())
                                        || op1->Prec() > op2->Prec()
                                   )) {

                              rpn_.push_back(const_cast<Tk*>(static_cast<const Tk*>(op2)));
                              opstack.pop();
                         }

                         opstack.push(const_cast<Tk*>(tkp1));

                         DBP((op1->to_str())<<"\t"<<"isb="<<(op1->isBin())<<"\tlass="<<(op1->Assoc()==OpT::L)<<endl);

                    }

               }
               break;
               case ENOS:
               case ENOL:
               case ENOF:
                    state=WANT_VAL;
                    //in any case, empty the opstack
                    while(!opstack.empty())	{
                         //get string representation
                         op2=static_cast<const OpT*>(opstack.top());
                         string op2s=op2->to_str();

                         if(op2s=="("||op2s==")")
                              throw typename Tk::exInvalidTk(op2s);

                         rpn_.push_back(const_cast<Tk*>(static_cast<const Tk*>(op2)));
                         opstack.pop();
                    }
                    if(tt==ENOS) {
                         DBT("ENOS\t;\n");
                         Failures+=_run(0);
                         break;
                    }
                    if(tt==ENOL) {
                         DBT("ENOL\t\n\n");
                         Failures+=_run(1);
                         break;
                    }
                    if(tt==ENOF) {
                         DBT("ENOF\n");		/*cerr<<"EOF"<<endl;*/
                         return Failures;
                    }

                    break;
               default:
                    throw runtime_error("Unknown token");
                    break;
               }//switch
          } catch(typename Tk::exInvalidTk& e) {
               Failures++;
               cerr<<"Exception "<<(e.what())<<endl;
               //Clear till the end of statement/line/file
               cerr<<"Invalidating the rest of the statement"<<endl;

               for(int ch=is.get(); ch!='\n' && ch!=';' && ch!=EOF; ch=is.get())
                    ;;
               _clear_tp_vec(rpn_);
               _clear_tp_vec(opstack.v_);
               state=WANT_VAL;
          }

     }//while

     return Failures;

}

}

#endif /* CALCULATOR_PARSING_HPP_*/
