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
#include <string>
#include <iostream>
#include <vector>
#include <set>
// modif
#include <cassert>
#include <ext/hash_map>
#include <map>
// modif
#include <sstream>
#include <limits>

#include "util/configuration.hh"
#include "DDD.h"
#include "UniqueTable.h"
#include "DED.h"

#ifdef REENTRANT
#include "tbb/atomic.h"
#include "tbb/mutex.h"
#endif


/******************************************************************************/
/*                             class _GDDD                                     */
/******************************************************************************/

class _GDDD
{
public:
  /* Attributs*/
  const int variable;
  GDDD::Valuation valuation;

#ifdef REENTRANT
	mutable tbb::atomic<unsigned long int> refCounter;
	mutable tbb::atomic<bool> marking;
#else
  	mutable unsigned long int refCounter;
  	mutable bool marking;
#endif

  /* Constructor */
	_GDDD(int var,int cpt=0)
		: variable(var)
	{
		refCounter = cpt;
		marking = false;
	} 

	_GDDD(int var,GDDD::Valuation val,int cpt=0)
		: variable(var)
		, valuation(val)
	{
		refCounter = cpt;
		marking = false;
	}

  /* Compare */
  bool operator==(const _GDDD& g) const{return variable==g.variable && valuation==g.valuation;};

  /* Memory Manager */
  void mark()const;

  _GDDD * clone () const { return new _GDDD(*this); }

  size_t hash () const {
    size_t res=(size_t) variable;
    for(GDDD::const_iterator vi=valuation.begin();vi!=valuation.end();++vi)
      res+=(size_t)(vi->first+1011)* vi->second.hash();
    return res;
  }

};


static UniqueTable<_GDDD> canonical;
std::map<int,std::string> mapVarName;

#ifdef REENTRANT

static tbb::atomic<size_t> Max_DDD;

class DDD_parallel_init
{
public:
	 
	DDD_parallel_init()
	{
		Max_DDD = 0;
	}
		
};
static DDD_parallel_init DDD_init;

#else

static size_t Max_DDD = 0;

#endif

/******************************************************************************/
/*                             class GDDD                                     */
/******************************************************************************/

/* Memory manager */
unsigned int GDDD::statistics() {
  return canonical.size();
}

// Todo
void GDDD::mark()const{
  concret->mark();
}

void _GDDD::mark()const{
  if(!marking){
    marking=true;
    for(GDDD::Valuation::const_iterator vi=valuation.begin();vi!=valuation.end();++vi){
      vi->second.mark();
    }
  }
}

size_t GDDD::peak() {
  if (canonical.size() > Max_DDD) 
    Max_DDD=canonical.size();  

  return Max_DDD;
}


void GDDD::pstats(bool)
{
  std::cout << "Peak number of DDD nodes in unicity table :" << peak() << std::endl; 
}



/* Visualisation*/
void GDDD::print(std::ostream& os,std::string s) const{
  if (*this == one)
    os << "[ " << s << "]"<<std::endl;
  else if(*this == top)
      os << "[ " << s << "T ]"<<std::endl;
  else if(*this == null)
      os << "[ " << s << "0 ]"<<std::endl;
  else{
    for(GDDD::const_iterator vi=begin();vi!=end();++vi){
      // modif strstream -> std::stringstream
      std::stringstream tmp;
      tmp << getvarName(variable())<<'('<<vi->first<<")";
      vi->second.print(os,s+tmp.str() +" ");
    }
  }
}

std::ostream& operator<<(std::ostream &os,const GDDD &g){
  std::string s;
  g.print(os,s);
  return(os);
}

GDDD::GDDD(const _GDDD &_g):concret(canonical(_g)){}
GDDD::GDDD(const _GDDD *_g):concret(_g){}


GDDD::GDDD(int variable,Valuation value){
#ifdef EVDDD
  if (variable != DISTANCE) {
#endif
    concret=(value.size()!=0)? canonical(_GDDD(variable,value)): null.concret;
#ifdef EVDDD
  } else {
    assert(value.size() == 1);
    Valuation::iterator it = value.begin();
    new(this) GDDD(variable,it->first,it->second);
  }
#endif
}

/* Accessors */
int GDDD::variable() const{
  return concret->variable;
}

size_t GDDD::nbsons () const { 
  return concret->valuation.size();
}

GDDD::const_iterator GDDD::begin() const{
  return concret->valuation.begin();
}

GDDD::const_iterator GDDD::end() const{
  return concret->valuation.end();
}

/* Visualisation */
unsigned int GDDD::refCounter() const{
  return concret->refCounter;
}

class MySize{

private:
#ifdef REENTRANT
	tbb::atomic<unsigned long int> res;
#else
  unsigned long int res;
#endif

  std::set<GDDD> s;
  void mysize(const GDDD& g)
  {
    if( s.find(g) == s.end() )
	{
		s.insert(g);
		res++;
		for(GDDD::const_iterator gi=g.begin();gi!=g.end();++gi)
			mysize(gi->second);
    }
  }

public:

  unsigned long int operator()(const GDDD& g){
    res=0;
    s.clear();
    mysize(g);
    return res;
  }
};

