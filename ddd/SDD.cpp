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
#include "ddd/util/set.hh"
#include <map>
#include <sstream>
#include <cassert>
#include <typeinfo>

#include "ddd/SDED.h"
#include "ddd/SDD.h"
#include "ddd/UniqueTable.h"
#include "ddd/IntDataSet.h"
#include "ddd/DDD.h"
#include "ddd/SHom.h"
#include "ddd/util/hash_support.hh"
#include "ddd/util/ext_hash_map.hh"


#ifdef REENTRANT
#include "tbb/atomic.h"
#include "tbb/queuing_rw_mutex.h"
#endif




/******************************************************************************/
/*                             class _GSDD                                     */
/******************************************************************************/

static bool valsorter (const GSDD::Valuation::value_type &a, const GSDD::Valuation::value_type &b) {
  return a.first->set_less_than (*b.first);
}

class _GSDD{
public:
  /* Attributs*/
  const int variable;
  GSDD::Valuation valuation;
  mutable unsigned long int _refCounter;
#ifdef HEIGHTSDD
  mutable short int height;
#endif


  /* Constructor */
  _GSDD(int var,int cpt=0):variable(var),_refCounter(2*cpt)
#ifdef HEIGHTSDD
			   ,height(-1)
#endif 
			   {}; 
  _GSDD(int var,GSDD::Valuation val,int cpt=0):variable(var),valuation(val),_refCounter(2*cpt)
#ifdef HEIGHTSDD
			   ,height(-1)
#endif 
			   {
    sort (valuation.begin(), valuation.end(), valsorter);
  }; 

  virtual ~_GSDD () {
    for (GSDD::Valuation::iterator it= valuation.begin(); it != valuation.end() ; ++it) {
      delete it->first;
    }
  }


  

  _GSDD (const _GSDD &g):variable(g.variable),valuation(g.valuation),_refCounter(g._refCounter)
#ifdef HEIGHTSDD
			   ,height(g.height)
#endif 
			{
    for (GSDD::Valuation::iterator it= valuation.begin(); it != valuation.end() ; ++it) {
      it->first = it->first->newcopy();
    }
  }

    _GSDD * clone () const { return new _GSDD(*this); }

  bool operator<(const _GSDD& g) const{
    if ( variable !=g.variable)
      return variable < g.variable;
    int n1 = valuation.size();
    int n2 = g.valuation.size();
    if (n1 < n2) return true;
    if (n1 > n2) return false;
    GSDD::const_iterator jt = g.valuation.begin();
    for (GSDD::const_iterator it= valuation.begin() ; it != valuation.end() ; ++it, ++jt )
      {
	if (*it == *jt)
	  continue;
	if (*it < *jt)
	  return true;
	return false;
      }
    return false;
  }

  /* Compare */
  bool operator==(const _GSDD& g) const 
  { 
    if (variable!=g.variable || valuation.size()!= g.valuation.size()) 
      return false;  
   
    for (GSDD::const_iterator it = valuation.begin(),jt=g.valuation.begin(); it != valuation.end() && jt != g.valuation.end() ; it++,jt++ )
      if (!(it->first->set_equal(*jt->first) && it->second == jt->second))
	return false;
    return true;
  }
  
  
#ifdef HEIGHTSDD
  short int getHeight () const {
    if (height == -1) {
      for (GSDD::const_iterator it= valuation.begin(); it != valuation.end() ; ++it) {
	short sonheight = it->second.concret->getHeight();
	if (typeid(*it->first) == typeid(SDD) ) 
	  sonheight += ((const SDD *) it->first)->concret->getHeight();
	height = (height < sonheight) ? sonheight : height;	
      }
      ++height;
    }
    return height;
  }
#endif // HEIGHTSDD

  /* Memory Manager */
  void mark()const;

  size_t hash() const{
    size_t res=(size_t) variable;
    for(GSDD::const_iterator vi=valuation.begin();vi!=valuation.end();++vi)
      res ^=   vi->first->set_hash()  
	+  vi->second.hash()  ;
    return res;
  }

void mark_if_refd () const {
	if ( refCounter() ) {		
		mark();	
	}  
  }
  
  void ref () const {
	_refCounter += 2;
  }
  
  void deref () const {
	_refCounter -= 2;
  }
  
  unsigned long int refCounter() const {
	return _refCounter >> 1;
  }
  
