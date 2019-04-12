/****************************************************************************/
/*								            */
/* This file is part of libDDD, a library for manipulation of DDD and SDD.  */
/*     						                            */
/*     Copyright (C) 2001-2008 Yann Thierry-Mieg, Jean-Michel Couvreur      */
/*                             and Denis Poitrenaud                         */
/*     						                            */
/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU Lesser General Public License as       */
/*     published by the Free Software Foundation; either version 3 of the   */
/*     License, or (at your option) any later version.                      */
/*     This program is distributed in the hope that it will be useful,      */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/*     GNU LEsserGeneral Public License for more details.                   */
/*     						                            */
/* You should have received a copy of the GNU Lesser General Public License */
/*     along with this program; if not, write to the Free Software          */
/*Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*     						                            */
/****************************************************************************/

/* -*- C++ -*- */
#include <set>
#include <map>
#include <typeinfo>
#include <cassert>
#include <iostream>

#include "ddd/util/configuration.hh"
#include "ddd/DataSet.h"
#include "ddd/DED.h"
#include "ddd/SDD.h"
#include "ddd/SDED.h"
#include "ddd/SHom.h"

#include "ddd/UniqueTable.h"

#ifdef REENTRANT
# include "tbb/atomic.h"
#endif

/******************************************************************************/
/******************************************************************************/
class _SDED{
public:
  GSDD result;
  /* Destructor */
  virtual ~_SDED(){};

  /* Compare */
  virtual size_t hash() const =0;
  virtual bool operator==(const _SDED &) const=0;
  virtual _SDED * clone () const =0;
  /* Transform */
  virtual GSDD eval() const  = 0; 

};

typedef UniqueTable<_SDED> SDEDtable;

static SDEDtable uniqueSDED;


namespace namespace_SDED {
    
#ifdef REENTRANT

  static tbb::atomic<int> Hits;
  static tbb::atomic<int> Misses;
  static tbb::atomic<size_t> Max_SDED;
  
  class SDED_parallel_init
  {
  public:
    
    SDED_parallel_init()
    {
      Hits = 0;
      Misses = 0;
      Max_SDED = 0;
    }
    
  };
  static SDED_parallel_init SDED_init;
  
#else
  
static int Hits=0;
static int Misses=0;
static size_t Max_SDED=0;


#endif

} //namespace namespace_SDED 

static GSDD compute (const _SDED & op) {
  return uniqueSDED(op)->result;  
}

/******************** BASIS FOR CANONIZATION OPERATIONS **********************/
void square_union (std::map<GSDD,DataSet *> &res,const GSDD & s, DataSet * d) {
	
  std::map<GSDD,DataSet *>::iterator kt = res.find(s);
  if (kt != res.end()) {
    /* found it in res compute union */
    DataSet * tofree = kt->second;
    kt->second  = kt->second->set_union(*d);
    delete tofree;
  } else {
    /* not yet in res, add it */
    res.insert(std::make_pair(s,d->newcopy()));
  }
}


#ifdef EVDDD
/// increment the value of the next variable
class _pushEVSDD:public StrongShom {
  int dist;
public:
  _pushEVSDD(int dist_) :dist(dist_){}

  GSDD phiOne() const {
    return GSDD::one;
  }                   

  GShom phi(int vr, const DataSet & vl) const {    
    return GShom(vr,*vl.normalizeDistance(dist));
  }

  size_t hash() const {
    return 16759*dist;
  }

  bool operator==(const StrongShom &s) const {
    return dist == ((const _pushEVSDD &)s).dist;
  }
  _GShom * clone () const { return new _pushEVSDD(*this); }
};
/// User function : Construct a Hom for a Strong Hom _plusplus
GShom pushEVSDD(int v){return new _pushEVSDD(v);};
#endif



