/* -*- C++ -*- */
#include <set>
#include <map>
// modif
#include <ext/hash_map>
#include <typeinfo>
// ajout
#include <cassert>
using namespace std;
using namespace __gnu_cxx;

#include "DataSet.h"
#include "DED.h"
#include "SDD.h"
#include "SDED.h"
#include "SHom.h"

/******************************************************************************/
namespace namespace_SDED {
#ifdef INST_STL
  long long NBJumps=0;
  long long NBAccess=0;
#endif

  typedef hash_map<SDED,GSDD> Cache;
  static Cache cache;
  static Cache recentCache;
  
  static int Hits=0;
  static int Misses=0;
  static size_t Max_SDED=0;
  static size_t Max_Recent_SDED=0;
} //namespace namespace_SDED 
using namespace namespace_SDED ;

/******************************************************************************/
class _SDED_GSDD:public _SDED{
private:
  GSDD parameter;
public:
  _SDED_GSDD(const GSDD& g):parameter(g){};
  size_t hash()const {return 1433*::hash<GSDD>()(parameter);};
  bool  operator==(const _SDED &e)const{
    return (parameter==((_SDED_GSDD*)&e)->parameter);
  };

  bool AllParamsMarkAsRecent() const {return true;};
  void AllParamsUnmarkAsRecent() const {};

  GSDD eval() const{return parameter;};
};


/******************** BASIS FOR CANONIZATION OPERATIONS **********************/
inline void square_union (map<GSDD,DataSet *> &res,const GSDD & s, DataSet * d) {
  map<GSDD,DataSet *>::iterator kt = res.find(s);
  if (kt != res.end()) {
    /* found it in res compute union */
    DataSet * tofree = kt->second;
    kt->second  = kt->second->set_union(*d);
    delete tofree;
  } else {
    /* not yet in res, add it */
    res.insert(make_pair(s,d->newcopy()));
  }
}



/******************************************************************************/
/*                    class _SDED_Add:public _SDED                              */
/******************************************************************************/
class _SDED_Add:public _SDED{
private:
  set<GSDD> parameters;
  _SDED_Add(const set<GSDD> &d):parameters(d){};
public:
  static  _SDED *create(const GSDD &g1,const GSDD &g2);
  static  _SDED *create(const set<GSDD> &d);
  /* Compare */
  size_t hash() const;
  bool operator==(const _SDED &e)const;

  bool shouldCache () {
    for(set<GSDD>::const_iterator si=parameters.begin();si!=parameters.end();si++)
      if (! si->isSon()) 
	return false;
    return true;
  }

  /* Transform */
  GSDD eval() const;

  /* constructor*/
  ~_SDED_Add(){};
};