unsigned long int GDDD::size() const{
 static	MySize mysize;

  return mysize(*this);
}

class MyNbStates{
private:
  int val; // val=0 donne nbState , val=1 donne noSharedSize
  typedef ext_hash_map<GDDD,long double> cache_type;
  static cache_type cache;


long double nbStates(const GDDD& g){

	if(g==GDDD::one)
		return 1;
	else if(g==GDDD::top || g==GDDD::null)
		return 0;
	else{
	  cache_type::accessor access;  
	  cache.find(access,g);
  
	  if( access.empty() ) {
	    long double res=0;
	    for(GDDD::const_iterator gi=g.begin();gi!=g.end();++gi)
	      res+=nbStates(gi->second)+val;
	    cache.insert(access,g);
	    access->second = res;
	    return res;
	  } else {
	    return access->second;
	  }

	}
}

public:
  MyNbStates(int v):val(v){};
  long double operator()(const GDDD& g){
    long double res=nbStates(g);
//    s.clear();
    return res;
  }

  static void clear () {
    cache.clear();
  }
};

ext_hash_map<GDDD,long double> MyNbStates::cache = ext_hash_map<GDDD,long double> ();

long double
GDDD::nbStates() const
{
#ifdef REENTRANT
tbb::mutex nb_states_mutex_;
  tbb::mutex::scoped_lock lock(nb_states_mutex_);
#endif
  static MyNbStates myNbStates(0);
  return myNbStates(*this);
}

long double GDDD::noSharedSize() const{
  static MyNbStates myNbStates(1);
  return myNbStates(*this);
}


void GDDD::garbage(){
  // mark phase
  if (canonical.size() > Max_DDD) 
    Max_DDD=canonical.size();  

  MyNbStates::clear();

  for(UniqueTable<_GDDD>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();++di){
    if((*di)->refCounter!=0)
      (*di)->mark();
  }

  // sweep phase
  
  for(UniqueTable<_GDDD>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();){
    if(!((*di)->marking)){
      UniqueTable<_GDDD>::Table::iterator ci=di;
      di++;
      const _GDDD *g=(*ci);
      canonical.table.erase(ci);
      delete g;
    }
    else{
      (*di)->marking=false;
      di++;
    }
  }
}




/* Constants */
const GDDD GDDD::one(canonical(_GDDD(1,1)));
const GDDD GDDD::null(canonical(_GDDD(0,1)));
const GDDD GDDD::top(canonical(_GDDD(-1,1)));

/******************************************************************************/
/*                   class DDD:public GDDD                                    */
/******************************************************************************/

DDD::DDD(const DDD &g):GDDD(g.concret),DataSet(){
  (concret->refCounter)++;
}

DDD::DDD(const GDDD &g):GDDD(g.concret){
  (concret->refCounter)++;
}

GDDD::GDDD(int var,int val,const GDDD &d):concret(null.concret){ //var-val->d
  if(d!=null){
    _GDDD _g = _GDDD(var,0);
#ifdef EVDDD
    GDDD succ = d;
    if (var == DISTANCE) {
	int minsucc=succ.getMinDistance();
	if (minsucc != 0) {
	  val += minsucc;
	  succ = succ.normalizeDistance(-minsucc);
	}
    }
    std::pair<int,GDDD> x(val,succ);
#else
    std::pair<int,GDDD> x(val,d);
#endif
    _g.valuation.push_back(x);
    concret=canonical(_g);
  }
  //  concret->refCounter++;
}

GDDD::GDDD(int var,int val1,int val2,const GDDD &d):concret(null.concret){ //var-[val1,val2]->d
  if(val1<=val2 && null!=d){
    _GDDD _g = _GDDD(var,0);
    for(int val=val1;val<=val2;++val){
      std::pair<int,GDDD> x(val,d);
      _g.valuation.push_back(x);
    }
    concret=canonical(_g);
  }
  //  concret->refCounter++;
}

DDD::DDD(int var,int val,const GDDD &d):GDDD(var,val,d){
  concret->refCounter++;
}

DDD::DDD(int var,int val1,int val2,const GDDD &d):GDDD(var,val1,val2,d){
  concret->refCounter++;
}

DDD::~DDD(){
  assert(concret->refCounter>0);
  concret->refCounter--;
}

DDD &DDD::operator=(const GDDD &g){
  concret->refCounter--;
  concret=g.concret;
  concret->refCounter++;
  return *this;
}

DDD &DDD::operator=(const DDD &g){
  concret->refCounter--;
  concret=g.concret;
  concret->refCounter++;
  return *this;
}

/* Visualisation */
void GDDD::varName(int var,const std::string &name){
  mapVarName[var]=name;
}

const std::string GDDD::getvarName(int var)
{
#ifdef EVDDD
  if (var==DISTANCE) 
    return "dist";
#endif
  std::stringstream tmp;
  std::map<int,std::string>::iterator i=mapVarName.find(var);
  if (i==mapVarName.end())
    tmp<<"var"<< var ;
  else
    tmp<<i->second;
  return tmp.str();

}


