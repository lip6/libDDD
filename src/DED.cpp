
/* -*- C++ -*- */
#include <set>
#include <iostream>
#include <map>
// modif
#include <assert.h>
#include <ext/hash_map>
#include <typeinfo>
// ajout
using namespace std;
using namespace __gnu_cxx;

#include "DDD.h"
#include "DED.h"
#include "Hom.h"

/******************************************************************************/


#ifdef INST_STL
long long NBJumps=0;
long long NBAccess=0;
#endif

typedef hash_map<DED,GDDD> Cache;
static Cache cache;

static int Hits=0;
static int Misses=0;

/******************************************************************************/
class _DED_GDDD:public _DED{
private:
  GDDD parameter;
public:
  _DED_GDDD(const GDDD& g):parameter(g){};
  size_t hash()const {return 1433*::hash<GDDD>()(parameter);};
  bool  operator==(const _DED &e)const{
    return (parameter==((_DED_GDDD*)&e)->parameter);
  };
  GDDD eval() const{return parameter;};
};

/******************************************************************************/
/*                    class _DED_Add:public _DED                              */
/******************************************************************************/
class _DED_Add:public _DED{
private:
  set<GDDD> parameters;
  _DED_Add(const set<GDDD> &d):parameters(d){};
public:
  static  _DED *create(const set<GDDD> &d);
  /* Compare */
  size_t hash() const;
  bool operator==(const _DED &e)const;

  /* Transform */
  GDDD eval() const;

  /* constructor*/
  ~_DED_Add(){};
};

/*********/
/* Compare */
size_t _DED_Add::hash() const{
  size_t res=0;
  for(set<GDDD>::const_iterator si=parameters.begin();si!=parameters.end();si++){
    res+=::hash<GDDD>()(*si);
  }
  return res;
}

bool _DED_Add::operator==(const _DED &e)const{
  return (parameters==((_DED_Add*)&e)->parameters);
}

/* Transform */
GDDD _DED_Add::eval() const{
  assert(parameters.size()>1);
  int variable=parameters.begin()->variable();
  GDDD::Valuation value;
  map<int,set<GDDD> > map_set;
  
  for(set<GDDD>::const_iterator si=parameters.begin();si!=parameters.end();si++){
    for(GDDD::const_iterator vi=si->begin();vi!=si->end();vi++){
      map_set[vi->first].insert(vi->second);
    }
  }
  for(map<int,set<GDDD> >::const_iterator map_set_i=map_set.begin();map_set_i!=map_set.end();map_set_i++){
    assert(map_set_i->second.size()!=0);
    pair<int,GDDD> x;
    x.first=map_set_i->first;
    if(map_set_i->second.size()==1)
      x.second=*(map_set_i->second.begin());
    else
      x.second=DED::add(map_set_i->second);
    value.push_back(x);
  }
  return GDDD(variable,value);
};

/* constructor*/
_DED *_DED_Add::create(const set<GDDD> &s){
  assert(s.size()!=0); // s is not empty
  set<GDDD> parameters=s;
  parameters.erase(GDDD::null);
  if(parameters.size()==1)
    return new _DED_GDDD(*parameters.begin());  
  else { 
    if(parameters.size()==0)
      return new _DED_GDDD(GDDD::null);
    else if(parameters.find(GDDD::top)!=parameters.end()||parameters.find(GDDD::one)!=parameters.end()){ // 
      return new _DED_GDDD(GDDD::top);
    }
    else{ 
      set<GDDD>::const_iterator si=parameters.begin();
      int variable = si->variable();
      for(;(si!=parameters.end())?(variable == si->variable()):false;si++);
      if(si!=parameters.end())// s contains at least 2 GDDDs with different variables
	return new _DED_GDDD(GDDD::top);
      return new _DED_Add(parameters);    
    }
  }
};

/******************************************************************************/
/*                    class _DED_Mult:public _DED                             */
/******************************************************************************/
class _DED_Mult:public _DED{
private:
  GDDD parameter1;
  GDDD parameter2;
  _DED_Mult(const GDDD &g1,const GDDD &g2):parameter1(g1),parameter2(g2){};
public:
  static  _DED *create(const GDDD &g1,const GDDD &g2);
  /* Compare */
  size_t hash() const;
  bool operator==(const _DED &e)const;

  /* Transform */
  GDDD eval() const;

  /* constructor*/
  ~_DED_Mult(){};
};

/*********/
/* Compare */
size_t _DED_Mult::hash() const{
  return ::hash<GDDD>()(parameter1)+13*::hash<GDDD>()(parameter2);
};

bool _DED_Mult::operator==(const _DED &e)const{
  return ((parameter1==((_DED_Mult*)&e)->parameter1)&&(parameter2==((_DED_Mult*)&e)->parameter2));
};