/******************************************************************************/
/*                    class _SDED_Add:public _SDED                              */
/******************************************************************************/
class _SDED_Add:public _SDED{
private:
  typedef std::vector<GSDD> parameters_t; 
  typedef parameters_t::const_iterator parameters_it;
  parameters_t parameters;
  _SDED_Add(const d3::set<GSDD>::type &d):parameters(d.begin(),d.end()){};
public:
  static GSDD create(const GSDD &g1,const GSDD &g2);
  static GSDD create(const d3::set<GSDD>::type &d);
  /* Compare */
  size_t hash() const;
  bool operator==(const _SDED &e)const;
  _SDED * clone () const { auto res = new _SDED_Add(*this); res->result = res->eval() ; return res; }

  /* Transform */
  GSDD eval() const;

  /* constructor*/
  ~_SDED_Add(){};
};

/*********/
/* Compare */
size_t _SDED_Add::hash() const{
  size_t res=0;
  for(parameters_it si=parameters.begin();si!=parameters.end();++si){
    res+= si->hash();
  }
  return res;
}



bool _SDED_Add::operator==(const _SDED &e)const{
  return (parameters==((_SDED_Add*)&e)->parameters);
}

/* Transform */
GSDD _SDED_Add::eval() const{
  assert(parameters.size() > 1);
  int variable=parameters.begin()->variable();
  // To compute the result 
  std::map<GSDD,DataSet *> res;

  // for memory collection
  DataSet * tofree;

  // The current operand
  parameters_it opit =  parameters.begin();

  // Initialize with copy of first operand
  for (GSDD::Valuation::const_iterator it = opit->begin();it != opit->end() ; ++it) 
    res[it->second]=it->first->newcopy();

  // main loop
  // Foreach  opit in (operands)
  for (opit++ ; opit != parameters.end() ; ++opit) {
    // To store non empty intersection results and remainders;
    std::vector< std::pair <GSDD,DataSet *> > sums;
    std::vector< std::pair <GSDD,DataSet *> > rems;
    
    // Foreach arc in current operand  : e-a->A
    for (GSDD::Valuation::const_iterator arc = opit->begin() ; arc != opit->end() ; arc++ ) {      
      DataSet * a = arc->first->newcopy();
      // foreach value already in result : e-b->B
      for (std::map<GSDD,DataSet *>::iterator resit = res.begin() ; resit != res.end() ;  ) {
	DataSet *b = resit->second;
	 // test for equality first, fastest test
	if ( a->set_equal(*b) ) {
	  // will be reinserted in the result for testing against the next operand of union
	  sums.push_back( std::make_pair(resit->first + arc->second , a) );
	  // no more need to test against this element
	  std::map<GSDD,DataSet *>::iterator jt = resit;
	  ++resit;
	  delete jt->second;
	  res.erase(jt);
	  //  break to next arc of this operand, a has been emptied
	  a = a->empty_set();
	  break;
	}

	// compute a*b
	DataSet *ainterb = arc->first->set_intersect(*resit->second);
	// if a*b = 0, skip
	if (ainterb->empty()) {
	  delete ainterb;
	  ++resit;
	  // skip to next arc of res
	  continue;
	}
	
	// non empty intersection + non equality
	// add intersection to sums : e- b * a -> A+B
	sums.push_back( std::make_pair(resit->first + arc->second , ainterb) );
	
	// Test containment case
	if ( b->set_equal(*ainterb) ) {
	  // a contains b (STRICTLY, equality tested above)
	  // remove the b mapping from the test set
	  std::map<GSDD,DataSet *>::iterator jt = resit;
	  resit++;
	  delete jt->second;
	  res.erase(jt);	  
	} else {
	  // update result : res[cur] -= ainterb :  e- b \ a -> B
	  tofree = resit->second;
	  resit->second = resit->second->set_minus(*ainterb);
	  delete tofree;
	  resit++;
	}

	// update a (sieve b values) 
	tofree = a ;
	a = a->set_minus(*ainterb);
	delete tofree;

	// test terminal containment case
	if (a->empty()) {
	  // we can stop, a is fully treated, break to next arc
	  break;
	}      

      } // end foreach resit in result
      
      // if there is a remainder, store it
      if (! a->empty()) {
	// traversed sieve without emptying a
	rems.push_back(std::make_pair(arc->second,a));
      } else 
	delete a;
    } // end foreach arc in operand
    
    // Now process remainders and sums
    for (std::vector< std::pair <GSDD,DataSet *> >::iterator it=sums.begin(); it != sums.end(); ++it ) {
      square_union(res,it->first,it->second);
      delete it->second;
    }
    for (std::vector< std::pair <GSDD,DataSet *> >::iterator it=rems.begin(); it != rems.end(); ++it ) {
      square_union(res,it->first,it->second);
      delete it->second;
    }

//     if ( ! opit->isSon() && ! opit->refCounter() )
//       opit->clearNode();
  } // end foreach operand

  GSDD::Valuation value;

#ifdef EVDDD
  // foreach value already in result : e-b->B
  for (std::map<GSDD,DataSet *>::iterator resit = res.begin() ; resit != res.end() ;  ) {
    int mindist = resit->first.getMinDistance();
    if (mindist>0) {
      std::map<GSDD,DataSet *>::iterator jt = resit;
      resit++;
//      square_union(res, jt->first.normalizeDistance(-mindist), jt->second->normalizeDistance(mindist));
//       std::cerr << "got positive mindist = " << mindist <<std::endl;
//       std::cerr << "apply -mindist to = " <<jt->first << "gives \n" << jt->first.normalizeDistance(-mindist) <<std::endl;
//       std::cerr << "apply +mindist to = " ;
//       jt->second->set_print(std::cerr);
//       std::cerr << "gives \n" ; 
//       jt->second->normalizeDistance(mindist)->set_print(std::cerr);
//       std::cerr  <<std::endl;
      res[jt->first.normalizeDistance(-mindist)] = jt->second->normalizeDistance(mindist);
      delete jt->second;
      res.erase(jt);
    } else {
      resit++;
    }
  }
#endif

  value.reserve(res.size());  
  for (std::map<GSDD,DataSet *>::iterator it =res.begin() ;it!= res.end();++it) {
    assert ( ! it->second->empty() && it->first != GSDD::null);
      value.push_back(std::make_pair(it->second,it->first));
  }

//   int id=0;
//   std::cerr << "operating over parameters : " <<std::endl ;
//   for (d3::set<GSDD>::type::const_iterator it = parameters.begin() ; it != parameters.end() ; it++ ) {
//     std::cerr << "PARAM "<< id++ << "  "<< *it << std::endl ;
//   }
  // GSDD ret(variable,value);
//   std::cerr << "produced node : " << ret << std::endl ;
  // return ret;


  return GSDD(variable,value);
};