#ifdef EVDDD
/// returns the minimum value of the function encoded by a node
int GDDD::getMinDistance () const {
  if (variable() == DISTANCE) {
    assert (nbsons() == 1);
    return begin()->first;
  } else {
    int minsucc=-1;
    for (GDDD::const_iterator it = begin() ; it != end() ; ++it) {
      assert (it->second.nbsons() == 1);
      GDDD::const_iterator succd = it->second.begin();
      if (minsucc==-1 || succd->first < minsucc)
	minsucc = succd->first;
    }
    return minsucc==-1?0:minsucc;
  }
}

GDDD GDDD::normalizeDistance(int n) const {
  return pushEVDDD (n) (*this);
}
#endif

// DataSet interface

DataSet *DDD::set_intersect (const DataSet & b) const {
  return new DDD((*this) * (DDD &) b);
}
DataSet *DDD::set_union (const DataSet & b)  const {
  return new DDD(*this +(DDD &) b);
}
DataSet *DDD::set_minus (const DataSet & b) const {
  return new DDD(*this -(DDD &) b);
}

bool DDD::empty() const {
  return this->GDDD::operator==(GDDD::null);
}

DataSet * DDD::empty_set() const {
  return new DDD();
}

bool DDD::set_equal(const DataSet & b) const {
  return *this == (DDD &) b;
}

bool DDD::set_less_than(const DataSet & b) const {
  return *this < (DDD&) b;
}

  
long double DDD::set_size() const { return nbStates(); }

size_t DDD::set_hash() const {
  return hash();
}




//My funs

unsigned long int GDDD::nodeIndex(const std::vector<const _GDDD*> & list) const{
    assert(this);
    assert(concret);
    unsigned long int i=0;
    for (i=0; i<list.size();++i)
      if (concret==list[i]) return i;
    return std::numeric_limits<unsigned long>::max();
}


void GDDD::saveNode(std::ostream& os, std::vector<const _GDDD*>& list)const {
    assert(this);
    //assert(concret);
    unsigned long int index = nodeIndex(list);
    if (index == std::numeric_limits<unsigned long>::max() ) {

        if (*this==one) list.push_back(concret);
        else 
        if (*this==null) list.push_back(concret);
        else 
        if (*this==top) list.push_back(concret);
        else {
            assert(concret);
                for (GDDD::Valuation::const_iterator vi=begin();vi!=end();++vi) 
                    vi->second.saveNode(os, list);
                list.push_back(concret);
        }
    }
}



void saveDDD(std::ostream& os, std::vector<DDD> list) {
  std::vector<const _GDDD*> SavedDDD;
    for (unsigned int i= 0; i<list.size(); ++i) {
        list[i].saveNode(os, SavedDDD);
    }
    os<<SavedDDD.size()<<std::endl;
    for (unsigned long int i=0; i<SavedDDD.size();++i) {
        if (GDDD(SavedDDD[i])==GDDD::one) os<<i<<" one"<<std::endl;
        else
        if (GDDD(SavedDDD[i])==GDDD::null) os<<i<<" null"<<std::endl;
        else
        if (GDDD(SavedDDD[i])==GDDD::top) os<<i<<" top"<<std::endl;
        else {
            os<<i<<"[ "<<SavedDDD[i]->variable;
            for (GDDD::const_iterator vi=GDDD(SavedDDD[i]).begin();vi!=GDDD(SavedDDD[i]).end();++vi)
                os<<" "<<vi->first<<" "<<vi->second.nodeIndex(SavedDDD);
            os<<" ]"<<std::endl;
        }
    }
        
    os<<std::endl<<"Saved:";
    for (unsigned int i= 0; i<list.size(); ++i) os<<" "<<list[i].nodeIndex(SavedDDD);
    os<<std::endl;
    
}

void loadDDD(std::istream& is, std::vector<DDD>& list) {
    unsigned long int size;
    unsigned long int index;
    int var;
    int val;
    std::vector<std::pair<int,GDDD> > valuation;
    std::string temp;
    is>>size;
    std::vector<GDDD> nodes(size);
    for (unsigned long int i=0;i<size;++i) {
        is>>index;
        is>>temp;
        if (temp==std::string("one")) {nodes[index]=GDDD::one; /*std::cout<<"one"<<std::endl;std::cout.flush();*/}
        else if (std::string(temp)==std::string("null")) nodes[index]=GDDD::null;
        else if (std::string(temp)==std::string("top")) nodes[index]=GDDD::top;
        else {
            assert (temp==std::string("["));
            is>>var;
            is>>temp;
            while(temp!=std::string("]")) {
                val=atoi(temp.c_str());
                is>>temp;
                valuation.push_back(std::pair<int,GDDD>(val,nodes[strtoul(temp.c_str(),NULL,10)]));
                is>>temp;
            }
            nodes[index]=GDDD(var,valuation);
            valuation.clear();
        }
    }
    is>>temp;
    assert (temp==std::string("Saved:"));
    for(unsigned long int i=0;i<list.size();++i) {
        is>>index; list[i]=nodes[index];
    }
    
}



