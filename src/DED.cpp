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
#include <iostream>
#include <map>
// modif
#include <assert.h>
//#include <ext/hash_map>
#include <typeinfo>
// ajout

#include "DDD.h"
#include "DED.h"
#include "Hom.h"

#include "Cache.h"

#ifdef PARALLEL_DD
#include "tbb/atomic.h"
#endif
/******************************************************************************/

//typedef __gnu_cxx::hash_map<DED,GDDD> Cache;
typedef __Cache<DED,GDDD> Cache;
static Cache cache;

#ifdef PARALLEL_DD

static tbb::atomic<int> Hits;
static tbb::atomic<int> Misses;
class DED_parallel_init
{
public:
	 
	DED_parallel_init()
	{
		Hits = 0;
		Misses = 0;
	}
		
};
static DED_parallel_init DED_init;

#else
static int Hits=0;
static int Misses=0;

#endif

/******************************************************************************/
class _DED_GDDD:public _DED{
private:
  GDDD parameter;
public:
  _DED_GDDD(const GDDD& g):parameter(g){};
  size_t hash()const {return 1433* parameter.hash();};
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
  std::set<GDDD> parameters;
  _DED_Add(const std::set<GDDD> &d):parameters(d){};
public:
  static  _DED *create(const std::set<GDDD> &d);
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
  for(std::set<GDDD>::const_iterator si=parameters.begin();si!=parameters.end();++si){
    res+= si->hash();
  }
  return res;
}

bool _DED_Add::operator==(const _DED &e)const{
  return (parameters==((_DED_Add*)&e)->parameters);
}

#ifdef EVDDD
/// increment the value of the next variable
class _pushEVDDD:public StrongHom {
  int dist;
public:
  _pushEVDDD(int dist_) :dist(dist_){}

  GDDD phiOne() const {
    return GDDD::one;
  }                   

  GHom phi(int vr, int vl) const {
    if (vr == DISTANCE)
      return GHom(vr,vl+dist);
    else 
      return GHom(vr,vl,this);
  }

  size_t hash() const {
    return 11213*dist;
  }

  bool operator==(const StrongHom &s) const {
    return dist == ((const _pushEVDDD &)s).dist;
  }
};
/// User function : Construct a Hom for a Strong Hom _plusplus
GHom pushEVDDD(int v){return new _pushEVDDD(v);};
#endif