  bool is_marked() const {
	return _refCounter & 1;
  }
  
  void set_mark (bool val) const {
	if (val) {
		_refCounter |= 1;
	} else {
		_refCounter >>= 1;
		_refCounter <<= 1;		
	}
  }

};


// map<int,string> mapVarName;
#ifdef REENTRANT

static tbb::atomic<size_t> Max_SDD;
class SDD_parallel_init
{
public:
	 
	SDD_parallel_init()
	{
		Max_SDD = 0;
	}
		
};
static SDD_parallel_init SDD_init;

#else
static size_t Max_SDD=0;
#endif

static UniqueTable<_GSDD> canonical;
namespace sns{
  UniqueTable<_GShom> canonical;
}

namespace SDDutil {
  
  UniqueTable<_GSDD>  * getTable () {return &canonical;}
  


  void foreachTable (void (*foo) (const GSDD & g)) {
    for(UniqueTable<_GSDD>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();++di){
      (*foo) (GSDD( (*di)));
    }
  }

}


/* Constants */
const GSDD GSDD::one(canonical( _GSDD(1,1)));
const GSDD GSDD::null(canonical( _GSDD(0,1)));
const GSDD GSDD::top(canonical( _GSDD(-1,1)));

// declared here to ensure correct static init order
const Shom Shom::null = GSDD::null ;
// constant Id
namespace sns {
  const _GShom * getIdentity();
} 
const GShom GShom::id (sns::getIdentity());

/******************************************************************************/
/*                             class GSDD                                     */
/******************************************************************************/

/* Memory manager */
unsigned int GSDD::statistics() {
  return canonical.size();
}

// Todo
void GSDD::mark()const{
  concret->mark();
}

void _GSDD::mark()const{
  if(! is_marked() ){
    set_mark(true);
    for(GSDD::Valuation::const_iterator vi=valuation.begin();vi!=valuation.end();++vi){
			vi->first->mark();
      vi->second.mark();
    }
  }
}

size_t GSDD::peak() {
 if (canonical.size() > Max_SDD) 
    Max_SDD=canonical.size();  

  return Max_SDD;
}

void GSDD::pstats(bool)
{
  std::cout << "Current/Peak number of SDD nodes in unicity table :" << canonical.size() << "/" << peak() << std::endl; 

  std::cout << "sizeof(_GSDD):" << sizeof(_GSDD) << std::endl;
  
#ifdef HASH_STAT
  std::cout << std::endl << "SDD Unicity table stats :" << std::endl;
  print_hash_stats(canonical.get_hits(), canonical.get_misses(), canonical.get_bounces());
#endif // HASH_STAT
}



/* Visualisation*/
void GSDD::print(std::ostream& os,std::string s) const{
  if (*this == one)
    os << "[ " << s << "]"<<std::endl;
  else if(*this == top)
      os << "[ " << s << "T ]"<<std::endl;
  else if(*this == null)
      os << "[ " << s << "0 ]"<<std::endl;
  else{
    // should not happen
    assert ( begin() != end());

    for(GSDD::const_iterator vi=begin();vi!=end();++vi){
      std::stringstream tmp;
      // Fixme  for pretty print variable names
//      string varname = GDDD::getvarName(variable());
      tmp<<"var" << variable() <<  " " ;
      vi->first->set_print(tmp); 
      vi->second.print(os,s+tmp.str()+" ");
    }
  }
}
 

std::ostream& operator<<(std::ostream &os,const GSDD &g){
  std::string s;
  g.print(os,s);
  return(os);
}


GSDD::GSDD(const _GSDD *_g):concret(_g){
} 

GSDD::GSDD(const _GSDD &_g):concret(canonical(_g)){ 
}



GSDD::GSDD(int variable,Valuation value){
  
  concret= value.size() != 0 ?  canonical(_GSDD(variable,value)) : null.concret;
}


GSDD::GSDD(int var,const DataSet &val,const GSDD &d):concret(null.concret){ //var-val->d
  if(d!=null && ! val.empty() ){
    _GSDD _g = _GSDD(var,0);
    // cast to (DataSet*) to lose "const" type
    std::pair<DataSet *, GSDD> x( val.newcopy(),d);
    _g.valuation.push_back(x);
    concret=canonical(_g);    
  }
  //  concret->refCounter++;
}

