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

static int Hits=0;
static int Misses=0;
static size_t Max_SDED=0;
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
  GSDD eval() const{return parameter;};
};

/******************************************************************************/
/*                    class _SDED_Add:public _SDED                              */
/******************************************************************************/
class _SDED_Add:public _SDED{
private:
  GSDD parameter1;
  GSDD parameter2;
  _SDED_Add(const GSDD &g1,const GSDD &g2):parameter1(g1),parameter2(g2){};
public:
  static  _SDED *create(const GSDD &g1,const GSDD &g2);
  /* Compare */
  size_t hash() const;
  bool operator==(const _SDED &e)const;

  /* Transform */
  GSDD eval() const;

  /* constructor*/
  ~_SDED_Add(){};
};

/*********/
/* Compare */
size_t _SDED_Add::hash() const{
  return ::hash<GSDD>()(parameter1)^::hash<GSDD>()(parameter2);
}

bool _SDED_Add::operator==(const _SDED &e)const{
  return  ((parameter1==((_SDED_Add*)&e)->parameter1)&&(parameter2==((_SDED_Add*)&e)->parameter2));
}

/* Transform */
GSDD _SDED_Add::eval() const{
  assert(parameter1.variable()==parameter2.variable());
  int variable=parameter1.variable();
  map<GSDD,DataSet *> res;

  // remainder for p1 and p2
  map<GSDD,DataSet *> rem_p1,rem_p2;


  // for each son of p1 initialize remainder
  for (GSDD::Valuation::const_iterator it = parameter1.begin();it != parameter1.end() ; it++) 
    rem_p1[it->second] = it->first;
  // for each son of p2 initialize remainder
  for (GSDD::Valuation::const_iterator it = parameter2.begin();it != parameter2.end() ; it++) 
    rem_p2[it->second] = it->first;

  GSDD s1Us2 ;
  // for each son of p1 :   v - a -> s1 
  for (GSDD::Valuation::const_iterator it = parameter1.begin();it != parameter1.end() ; it++) {
    // for each son of p2 :   v - b -> s2 
    for (GSDD::Valuation::const_iterator jt = parameter2.begin();jt != parameter2.end() ; jt++) {
      // compute a*b
      DataSet *ainterb = it->first->set_intersect(*jt->first);
      // if a*b = 0, skip
      if (ainterb->empty()) 
	continue;

      //   (a-c) -> s1  + (c-a) -> s2 + (c*a) -> s1+s2
      // treat (c*a) -> s1+s2
      s1Us2 = it->second + jt->second;
      map<GSDD,DataSet *>::iterator kt = res.find(s1Us2);
      if (kt != res.end())
	kt->second  = kt->second->set_union(*ainterb);
      else
	res[s1Us2] = ainterb;
      // treat other terms
      rem_p1[it->second] = rem_p1[it->second]->set_minus(*ainterb);
      rem_p2[jt->second] = rem_p2[jt->second]->set_minus(*ainterb);
    }
  }
  
  // add remainders
  // for each son of p1 
  for (map<GSDD,DataSet *>::const_iterator it = rem_p1.begin();it != rem_p1.end() ; it++) 
    if (! it->second->empty() )      
      {
	map<GSDD,DataSet *>::iterator kt = res.find(it->first);
	if (kt != res.end())
	  kt->second  = kt->second->set_union(*it->second);
	else
	  res[it->first] = it->second ;
      }
  // for each son of p2 
  for (map<GSDD,DataSet *>::const_iterator it = rem_p2.begin();it != rem_p2.end() ; it++) 
    if (! it->second->empty() )      
      {
	map<GSDD,DataSet *>::iterator kt = res.find(it->first);
	if (kt != res.end())
	  kt->second  = kt->second->set_union(*it->second);
	else
	  res[it->first] = it->second ;
      }

  GSDD::Valuation value;
  map<GSDD,DataSet *>::iterator nullmap = res.find(GSDD::null);
  if (nullmap != res.end())
    res.erase(nullmap);
  value.reserve(res.size());  
  for (map<GSDD,DataSet *>::iterator it =res.begin() ;it!= res.end();it++)
    value.push_back(make_pair(it->second,it->first));

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

  if (::hash<GSDD>()(g1) < ::hash<GSDD>()(g2))
    return new _SDED_Add(g1,g2);
  else
    return new _SDED_Add(g2,g1);
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
      if (ainterb->empty() ) 
	continue;

      s1inters2 = it->second * jt->second;
      map<GSDD,DataSet *>::iterator kt = res.find(s1inters2);
      if (kt != res.end())
	kt->second  = kt->second->set_union(*ainterb);
      else
	res[s1inters2] = ainterb;
    }
  }

  GSDD::Valuation value;
  map<GSDD,DataSet *>::iterator nullmap = res.find(GSDD::null);
  if (nullmap != res.end())
    res.erase(nullmap);
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
    rem_p1[it->second] = it->first;


  GSDD s1moinss2 ;
  // for each son of p1 :   v - a -> s1 
  for (GSDD::Valuation::const_iterator it = parameter1.begin();it != parameter1.end() ; it++) {
    // for each son of p2 :   v - b -> s2 
    for (GSDD::Valuation::const_iterator jt = parameter2.begin();jt != parameter2.end() ; jt++) {
      // compute a*b
      DataSet * ainterb = it->first->set_intersect(*jt->first);
      // if a*b = 0, skip
      if (ainterb->empty()) 
	continue;

      
      s1moinss2 = it->second - jt->second;
      map<GSDD,DataSet *>::iterator kt = res.find(s1moinss2);
      if (kt != res.end())
	kt->second  = kt->second->set_union(*ainterb);
      else
	res[s1moinss2] = ainterb;
      rem_p1[it->second] = rem_p1[it->second]->set_minus(*ainterb);
    }
  }
  // add remainders
  // for each son of p1 
  for (map<GSDD,DataSet *>::const_iterator it = rem_p1.begin();it != rem_p1.end() ; it++) 
    if (! it->second->empty() )      
      {
	map<GSDD,DataSet *>::iterator kt = res.find(it->first);
	if (kt != res.end())
	  kt->second  = kt->second->set_union(*it->second);
	else
	  res[it->first] = it->second ;
      }

  GSDD::Valuation value;
  map<GSDD,DataSet *>::iterator nullmap = res.find(GSDD::null);
  if (nullmap != res.end())
    res.erase(nullmap);
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
    kt = res.find(next);
    if (kt != res.end())
      kt->second  = kt->second->set_union(*v1->first);
    else 
      res[next] = v1->first;
  }

  GSDD::Valuation value;
  map<GSDD,DataSet *>::iterator nullmap = res.find(GSDD::null);
  if (nullmap != res.end())
    res.erase(nullmap);
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

  virtual bool dogarbage () { return shom.refCounter() <= 0 && parameter.refCounter() <= 0; }
  virtual bool shouldCache() { return parameter.refCounter()>1 || parameter.nbsons() > 1 ; }

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
  return shom.eval(parameter);
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