/* constructor*/
GSDD _SDED_Add::create(const GSDD &g1,const GSDD &g2){
  if (g1 == g2)
    return g1;
  
  if(g1 == GSDD::null)
    return g2;
  if (g2 == GSDD::null)
    return g1;

  if (g1 == GSDD::one || g2 == GSDD::one || g1 == GSDD::top || g2 == GSDD::top || g1.variable() != g2.variable() ) 
    return GSDD::top;

  d3::set<GSDD>::type parameters;
  parameters.insert(g1);
  parameters.insert(g2);

  return compute(_SDED_Add(parameters));
}
/* constructor*/
GSDD _SDED_Add::create(const d3::set<GSDD>::type &s){
  assert(s.size()!=0); // s is not empty
  d3::set<GSDD>::type parameters=s;
  parameters.erase(GSDD::null);
  if(parameters.size()==1)
    return *parameters.begin();  
  else { 
    if(parameters.size()==0)
      return GSDD::null;
    else if(parameters.find(GSDD::top)!=parameters.end()||parameters.find(GSDD::one)!=parameters.end()){  
      return GSDD::top;
    }
    else{ 
      d3::set<GSDD>::type::const_iterator si=parameters.begin();
      int variable = si->variable();
      for(;(si!=parameters.end())?(variable == si->variable()):false;++si){}
      if(si!=parameters.end())// s contains at least 2 GDDDs with different variables
	return GSDD::top;
      return compute(_SDED_Add(parameters));    
    }
  }

}



