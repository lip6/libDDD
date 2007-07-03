#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include "Nat_Const.hpp"

using std::vector ;
using std::cout;
using std::endl;
using std::string;

// a dataset for "+"
const IntDataSet natPlus = vector<int> (1,PLUS) ;
// a dataset for "succ"
const IntDataSet natSucc = vector<int> (1,SUCC) ;
// a dataset for constant 0
const IntDataSet natZero = vector<int> (1, 0) ;
// a dataset for constant 1
const IntDataSet natOne = vector<int> (1, 1) ;

// a SDD dataset for constant +
const SDD SDDnatSucc =SDD (NAT, natSucc ) ;
// a SDD dataset for constant +
const SDD SDDnatPlus =SDD (NAT, natPlus ) ;
// a SDD dataset for constant 0
const SDD SDDnatZero = SDD (NAT, natZero ) ;
// a SDD dataset for constant 1
const SDD SDDnatOne = SDD (NAT, natOne ) ;

void printExpression(std::ostream& os,const GSDD *d, std::string s, bool withendl) 
{
  if (*d == GSDD::one) {
    os <<  s ;
    if (withendl) os << endl ;
    else  os << ";" ;
  } else if(*d == GSDD::top) {
    os <<  s << " TOP "<<endl;
  } else if(*d == GSDD::null) {
    os <<  s << " NULL "<<endl;
  } else {
    int vr = d->variable();
    if (vr == NAT) {
      for (GSDD::const_iterator it = d->begin() ; it != d->end() ; ++it ) {	
	std::stringstream tmp;
	tmp << " Nat[" ;
	IntDataSet * ds = (IntDataSet*) it->first;
	for (IntDataSet::const_iterator jt = ds->begin() ; jt != ds->end() ; ++jt) {
	  switch (*jt) {
	  case PLUS :
	    tmp << " + ";
	    break;
	  case SUCC :
	    tmp << " succ ";
	    break ;
	  default :
	    tmp << " " << *jt << " ";
	    break;
	  }
	}
        tmp << "]" ;
	printExpression(os, & it->second , s + tmp.str() + " ", withendl);
      }      
    } else if (vr == LEFT || vr == RIGHT || vr == SUCCARG ) {
      
      for(GSDD::const_iterator vi=d->begin();vi!=d->end();++vi){
	std::stringstream tmp;
	// Fixme  for pretty print variable names
	//      tmp<<"" << variable() <<  " " ;
	tmp << "(" ;
	printExpression(tmp,(const SDD *) vi->first,"",false);
	tmp << ")" ;
	printExpression(os, &vi->second , s+tmp.str()+" ", withendl);
      }    
    } else {
      os << " WTF ??" ;
    }
    //   else if (vr == BOOL) {
    //       // TO BE DONE
    
  }
}