void SDED::garbage(){
  if (cache.size() > Max_SDED) 
    Max_SDED=cache.size();  
  for(Cache::iterator di=cache.begin();di!=cache.end();){
      Cache::iterator ci=di;
      di++;
      _SDED *d=ci->first.concret;
      cache.erase(ci);
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
//  else 
//     if (! concret->shouldCache() ){
//       // we don't need to store it
//       GDDD res=concret->eval(); // compute the result
      
//       delete concret;
//       Misses++;
//       return res;
//     }
#ifdef INST_STL
    NBAccess++;
    NBJumps++;
    int temp=0;
    //    Cache::const_iterator 
    Cache::const_iterator ci=cache.find(*this, temp); // search e in the cache
    NBJumps+=temp;
#else
  Cache::const_iterator ci=cache.find(*this); // search e in the cache
#endif

  if(ci==cache.end()){ // *this is not in the cache
    Misses++;
    GSDD res=concret->eval(); // compute the result
    cache[*this]=res;
    concret=NULL;
    return res;
  }
  else {// *this is in the cache
    Hits++;
    delete concret;
    return ci->second;
  }
};


/* binary operators */

GSDD SDED::Shom(const GShom &h,const GSDD&g){
  SDED e(_SDED_Shom::create(h,g));
  return e.eval();
};


// GSDD SDED::add(const set<GSDD> &s){
//   SDED e(_SDED_Add::create(s));
//   return e.eval();
// };

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