GSDD::GSDD(int var,const GSDD &va,const GSDD &d):concret(null.concret){ //var-val->d
  SDD val (va);
  if(d!=null && ! val.empty() ){
    _GSDD _g =  _GSDD(var,0);
    // cast to (DataSet*) to lose "const" type
    std::pair<DataSet *, GSDD> x( val.newcopy(),d);
    _g.valuation.push_back(x);
    concret=canonical(_g);    
  }
  //  concret->refCounter++;
}

GSDD::GSDD(int var,const SDD &val,const GSDD &d):concret(null.concret){ //var-val->d
  if(d!=null && ! val.empty() ){
    _GSDD _g = _GSDD(var,0);
    // cast to (DataSet*) to lose "const" type
    std::pair<DataSet *, GSDD> x( val.newcopy(),d);
    _g.valuation.push_back(x);
    concret=canonical(_g);    
  }
  //  concret->refCounter++;
}



/* Accessors */
int GSDD::variable() const{
  return concret->variable;
}

bool GSDD::operator<(const GSDD& g) const {  
      return *concret < * g.concret;
};


size_t GSDD::nbsons () const { 
  return concret->valuation.size();
}

#ifdef HEIGHTSDD
short GSDD::getHeight () const {
  return concret->getHeight();
}
#endif

GSDD::const_iterator GSDD::begin() const{
  return concret->valuation.begin();
}

GSDD::const_iterator GSDD::end() const{
  return concret->valuation.end();
}

/* Visualisation */
unsigned int GSDD::refCounter() const{
  return concret->refCounter();
}

class SddSize{
private:

  bool firstError;
  d3::set<GSDD>::type s;
  // Was used to compute number of nodes in referenced datasets as well
  // but dataset doesn't define what we need as it is not necessarily 
  // a decision diagram implementation => number of nodes = ??
//   set<DataSet &> sd3;
  // trying to repair it : consider we reference only SDD or DDD for now, corresponds to current usage patterns
  d3::set<GDDD>::type sd3;



  void sddsize(const GSDD& g)
{
    if(s.find(g)==s.end()){
      s.insert(g);
      res++;
      for(GSDD::const_iterator gi=g.begin();gi!=g.end();++gi) 
	sddsize(gi->first);
      
      for(GSDD::const_iterator gi=g.begin();gi!=g.end();++gi)
	sddsize(gi->second);
      
    }
  }

  void sddsize(const DataSet* g)
{
    // Used to work for referenced DDD
    if (typeid(*g) == typeid(GSDD) ) {
      sddsize( GSDD ((SDD &) *g) );
    } else if (typeid(*g) == typeid(DDD)) {
      sddsize( GDDD ((DDD &) *g) );
    } else if (typeid(*g) == typeid(IntDataSet)) {
      // nothing, no nodes for this implem
    } else {
      if (firstError) {
        std::cerr << "Warning : unknown referenced dataset type on arc, node count is inacurate"<<std::endl;
        std::cerr << "Read type :" << typeid(*g).name() <<std::endl ;
	firstError = false;
      }
    }
  }

  void sddsize(const GDDD& g)
	{
      if (sd3.find(g)==sd3.end()) {
	sd3.insert(g);
	d3res ++;
	for(GDDD::const_iterator gi=g.begin();gi!=g.end();++gi)
	  sddsize(gi->second);
      }
  }

public:
  unsigned long int res;
  unsigned long int d3res;

	SddSize()
		:
		firstError(true), res(0), d3res(0)
	{
	};
//  pair<unsigned long int,unsigned long int> operator()(const GSDD& g){
	unsigned long int operator()(const GSDD& g){
#ifndef REENTRANT
    res=0;
    d3res=0;
    sd3.clear();
    s.clear();
#endif
    sddsize(g);
    // we used to return a pair : number of nodes in SDD, number of nodes in referenced data structures
//    return make_pair(res,d3res);
    return res;
  }
};


std::pair<unsigned long int,unsigned long int> GSDD::node_size() const{
#ifndef REENTRANT
	static 
#endif
	SddSize sddsize;
	sddsize(*this);
	return std::make_pair(sddsize.res,sddsize.d3res);
}

// old prototype
// pair<unsigned long int,unsigned long int> GSDD::size() const{
unsigned long int GSDD::size() const{
  static SddSize sddsize;
  return sddsize(*this);
}