/******************************************************************************/
/*                    class _SDED_Mult:public _SDED                             */
/******************************************************************************/
class _SDED_Mult:public _SDED{
private:
  GSDD parameter1;
  GSDD parameter2;
  _SDED_Mult(const GSDD &g1,const GSDD &g2):parameter1(g1),parameter2(g2){};
public:
  static  GSDD create(const GSDD &g1,const GSDD &g2);
  /* Compare */
  size_t hash() const;
  bool operator==(const _SDED &e)const;
  _SDED * clone () const { auto res = new _SDED_Mult(*this); res->result = res->eval() ; return res; }

  /* Transform */
  GSDD eval() const;

  /* constructor*/
  ~_SDED_Mult(){};
};

/*********/
/* Compare */
size_t _SDED_Mult::hash() const{
  return parameter1.hash()+13*parameter2.hash();
};

bool _SDED_Mult::operator==(const _SDED &e)const{
  return ((parameter1==((_SDED_Mult*)&e)->parameter1)&&(parameter2==((_SDED_Mult*)&e)->parameter2));
};

/* Transform */
GSDD _SDED_Mult::eval() const{
  assert(parameter1.variable()==parameter2.variable());
  int variable=parameter1.variable();
  std::map<GSDD,DataSet *> res;



  GSDD s1inters2 ;
  // for each son of p1 :   v - a -> s1 
  for (GSDD::Valuation::const_iterator it = parameter1.begin();it != parameter1.end() ; ++it) {
    // for each son of p2 :   v - b -> s2 
    for (GSDD::Valuation::const_iterator jt = parameter2.begin();jt != parameter2.end() ; ++jt) {
      // test for equality first, fastest test
      if ( it->first->set_equal(*jt->first) ) {
	square_union(res,it->second * jt->second, it->first);
	// break out of inner loop
	break;
      }
      // compute a*b
      DataSet *ainterb = it->first->set_intersect(*jt->first);
      // if a*b = 0, skip
      if (ainterb->empty() ) {
	delete ainterb;
	continue;
      }
      s1inters2 = it->second * jt->second;
      square_union(res,s1inters2,ainterb);

      // b contains a
      if ( it->first->set_equal(*ainterb) ) {
	delete ainterb;
	// we can stop exploring for this operand
	break;
      }

      delete ainterb;
    }
  }

  GSDD::Valuation value;
  std::map<GSDD,DataSet *>::iterator nullmap = res.find(GSDD::null);
  if (nullmap != res.end()){
    delete nullmap->second;
    res.erase(nullmap);
  }
  value.reserve(res.size());  
  for (std::map<GSDD,DataSet *>::iterator it =res.begin() ;it!= res.end();++it)
    value.push_back(std::make_pair(it->second,it->first));
  
  return GSDD(variable,value);
};

/* constructor*/
GSDD _SDED_Mult::create(const GSDD &g1,const GSDD &g2){
  if(g1==g2)
    return g1;
  else if(g1==GSDD::null||g2==GSDD::null)
    return GSDD::null;
  else if(g1==GSDD::one||g2==GSDD::one||g1==GSDD::top||g2==GSDD::top)
    return GSDD::top;
  else if(g1.variable()!=g2.variable())
    return GSDD::null;
  else if(g1.hash() < g2.hash())
    return compute(_SDED_Mult(g1,g2));
  else
    return compute(_SDED_Mult(g2,g1));
};