/* Transform */
GDDD _DED_Add::eval() const{
  assert(parameters.size()>1);
  int variable=parameters.begin()->variable();

  GDDD::Valuation value;
  std::map<int,std::set<GDDD> > map_set;

#ifdef EVDDD
  if (variable == DISTANCE) {
    /// Special distance node canonization
    int min = -1;
    std::set<GDDD> succSet;
    // gather min
    for(std::set<GDDD>::const_iterator si=parameters.begin();si!=parameters.end();++si){
      for(GDDD::const_iterator vi=si->begin();vi!=si->end();++vi){
	if (min == -1 || min > vi->first)
	  min = vi->first;
      }
    }
    for(std::set<GDDD>::const_iterator si=parameters.begin();si!=parameters.end();++si){
      for(GDDD::const_iterator vi=si->begin();vi!=si->end();++vi){
	if (vi->first == min)
	  succSet.insert(vi->second);
	else
	  succSet.insert(pushEVDDD(vi->first - min)(vi->second));
      }
    }
    GDDD succ = DED::add(succSet);
//     if (succ != GDDD::one) {
//       int minsucc=-1;
//       for (GDDD::const_iterator it = succ.begin() ; it != succ.end() ; ++it) {
// 	assert (it->second.nbsons() == 1);
// 	GDDD::const_iterator succd = it->second.begin();
// 	if (minsucc==-1 || succd->first < minsucc)
// 	  minsucc = succd->first;
//       }
//       if (minsucc != 0) {
// 	min += minsucc;
// 	succ = push (-minsucc) (succ);
//       }
//     }
    return GDDD (variable,min,succ);
  } else {
    /// normal node canonization
#endif  
  for(std::set<GDDD>::const_iterator si=parameters.begin();si!=parameters.end();++si){
    for(GDDD::const_iterator vi=si->begin();vi!=si->end();++vi){
      map_set[vi->first].insert(vi->second);
    }
  }
#ifdef EVDDD
  }
#endif
  for(std::map<int,std::set<GDDD> >::const_iterator map_set_i=map_set.begin();map_set_i!=map_set.end();++map_set_i){
    assert(map_set_i->second.size()!=0);
    std::pair<int,GDDD> x;
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
_DED *_DED_Add::create(const std::set<GDDD> &s){
  assert(s.size()!=0); // s is not empty
  std::set<GDDD> parameters=s;
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
      std::set<GDDD>::const_iterator si=parameters.begin();
      int variable = si->variable();
      for(;(si!=parameters.end())?(variable == si->variable()):false;++si);
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
  return parameter1.hash()+13*parameter2.hash();
};

bool _DED_Mult::operator==(const _DED &e)const{
  return ((parameter1==((_DED_Mult*)&e)->parameter1)&&(parameter2==((_DED_Mult*)&e)->parameter2));
};

/* Transform */
GDDD _DED_Mult::eval() const{
  assert(parameter1.variable()==parameter2.variable());
  int variable=parameter1.variable();
#ifdef EVDDD
  if (variable == DISTANCE) {
    assert(parameter1.nbsons()==1);
    assert(parameter2.nbsons()==1);
    GDDD::const_iterator vv1=parameter1.begin();
    GDDD::const_iterator vv2=parameter2.begin();
    int succval;
    GDDD succ;        
    if (vv1->first > vv2->first) {
      succval = vv2->first;
      succ = pushEVDDD(vv1->first - vv2->first) (vv1->second)  * vv2->second;
    } else if (vv1->first < vv2->first) {
      succval = vv1->first;
      succ = vv1->second * pushEVDDD(vv2->first - vv1->first)(vv2->second);
    } else {
      succval = vv1->first;
      succ = vv1->second * vv2->second;
    }
    
//     if (succ != GDDD::one) {
//       int minsucc=-1;
//       for (GDDD::const_iterator it = succ.begin() ; it != succ.end() ; ++it) {
// 	assert (it->second.nbsons() == 1);
// 	GDDD::const_iterator succd = it->second.begin();
// 	if (minsucc==-1 || succd->first < minsucc)
// 	  minsucc = succd->first;
//       }
//       if (minsucc != 0) {
// 	succval += minsucc;
// 	succ = push (-minsucc) (succ);
//       }
//     }
    
    return GDDD(variable,succval,succ);
  }
#endif

  GDDD::Valuation value;
  std::map<int,std::set<GDDD> > map_set;
  GDDD::const_iterator v1=parameter1.begin();
  GDDD::const_iterator v2=parameter2.begin();
  while(v1!=parameter1.end()&&v2!=parameter2.end()){
    if(v1->first<v2->first)
      ++v1;
    else if(v1->first>v2->first)
      ++v2;
    else{
      GDDD g=(v1->second)*(v2->second);
      if(g!=GDDD::null){
        std::pair<int,GDDD> x;
	x.first=v1->first;
	x.second=g;
	value.push_back(x);
      }
      ++v1;
      ++v2;
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
  else if(g1.hash() < g2.hash())
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
  return 617*parameter1.hash() +307*parameter2.hash();
};

bool _DED_Minus::operator==(const _DED &e)const{
  return ((parameter1==((_DED_Minus*)&e)->parameter1)&&(parameter2==((_DED_Minus*)&e)->parameter2));
};

/* Transform */
GDDD _DED_Minus::eval() const{
  assert(parameter1.variable()==parameter2.variable());
  int variable=parameter1.variable();
  GDDD::Valuation value;

  //  std::map<int,std::set<GDDD> > std::map_std::set;
  GDDD::const_iterator v1=parameter1.begin();
  GDDD::const_iterator v2=parameter2.begin();
#ifdef EVDDD
  if (variable == DISTANCE) {
    assert(parameter1.nbsons() == 1);
    assert(parameter2.nbsons() == 1);
    return GDDD(variable,v1->first,v1->second - v2->second);
  }
#endif
  while(v1!=parameter1.end()&&v2!=parameter2.end()){
    if(v1->first<v2->first){
      std::pair<int,GDDD> x(v1->first,v1->second);
      value.push_back(x);
      v1++;
    }
    else if(v1->first>v2->first)
      v2++;
    else{
      GDDD g=(v1->second)-(v2->second);
      if(g!=GDDD::null){
        std::pair<int,GDDD> x(v1->first,g);
	value.push_back(x);
      }
      v1++;
      v2++;
    }
  }

  for(;v1!=parameter1.end();++v1){
    value.push_back(std::make_pair(v1->first,v1->second));
  }

  return GDDD(variable,value);
};

/* constructor*/
_DED *_DED_Minus::create(const GDDD &g1,const GDDD &g2){
  if(g1 == GDDD::top && g2 == GDDD::top)
    return new _DED_GDDD(GDDD::top);
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
  return 827*parameter1.hash()+1153*parameter2.hash();
};

bool _DED_Concat::operator==(const _DED &e)const{
  return ((parameter1==((_DED_Concat*)&e)->parameter1)&&(parameter2==((_DED_Concat*)&e)->parameter2));
};

/* Transform */
GDDD _DED_Concat::eval() const{
  int variable=parameter1.variable();
  GDDD::Valuation value;
  std::map<int,std::set<GDDD> > map_set;
  for(GDDD::const_iterator v1=parameter1.begin();v1!=parameter1.end();++v1){
    std::pair<int,GDDD> x(v1->first,(v1->second)^parameter2);
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
  return 1451*hom.hash()+1399*parameter.hash();
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
  std::cout << "*\nCache Stats : size=" << cache.size() << std::endl;  
  std::cout << "\nCache hit ratio : " << double (Hits*100) / double(Misses+1+Hits) << "%" << std::endl;
  // long hitr=(Hits*100) / (Misses+1+Hits) ;
  if (reinit){
    Hits = 0;
    Misses = 0;  
  }  

}


static size_t DEDpeak = 0;

size_t DED::peak() {
  if (cache.size() > DEDpeak)
    DEDpeak = cache.size();
  return DEDpeak;
}
// Todo
void DED::garbage(){
  if (cache.size() > DEDpeak)
    DEDpeak = cache.size();
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

// eval and std::set to NULL the DED
GDDD DED::eval(){

	if(typeid(*concret)==typeid(_DED_GDDD))
	{
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

 	Cache::const_iterator ci=cache.find(*this); // search e in the cache

	if(ci==cache.end())
	{ // *this is not in the cache
		Misses++;
//		std::cout << "NOT IN CACHE" << std::endl;
		GDDD res=concret->eval(); // compute the result
    	cache[*this]=res;
		concret=NULL;
		return res;
	}
	else 
	{// *this is in the cache
//		std::cout << "IN CACHE" << std::endl;
		Hits++;
		delete concret;
		return ci->second;
	}
};

size_t DED::hash () const {
  return concret->hash();
}

/* binary operators */

GDDD DED::hom(const GHom &h,const GDDD&g){
  DED e(_DED_Hom::create(h,g));
  return e.eval();
};

GDDD DED::add(const std::set<GDDD> &s){
  DED e(_DED_Add::create(s));
  return e.eval();
};

GDDD operator+(const GDDD &g1,const GDDD &g2){
  std::set<GDDD> s;
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