class MySDDNbStates{
private:
  int val; // val=0 donne nbState , val=1 donne noSharedSize
  typedef ext_hash_map<GSDD,long double> cache_type;
  static cache_type cache;
	
long double nbStates(const GSDD& g)
{
	if(g==GSDD::one)
		return 1;
	else if(g==GSDD::top || g==GSDD::null)
		return 0;
	else
	{
	  cache_type::accessor access;  
	  cache.find(access,g);
	  if( access.empty() ) {
	    long double res=0;
	    for(GSDD::const_iterator gi=g.begin();gi!=g.end();++gi)
	      res+=(gi->first->set_size())*nbStates(gi->second)+val;
	    cache.insert(access,g);
	    access->second = res;
	    return res;
	  } 
		else 
		{
			return access->second;
		}
	}
}

public:
  MySDDNbStates(int v):val(v){};
  long double operator()(const GSDD& g){
    long double res=nbStates(g);
//    s.clear();
    return res;
  }

  static void clear () {
    cache.clear();
  }
};

ext_hash_map<GSDD,long double> MySDDNbStates::cache = ext_hash_map<GSDD,long double> ();

long double GSDD::nbStates() const{
  static MySDDNbStates myNbStates(0);
  return myNbStates(*this);
}


void GSDD::garbage(){
  if (canonical.size() > Max_SDD) 
    Max_SDD=canonical.size();  

  MySDDNbStates::clear();
  // mark phase
  for(UniqueTable<_GSDD>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();++di){
    (*di)->mark_if_refd();
  }

  

  // sweep phase  
  for(UniqueTable<_GSDD>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();){
    if(! (*di)->is_marked()){
      UniqueTable<_GSDD>::Table::iterator ci=di;
      di++;
      const _GSDD *g=(*ci);
      canonical.table.erase(ci);
      delete g;
    }
    else{
      (*di)->set_mark(false);
      di++;
    }
  }
}


// FIXME
// long double GSDD::noSharedSize() const{
//   static MyNbStates myNbStates(1);
//   return myNbStates(*this);
// }


/******************************************************************************/
/*                   class SDD:public GSDD                                    */
/******************************************************************************/

SDD::SDD(const SDD &g)
    : GSDD(g.concret)
{
    concret->ref();
}

SDD::SDD(const GSDD &g):GSDD(g.concret){
  concret->ref();
}


SDD::SDD(int var,const DataSet& val,const GSDD &d):GSDD(var,val,d){
  concret->ref();
}

SDD::SDD(int var,const GSDD& val,const GSDD &d):GSDD(var,val,d){
  concret->ref();
}

SDD::SDD(int var,const SDD& val,const GSDD &d):GSDD(var, val,d){
  concret->ref();
}


SDD::~SDD(){
  assert(concret->refCounter()>0);
  concret->deref();
}


SDD &SDD::operator=(const GSDD &g){
  concret->deref();
  concret=g.concret;
  concret->ref();
  return *this;
}

SDD &SDD::operator=(const SDD &g){
  concret->deref();
  concret=g.concret;
  concret->ref();
  return *this;
}

#ifdef EVDDD
/// returns the minimum value of the function encoded by a node
int GSDD::getMinDistance () const {
  int minsucc=-1;
  for (GSDD::const_iterator it = begin() ; it != end() ; ++it) {
    int lmin = it->first->getMinDistance();
    if (minsucc==-1 || lmin < minsucc)
      minsucc = lmin;
  }
  return minsucc==-1?0:minsucc;
}


GSDD GSDD::normalizeDistance(int n) const {
  return pushEVSDD (n) (*this);
}
#endif


// DataSet interface

DataSet *GSDD::set_intersect (const DataSet & b) const {
  return new GSDD((*this) * (GSDD&)b );
}
DataSet *GSDD::set_union (const DataSet & b)  const {
  return new GSDD(*this + (GSDD&)b);
}
DataSet *GSDD::set_minus (const DataSet & b) const {
  return new GSDD(*this - (GSDD&)b);
}

bool GSDD::empty() const {
  return *this == GSDD::null;
}

DataSet * GSDD::empty_set() const {
  return new GSDD();
}

bool GSDD::set_equal(const DataSet & b) const {
  return *this == (GSDD&) b;
}

bool GSDD::set_less_than(const DataSet & b) const {
  return *this < (GSDD&) b;
}

long double GSDD::set_size() const { return nbStates(); }

size_t GSDD::set_hash() const {
  return size_t (concret);
}