/******************************************************************************/
/*                    class _SDED_Minus:public _SDED                            */
/******************************************************************************/
class _SDED_Minus:public _SDED{
private:
  GSDD parameter1;
  GSDD parameter2;
  _SDED_Minus(const GSDD &g1,const GSDD &g2):parameter1(g1),parameter2(g2){};
public:
  static  GSDD create(const GSDD &g1,const GSDD &g2);
  /* Compare */
  size_t hash() const;
  bool operator==(const _SDED &e)const;
  _SDED * clone () const { auto res = new _SDED_Minus(*this);  res->result = res->eval() ; return res;}
  /* Transform */
  GSDD eval() const;

  /* constructor*/
  ~_SDED_Minus(){};
};

/*********/
/* Compare */
size_t _SDED_Minus::hash() const{
  return 617*parameter1.hash()+307*parameter2.hash();
};

bool _SDED_Minus::operator==(const _SDED &e)const{
  return ((parameter1==((_SDED_Minus*)&e)->parameter1)&&(parameter2==((_SDED_Minus*)&e)->parameter2));
};

/* Transform */
GSDD _SDED_Minus::eval() const{
  assert(parameter1.variable()==parameter2.variable());
  int variable=parameter1.variable();

  std::map<GSDD,DataSet *> res;
  // remainder for p1 
  std::map<GSDD,DataSet *> rem_p1;

  // for each son of p1 initialize remainder
  for (GSDD::Valuation::const_iterator it = parameter1.begin();it != parameter1.end() ; ++it) 
    rem_p1[it->second] = it->first->newcopy();


  GSDD s1moinss2 ;
  // for each son of p1 :   v - a -> s1 
  for (GSDD::Valuation::const_iterator it = parameter1.begin();it != parameter1.end() ; ++it) {
    // for each son of p2 :   v - b -> s2 
    for (GSDD::Valuation::const_iterator jt = parameter2.begin();jt != parameter2.end() ; ++jt) {
      // compute a*b
      DataSet * ainterb = it->first->set_intersect(*jt->first);
      // if a*b = 0, skip
      if (ainterb->empty()) {
	delete ainterb;
	continue;
      }
      
      s1moinss2 = it->second - jt->second;
      square_union(res,s1moinss2,ainterb);
      
      DataSet * tofree = rem_p1[it->second];
      rem_p1[it->second] = rem_p1[it->second]->set_minus(*ainterb);
      delete tofree;
      delete ainterb;
    }
  }
  // add remainders
  // for each son of p1 
  for (std::map<GSDD,DataSet *>::const_iterator it = rem_p1.begin();it != rem_p1.end() ; ++it) {
    if (! it->second->empty() )      
      {
	square_union(res,it->first,it->second);
      }
    delete it->second;
  }

  GSDD::Valuation value;
  std::map<GSDD,DataSet *>::iterator nullmap = res.find(GSDD::null);
  if (nullmap != res.end()) {
    delete nullmap->second;
    res.erase(nullmap);
  }
  value.reserve(res.size());  
  for (std::map<GSDD,DataSet *>::iterator it =res.begin() ;it!= res.end();++it)
    value.push_back(std::make_pair(it->second,it->first));
 
  return GSDD(variable,value);
};

/* constructor*/
GSDD _SDED_Minus::create(const GSDD &g1,const GSDD &g2){
  if(g1==g2||g1==GSDD::null)
    return GSDD::null;
  else if(g2==GSDD::null)
    return g1;
  else if(g1==GSDD::one||g2==GSDD::one||g1==GSDD::top||g2==GSDD::top)
    return GSDD::top;
  else if(g1.variable()!=g2.variable())
    return g1;
  else
    return compute(_SDED_Minus(g1,g2));
};

/******************************************************************************/
/*                    class _SDED_Concat:public _SDED                           */
/******************************************************************************/
class _SDED_Concat:public _SDED{
private:
  GSDD parameter1;
  GSDD parameter2;
  _SDED_Concat(const GSDD &g1,const GSDD &g2):parameter1(g1),parameter2(g2){};
public:
  static GSDD create(const GSDD &g1,const GSDD &g2);
  /* Compare */
  size_t hash() const;
  bool operator==(const _SDED &e)const;
  _SDED * clone () const { auto res = new _SDED_Concat(*this);  res->result = res->eval() ; return res;}
  /* Transform */
  GSDD eval() const;