/*********/
/* Compare */
size_t _SDED_Add::hash() const{
  size_t res=0;
  for(set<GSDD>::const_iterator si=parameters.begin();si!=parameters.end();si++){
    res+=::hash<GSDD>()(*si);
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
  map<GSDD,DataSet *> res;

  // for memory collection
  DataSet * tofree;

  // The current operand
  set<GSDD>::const_iterator opit =  parameters.begin();

  // Initialize with copy of first operand
  for (GSDD::Valuation::const_iterator it = opit->begin();it != opit->end() ; it++) 
    res[it->second]=it->first->newcopy();

  // main loop
  // Foreach  opit in (operands)
  for (opit++ ; opit != parameters.end() ; opit++) {
    // To store non empty intersection results;
    vector< pair <GSDD,DataSet *> > sums;
    // To store the remainders (empty intersection with all previous elements)
    vector< pair <GSDD,DataSet *> > rems;
    
    // Foreach arc in current operand  : e-a->A
    for (GSDD::Valuation::const_iterator arc = opit->begin() ; arc != opit->end() ; arc++ ) {
      DataSet * a = arc->first->newcopy();
      // foreach value already in result : e-b->B
      for (map<GSDD,DataSet *>::iterator resit = res.begin() ; resit != res.end() ; resit ++ ) {
	// compute a*b
	DataSet *ainterb = arc->first->set_intersect(*resit->second);
	// if a*b = 0, skip
	if (ainterb->empty()) {
	  delete ainterb;
	  continue;
	}
	// update result : res[cur] -= ainterb :  e- b \ a -> B
	tofree = resit->second;
	resit->second = resit->second->set_minus(*ainterb);
	delete tofree;
	// add intersection to sums : e- b * a -> A+B
	sums.push_back( make_pair(resit->first + arc->second , ainterb) );
	// update current iteration value
	tofree = a ;
	a = a->set_minus(*ainterb);
	delete tofree;
	if (a->empty()) {
	  // we can stop
	  break;
	}
	
      } // end foreach resit in result
      
      // if there is a remainder, store it
      if (! a->empty()) {
	rems.push_back(make_pair(arc->second,a));
      } else 
	delete a;
    } // end foreach arc in operand
    
    // Now process remainders and sums
    for (vector< pair <GSDD,DataSet *> >::iterator it=sums.begin(); it != sums.end(); it++ ) {
      square_union(res,it->first,it->second);
      delete it->second;
    }
    for (vector< pair <GSDD,DataSet *> >::iterator it=rems.begin(); it != rems.end(); it++ ) {
      square_union(res,it->first,it->second);
      delete it->second;
    }

//     if ( ! opit->isSon() && ! opit->refCounter() )
//       opit->clearNode();
  } // end foreach operand

  GSDD::Valuation value;
  map<GSDD,DataSet *>::iterator nullmap = res.find(GSDD::null);
  if (nullmap != res.end()) {
    delete nullmap->second;
    res.erase(nullmap);
  }
  value.reserve(res.size());  
  for (map<GSDD,DataSet *>::iterator it =res.begin() ;it!= res.end();it++)
    if (! it->second->empty())
      value.push_back(make_pair(it->second,it->first));
    else
      delete  it->second;

  return GSDD(variable,value);
};

/* constructor*/
_SDED *_SDED_Add::create(const GSDD &g1,const GSDD &g2){
  if (g1 == g2)
    return new _SDED_GSDD(g1);
  
  if(g1 == GSDD::null)
    return new _SDED_GSDD(g2);
  if (g2 == GSDD::null)
    return new _SDED_GSDD(g1);

  if (g1 == GSDD::one || g2 == GSDD::one || g1 == GSDD::top || g2 == GSDD::top || g1.variable() != g2.variable() ) 
    return new _SDED_GSDD(GSDD::top);

  set<GSDD> parameters;
  parameters.insert(g1);
  parameters.insert(g2);

  return new _SDED_Add(parameters);
}
/* constructor*/
_SDED *_SDED_Add::create(const set<GSDD> &s){
  assert(s.size()!=0); // s is not empty
  set<GSDD> parameters=s;
  parameters.erase(GSDD::null);
  if(parameters.size()==1)
    return new _SDED_GSDD(*parameters.begin());  
  else { 
    if(parameters.size()==0)
      return new _SDED_GSDD(GSDD::null);
    else if(parameters.find(GSDD::top)!=parameters.end()||parameters.find(GSDD::one)!=parameters.end()){  
      return new _SDED_GSDD(GSDD::top);
    }
    else{ 
      set<GSDD>::const_iterator si=parameters.begin();
      int variable = si->variable();
      for(;(si!=parameters.end())?(variable == si->variable()):false;si++);
      if(si!=parameters.end())// s contains at least 2 GDDDs with different variables
	return new _SDED_GSDD(GSDD::top);
      return new _SDED_Add(parameters);    
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
  static  _SDED *create(const GSDD &g1,const GSDD &g2);
  /* Compare */
  size_t hash() const;
  bool operator==(const _SDED &e)const;

  bool shouldCache () {
    return  ( parameter1.isSon() &&  parameter2.isSon());
  }

  /* Transform */
  GSDD eval() const;

  /* constructor*/
  ~_SDED_Mult(){};
};

/*********/
/* Compare */
size_t _SDED_Mult::hash() const{
  return ::hash<GSDD>()(parameter1)+13*::hash<GSDD>()(parameter2);
};

bool _SDED_Mult::operator==(const _SDED &e)const{
  return ((parameter1==((_SDED_Mult*)&e)->parameter1)&&(parameter2==((_SDED_Mult*)&e)->parameter2));
};

/* Transform */
GSDD _SDED_Mult::eval() const{
  assert(parameter1.variable()==parameter2.variable());
  int variable=parameter1.variable();
  map<GSDD,DataSet *> res;



  GSDD s1inters2 ;
  // for each son of p1 :   v - a -> s1 
  for (GSDD::Valuation::const_iterator it = parameter1.begin();it != parameter1.end() ; it++) {
    // for each son of p2 :   v - b -> s2 
    for (GSDD::Valuation::const_iterator jt = parameter2.begin();jt != parameter2.end() ; jt++) {
      // compute a*b
      DataSet *ainterb = it->first->set_intersect(*jt->first);
      // if a*b = 0, skip
      if (ainterb->empty() ) {
	delete ainterb;
	continue;
      }
      s1inters2 = it->second * jt->second;
      square_union(res,s1inters2,ainterb);
      delete ainterb;
    }
  }

  GSDD::Valuation value;
  map<GSDD,DataSet *>::iterator nullmap = res.find(GSDD::null);
  if (nullmap != res.end()){
    delete nullmap->second;
    res.erase(nullmap);
  }
  value.reserve(res.size());  
  for (map<GSDD,DataSet *>::iterator it =res.begin() ;it!= res.end();it++)
    value.push_back(make_pair(it->second,it->first));
  
  return GSDD(variable,value);
};

/* constructor*/
_SDED *_SDED_Mult::create(const GSDD &g1,const GSDD &g2){
  if(g1==g2)
    return new _SDED_GSDD(g1);
  else if(g1==GSDD::null||g2==GSDD::null)
    return new _SDED_GSDD(GSDD::null);
  else if(g1==GSDD::one||g2==GSDD::one||g1==GSDD::top||g2==GSDD::top)
    return new _SDED_GSDD(GSDD::top);
  else if(g1.variable()!=g2.variable())
    return new _SDED_GSDD(GSDD::null);
  else if(::hash<GSDD>()(g1) < ::hash<GSDD>()(g2))
    return new _SDED_Mult(g1,g2);
  else
    return new _SDED_Mult(g2,g1);
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
  static  _SDED *create(const GSDD &g1,const GSDD &g2);
  /* Compare */
  size_t hash() const;
  bool operator==(const _SDED &e)const;

  bool shouldCache () {
    return  ( parameter1.isSon() &&  parameter2.isSon());
  }

  /* Transform */
  GSDD eval() const;

  /* constructor*/
  ~_SDED_Minus(){};
};

/*********/
/* Compare */
size_t _SDED_Minus::hash() const{
  return 617*::hash<GSDD>()(parameter1)+307*::hash<GSDD>()(parameter2);
};

bool _SDED_Minus::operator==(const _SDED &e)const{
  return ((parameter1==((_SDED_Minus*)&e)->parameter1)&&(parameter2==((_SDED_Minus*)&e)->parameter2));
};

/* Transform */
GSDD _SDED_Minus::eval() const{
  assert(parameter1.variable()==parameter2.variable());
  int variable=parameter1.variable();

  map<GSDD,DataSet *> res;
  // remainder for p1 
  map<GSDD,DataSet *> rem_p1;

  // for each son of p1 initialize remainder
  for (GSDD::Valuation::const_iterator it = parameter1.begin();it != parameter1.end() ; it++) 
    rem_p1[it->second] = it->first->newcopy();


  GSDD s1moinss2 ;
  // for each son of p1 :   v - a -> s1 
  for (GSDD::Valuation::const_iterator it = parameter1.begin();it != parameter1.end() ; it++) {
    // for each son of p2 :   v - b -> s2 
    for (GSDD::Valuation::const_iterator jt = parameter2.begin();jt != parameter2.end() ; jt++) {
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
  for (map<GSDD,DataSet *>::const_iterator it = rem_p1.begin();it != rem_p1.end() ; it++) {
    if (! it->second->empty() )      
      {
	square_union(res,it->first,it->second);
      }
    delete it->second;
  }

  GSDD::Valuation value;
  map<GSDD,DataSet *>::iterator nullmap = res.find(GSDD::null);
  if (nullmap != res.end()) {
    delete nullmap->second;
    res.erase(nullmap);
  }
  value.reserve(res.size());  
  for (map<GSDD,DataSet *>::iterator it =res.begin() ;it!= res.end();it++)
    value.push_back(make_pair(it->second,it->first));
 
  return GSDD(variable,value);
};

/* constructor*/
_SDED *_SDED_Minus::create(const GSDD &g1,const GSDD &g2){
  if(g1==g2||g1==GSDD::null)
    return new _SDED_GSDD(GSDD::null);
  else if(g2==GSDD::null)
    return new _SDED_GSDD(g1);
  else if(g1==GSDD::one||g2==GSDD::one||g1==GSDD::top||g2==GSDD::top)
    return new _SDED_GSDD(GSDD::top);
  else if(g1.variable()!=g2.variable())
    return new _SDED_GSDD(g1);
  else
    return new _SDED_Minus(g1,g2);
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
  static _SDED *create(const GSDD &g1,const GSDD &g2);
  /* Compare */
  size_t hash() const;
  bool operator==(const _SDED &e)const;

  bool shouldCache () {
    return  ( parameter1.isSon() &&  parameter2.isSon()) ;
  }

  /* Transform */
  GSDD eval() const;

  /* constructor*/
  ~_SDED_Concat(){};
};

/*********/
/* Compare */
size_t _SDED_Concat::hash() const{
  return 827*::hash<GSDD>()(parameter1)+1153*::hash<GSDD>()(parameter2);
};

bool _SDED_Concat::operator==(const _SDED &e)const{
  return ((parameter1==((_SDED_Concat*)&e)->parameter1)&&(parameter2==((_SDED_Concat*)&e)->parameter2));
};

/* Transform */
GSDD _SDED_Concat::eval() const{
  int variable=parameter1.variable();

  map<GSDD,DataSet *> res;
  GSDD next;
  map<GSDD,DataSet *>::iterator kt;

  for(GSDD::const_iterator v1=parameter1.begin();v1!=parameter1.end();v1++){
    next = (v1->second)^parameter2 ;
    square_union(res,next,v1->first);
  }

  GSDD::Valuation value;
  map<GSDD,DataSet *>::iterator nullmap = res.find(GSDD::null);
  if (nullmap != res.end()) {
    delete nullmap->second;
    res.erase(nullmap);
  }
  value.reserve(res.size());  
  for (map<GSDD,DataSet *>::iterator it =res.begin() ;it!= res.end();it++)
    value.push_back(make_pair(it->second,it->first));
 
  return GSDD(variable,value);
};

/* constructor*/
_SDED * _SDED_Concat::create(const GSDD &g1,const GSDD &g2){
  if(g1==GSDD::null||g2==GSDD::null)
    return new _SDED_GSDD(GSDD::null);
  else if(g1==GSDD::one)
    return new _SDED_GSDD(g2);
  else if(g1==GSDD::top)
    return new _SDED_GSDD(GSDD::top);
  else
    return new _SDED_Concat(g1,g2);
};

/******************************************************************************/
/*                    class _Shom_Concat:public _SDED                           */
/******************************************************************************/
class _SDED_Shom:public _SDED{
private:
  GShom shom;
  GSDD parameter;
  _SDED_Shom(const GShom &h,const GSDD &d):shom(h),parameter(d){};
public: 
  static _SDED *create(const GShom &h,const GSDD &d);

  virtual bool shouldCache() { return parameter.isSon()  ; }

  /* Compare */
  size_t hash() const;
  bool operator==(const _SDED &e)const;

  /* Transform */
  GSDD eval() const;

  /* constructor*/
  ~_SDED_Shom(){};
};

/*********/
/* Compare */
size_t _SDED_Shom::hash() const{
  return 1451*::hash<GShom>()(shom)+1399*::hash<GSDD>()(parameter);
}

bool _SDED_Shom::operator==(const _SDED &e)const{
  return ((shom==((_SDED_Shom*)&e)->shom)&&(parameter==((_SDED_Shom*)&e)->parameter));
}

/* Transform */
GSDD _SDED_Shom::eval() const{
  GSDD res = shom.eval(parameter);
  return  res;
}

/* constructor*/
_SDED * _SDED_Shom::create(const GShom &g,const GSDD &d){
  if(d==GSDD::null)
    return new _SDED_GSDD(GSDD::null);
  else 
    return new _SDED_Shom(g,d);
}

/******************************************************************************/
/*                           class SDED                                        */
/******************************************************************************/

  /* Memory Manager */
unsigned int SDED::statistics() {
  return cache.size();
}


void SDED::pstats(bool reinit)
{
  cout << "*\nCache Stats : size=" << cache.size() << "   --- Peak size=" <<  Max_SDED << endl;
  
#ifdef INST_STL
  cout << "nb jump in hash table : " << NBJumps << "/" << "nbsearch " ;
  cout << NBAccess << "=" << double (NBJumps)/double(NBAccess)<< endl;
  if (reinit){
    NBAccess=0;
    NBJumps=0;
  }
#endif
  
  cout << "\nCache hit ratio : " << double (Hits*100) / double(Misses+1+Hits) << "%" << endl;
  // long hitr=(Hits*100) / (Misses+1+Hits) ;
  if (reinit){
    Hits =0;
    Misses =0;  
  }  

}

static unsigned int  recentLimit = 1024 ;
void SDED::recentGarbage(){
  size_t rcSize=recentCache.size();
  int longTerm = 0;
  int flSize ;
  if (rcSize > recentLimit && rcSize > 2*cache.size()) {
    flSize = cache.size();
    if (recentCache.size() > Max_Recent_SDED) 
      Max_Recent_SDED=recentCache.size();  
    for(Cache::iterator di=recentCache.begin();di!=recentCache.end();){
      Cache::iterator ci=di;
      di++;
      _SDED *d=ci->first.concret;
      if (d->shouldCache() && ci->second.isSon()) {
	cache.insert(*ci);
	recentCache.erase(ci);
	longTerm++;
      }      else {
	recentCache.erase(ci);
	delete d;
      }
    }
    if ( longTerm < (flSize / 20) )
      recentLimit*=2;
    cerr << " Recent SDED cache size was " << rcSize << "/"<< recentLimit<<" Full : "<< cache.size() << "  comitted " << longTerm << " results to storage "<<endl;
    SDDutil::recentGarbage();
  }
};



void SDED::garbage(){
  recentGarbage();
  if (cache.size() > Max_SDED) 
    Max_SDED=cache.size();  
  for(Cache::iterator di=cache.begin();di!=cache.end();){
      Cache::iterator ci=di;
      di++;
      _SDED *d=ci->first.concret;
      cache.erase(ci);
      delete d;
  } 
  for(Cache::iterator di=recentCache.begin();di!=recentCache.end();){
      Cache::iterator ci=di;
      di++;
      _SDED *d=ci->first.concret;
      recentCache.erase(ci);
      delete d;
  } 
  
//  cache.clear();

}; 

bool SDED::operator==(const SDED& e) const{
  if(typeid(*concret)!=typeid(*(e.concret)))
  return false;
  else 
  return (*concret==*(e.concret));
};

// eval and set to NULL the DED
GSDD SDED::eval(){
  if(typeid(*concret)==typeid(_SDED_GSDD)){
    GSDD res=concret->eval();
    delete concret;
    return res;
  }
  else {
#ifdef INST_STL
    NBAccess++;
    NBJumps++;
    int temp=0;
    //    Cache::const_iterator 
    Cache::const_iterator ci=recentCache.find(*this, temp); // search e in the recent storage cache
    NBJumps+=temp;
#else
    Cache::const_iterator ci=recentCache.find(*this); // search e in the recent storage cache
#endif
    if (ci==recentCache.end()){ // *this is not in the recent storage cache
      // test if parameters potentially allow long term storage
      if ( concret->shouldCache() ){
	// search in long term cache
#ifdef INST_STL
	NBAccess++;
	NBJumps++;
	temp=0;
	//    Cache::const_iterator 
	ci=cache.find(*this, temp); // search e in the long term storage cache
	NBJumps+=temp;
#else
	ci=cache.find(*this); // search e in the long term storage cache
#endif
	if (ci==cache.end()){ // *this is not in the long term storage cache
	  Misses++;  // this constitutes a cache miss (double truly) !!
	  GSDD res=concret->eval(); // compute the result
	  // test if result is eligible for long term storage status
 	  if ( ! res.isSon() ) {
	    // Not eligible
	    recentCache[*this]=res;
 	  } else {
 	    // eligible
 	    cache[*this]=res;
	    // Should we quick Garbage HERE ???
	    recentGarbage();
 	  }
	  concret=NULL;
	  return res;
	} else {
	  // found in long term cache
	  Hits++;
	  delete concret;
	  return ci->second;
	}

      } else { // parameters make Shom ineligible for long term storage	
	
	Misses++;  // this constitutes a cache miss (simple) !!
	GSDD res=concret->eval(); // compute the result
	recentCache[*this]=res;	
	concret=NULL;
	return res;
     }
  } else {// *this is in the cache
    Hits++;
    delete concret;
    return ci->second;
  }
  } // end else : not a constant GSDD
};


/* binary operators */

GSDD SDED::Shom(const GShom &h,const GSDD&g){
  SDED e(_SDED_Shom::create(h,g));
  return e.eval();
};


GSDD SDED::add(const set<GSDD> &s){
   SDED e(_SDED_Add::create(s));
   return e.eval();
};

GSDD operator+(const GSDD &g1,const GSDD &g2){
  SDED e(_SDED_Add::create(g1,g2));
  return e.eval();
};

GSDD operator*(const GSDD &g1,const GSDD &g2){
  SDED e(_SDED_Mult::create(g1,g2));
  return e.eval();
};

GSDD operator^(const GSDD &g1,const GSDD &g2){
  SDED e(_SDED_Concat::create(g1,g2));
  return e.eval();
};

GSDD operator-(const GSDD &g1,const GSDD &g2){
  SDED e(_SDED_Minus::create(g1,g2));
  return e.eval();
};

/******************************************************************************/

size_t hash<SDED>::operator()(const SDED &e) const{
  return e.concret->hash();
};

bool equal_to<SDED>::operator()(const SDED &e1,const SDED &e2) const{
  return e1==e2;
};