/* Transform */
GDDD _DED_Mult::eval() const{
  assert(parameter1.variable()==parameter2.variable());
  int variable=parameter1.variable();
  GDDD::Valuation value;

  map<int,set<GDDD> > map_set;
  GDDD::const_iterator v1=parameter1.begin();
  GDDD::const_iterator v2=parameter2.begin();
  while(v1!=parameter1.end()&&v2!=parameter2.end()){
    if(v1->first<v2->first)
      v1++;
    else if(v1->first>v2->first)
      v2++;
    else{
      GDDD g=(v1->second)*(v2->second);
      if(g!=GDDD::null){
	pair<int,GDDD> x;
	x.first=v1->first;
	x.second=g;
	value.push_back(x);
      }
      v1++;
      v2++;
    }
  }
  return GDDD(variable,value);
};

/* constructor*/
_DED *_DED_Mult::create(const GDDD &g1,const GDDD &g2){
  if(g1==g2)
    return new _DED_GDDD(g1);
  else if(g1==GDDD::null||g2==GDDD::null)
    return new _DED_GDDD(GDDD::null);
  else if(g1==GDDD::one||g2==GDDD::one||g1==GDDD::top||g2==GDDD::top)
    return new _DED_GDDD(GDDD::top);
  else if(g1.variable()!=g2.variable())
    return new _DED_GDDD(GDDD::null);
  else if(::hash<GDDD>()(g1) < ::hash<GDDD>()(g2))
    return new _DED_Mult(g1,g2);
  else
    return new _DED_Mult(g2,g1);
};


/******************************************************************************/
/*                    class _DED_Minus:public _DED                            */
/******************************************************************************/
class _DED_Minus:public _DED{
private:
  GDDD parameter1;
  GDDD parameter2;
  _DED_Minus(const GDDD &g1,const GDDD &g2):parameter1(g1),parameter2(g2){};
public:
  static  _DED *create(const GDDD &g1,const GDDD &g2);
  /* Compare */
  size_t hash() const;
  bool operator==(const _DED &e)const;

  /* Transform */
  GDDD eval() const;

  /* constructor*/
  ~_DED_Minus(){};
};

/*********/
/* Compare */
size_t _DED_Minus::hash() const{
  return 617*::hash<GDDD>()(parameter1)+307*::hash<GDDD>()(parameter2);
};

bool _DED_Minus::operator==(const _DED &e)const{
  return ((parameter1==((_DED_Minus*)&e)->parameter1)&&(parameter2==((_DED_Minus*)&e)->parameter2));
};

/* Transform */
GDDD _DED_Minus::eval() const{
  assert(parameter1.variable()==parameter2.variable());
  int variable=parameter1.variable();
  GDDD::Valuation value;

  //  map<int,set<GDDD> > map_set;
  GDDD::const_iterator v1=parameter1.begin();
  GDDD::const_iterator v2=parameter2.begin();
  while(v1!=parameter1.end()&&v2!=parameter2.end()){
    if(v1->first<v2->first){
      pair<int,GDDD> x(v1->first,v1->second);
      value.push_back(x);
      v1++;
    }
    else if(v1->first>v2->first)
      v2++;
    else{
      GDDD g=(v1->second)-(v2->second);
      if(g!=GDDD::null){
	pair<int,GDDD> x(v1->first,g);
	value.push_back(x);
      }
      v1++;
      v2++;
    }
  }

  for(;v1!=parameter1.end();v1++){
      pair<int,GDDD> x(v1->first,v1->second);
      value.push_back(x);
  }

  return GDDD(variable,value);
};

/* constructor*/
_DED *_DED_Minus::create(const GDDD &g1,const GDDD &g2){
  if(g1==g2||g1==GDDD::null)
    return new _DED_GDDD(GDDD::null);
  else if(g2==GDDD::null)
    return new _DED_GDDD(g1);
  else if(g1==GDDD::one||g2==GDDD::one||g1==GDDD::top||g2==GDDD::top)
    return new _DED_GDDD(GDDD::top);
  else if(g1.variable()!=g2.variable())
    return new _DED_GDDD(g1);
  else
    return new _DED_Minus(g1,g2);
};

/******************************************************************************/
/*                    class _DED_Concat:public _DED                           */
/******************************************************************************/
class _DED_Concat:public _DED{
private:
  GDDD parameter1;
  GDDD parameter2;
  _DED_Concat(const GDDD &g1,const GDDD &g2):parameter1(g1),parameter2(g2){};
public:
  static _DED *create(const GDDD &g1,const GDDD &g2);
  /* Compare */
  size_t hash() const;
  bool operator==(const _DED &e)const;

  /* Transform */
  GDDD eval() const;

  /* constructor*/
  ~_DED_Concat(){};
};