  /* constructor*/
  ~_SDED_Concat(){};
};

/*********/
/* Compare */
size_t _SDED_Concat::hash() const{
  return 827*parameter1.hash()+1153*parameter2.hash();
};

bool _SDED_Concat::operator==(const _SDED &e)const{
  return ((parameter1==((_SDED_Concat*)&e)->parameter1)&&(parameter2==((_SDED_Concat*)&e)->parameter2));
};

/* Transform */
GSDD _SDED_Concat::eval() const{
  int variable=parameter1.variable();

  std::map<GSDD,DataSet *> res;
  GSDD next;
  
  for(GSDD::const_iterator v1=parameter1.begin();v1!=parameter1.end();++v1){
    next = (v1->second)^parameter2 ;
    square_union(res,next,v1->first);
  }

  GSDD::Valuation value;
  std::map<GSDD,DataSet *>::iterator nullmap = res.find(GSDD::null);
  if (nullmap != res.end()) {
    delete nullmap->second;
    res.erase(nullmap);
  }

  value.reserve(res.size());  
  for (std::map<GSDD,DataSet *>::iterator it =res.begin() ;it!= res.end();++it)
    value.push_back(std::make_pair(it->second,it->first));
 
  return GSDD(variable,value);
};

/* constructor*/
GSDD _SDED_Concat::create(const GSDD &g1,const GSDD &g2){
  if(g1==GSDD::null||g2==GSDD::null)
    return GSDD::null;
  else if(g1==GSDD::one)
    return g2;
  else if(g1==GSDD::top)
    return GSDD::top;
  else
    return compute(_SDED_Concat(g1,g2));
};


/******************************************************************************/
/*                           class SDED                                        */
/******************************************************************************/

  /* Memory Manager */
unsigned int SDED::statistics() {
  return uniqueSDED.size();
}


void SDED::pstats(bool reinit)
{
  std::cout << "*\nCache Stats : size=" << statistics() << "   --- Peak size=" <<  namespace_SDED::Max_SDED << std::endl;
  
  
  std::cout << "Cache hit ratio : " << double (namespace_SDED::Hits*100) / double(namespace_SDED::Misses+1+namespace_SDED::Hits) << "% ("<< namespace_SDED::Hits << "/" <<namespace_SDED::Misses << ")" << std::endl;
  // long hitr=(Hits*100) / (Misses+1+Hits) ;
  if (reinit){
    namespace_SDED::Hits =0;
    namespace_SDED::Misses =0;  
  }  

}


size_t SDED::peak() {
  if (uniqueSDED.size() > namespace_SDED::Max_SDED)
    namespace_SDED::Max_SDED = uniqueSDED.size();
  return namespace_SDED::Max_SDED;
}

void SDED::garbage(){
  if (uniqueSDED.size() > namespace_SDED::Max_SDED)
    {
      namespace_SDED::Max_SDED= uniqueSDED.size();
    }
  for (auto ded : uniqueSDED.table ){
    delete ded;
  }
  uniqueSDED.table.clear();
}; 

/* binary operators */

GSDD SDED::add(const d3::set<GSDD>::type &s){
  if (s.empty()) {
    return GSDD::null;
  } else if (s.size() == 1) {
    return *s.begin();
  } else {
    return _SDED_Add::create(s);
  }
};

GSDD operator+(const GSDD &g1,const GSDD &g2){
  return _SDED_Add::create(g1,g2);
};

GSDD operator*(const GSDD &g1,const GSDD &g2){
  return _SDED_Mult::create(g1,g2);
};

GSDD operator^(const GSDD &g1,const GSDD &g2){
  return _SDED_Concat::create(g1,g2);
};

GSDD operator-(const GSDD &g1,const GSDD &g2){
  return _SDED_Minus::create(g1,g2);
};

/******************************************************************************/