/*********/
/* Compare */
size_t _DED_Concat::hash() const{
  return 827*::hash<GDDD>()(parameter1)+1153*::hash<GDDD>()(parameter2);
};

bool _DED_Concat::operator==(const _DED &e)const{
  return ((parameter1==((_DED_Concat*)&e)->parameter1)&&(parameter2==((_DED_Concat*)&e)->parameter2));
};

/* Transform */
GDDD _DED_Concat::eval() const{
  int variable=parameter1.variable();
  GDDD::Valuation value;
  map<int,set<GDDD> > map_set;
  for(GDDD::const_iterator v1=parameter1.begin();v1!=parameter1.end();v1++){
    pair<int,GDDD> x(v1->first,(v1->second)^parameter2);
    value.push_back(x);
  }
  return GDDD(variable,value);
};

/* constructor*/
_DED * _DED_Concat::create(const GDDD &g1,const GDDD &g2){
  if(g1==GDDD::null||g2==GDDD::null)
    return new _DED_GDDD(GDDD::null);
  else if(g1==GDDD::one)
    return new _DED_GDDD(g2);
  else if(g1==GDDD::top)
    return new _DED_GDDD(GDDD::top);
  else
    return new _DED_Concat(g1,g2);
};

/******************************************************************************/
/*                    class _DED_Concat:public _DED                           */
/******************************************************************************/
class _DED_Hom:public _DED{
private:
  GHom hom;
  GDDD parameter;
  _DED_Hom(const GHom &h,const GDDD &d):hom(h),parameter(d){};
public: 
  static _DED *create(const GHom &h,const GDDD &d);

//  virtual bool shouldCache() { return /*parameter.refCounter()>1 ||*/ parameter.nbsons() > 1 ; }

  /* Compare */
  size_t hash() const;
  bool operator==(const _DED &e)const;

  /* Transform */
  GDDD eval() const;

  /* constructor*/
  ~_DED_Hom(){};
};

/*********/
/* Compare */
size_t _DED_Hom::hash() const{
  return 1451*::hash<GHom>()(hom)+1399*::hash<GDDD>()(parameter);
}

bool _DED_Hom::operator==(const _DED &e)const{
  return ((hom==((_DED_Hom*)&e)->hom)&&(parameter==((_DED_Hom*)&e)->parameter));
}

/* Transform */
GDDD _DED_Hom::eval() const{
  return hom.eval(parameter);
}

/* constructor*/
_DED * _DED_Hom::create(const GHom &g,const GDDD &d){
  if(d==GDDD::null)
    return new _DED_GDDD(GDDD::null);
  else 
    return new _DED_Hom(g,d);
}

/******************************************************************************/
/*                           class DED                                        */
/******************************************************************************/


  /* Memory Manager */
unsigned int DED::statistics() {
  return cache.size();
}

void DED::pstats(bool reinit)
{
  cout << "*\nCache Stats : size=" << cache.size() << endl;
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



// Todo
void DED::garbage(){
  for(Cache::iterator di=cache.begin();di!=cache.end();){
      Cache::iterator ci=di;
      di++;
      _DED *d=ci->first.concret;
      cache.erase(ci);
      delete d;
  } 
  //cache.clear();
}; 


bool DED::operator==(const DED& e) const{
  if(typeid(*concret)!=typeid(*(e.concret)))
  return false;
  else 
  return (*concret==*(e.concret));
};

// eval and set to NULL the DED
GDDD DED::eval(){
  if(typeid(*concret)==typeid(_DED_GDDD)){
    GDDD res=concret->eval();
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
    GDDD res=concret->eval(); // compute the result
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

GDDD DED::hom(const GHom &h,const GDDD&g){
  DED e(_DED_Hom::create(h,g));
  return e.eval();
};

GDDD DED::add(const set<GDDD> &s){
  DED e(_DED_Add::create(s));
  return e.eval();
};

GDDD operator+(const GDDD &g1,const GDDD &g2){
  set<GDDD> s;
  s.insert(g1);
  s.insert(g2);
  return DED::add(s);
};

GDDD operator*(const GDDD &g1,const GDDD &g2){
  DED e(_DED_Mult::create(g1,g2));
  return e.eval();
};

GDDD operator^(const GDDD &g1,const GDDD &g2){
  DED e(_DED_Concat::create(g1,g2));
  return e.eval();
};

GDDD operator-(const GDDD &g1,const GDDD &g2){
  DED e(_DED_Minus::create(g1,g2));
  return e.eval();
};

/******************************************************************************/

size_t hash<DED>::operator()(const DED &e) const{
  return e.concret->hash();
};

bool equal_to<DED>::operator()(const DED &e1,const DED &e2) const{
  return e1==e2;
};
