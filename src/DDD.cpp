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

// C functions atoi, etc..
#include <cstdlib>
#include <cstring>
/* -*- C++ -*- */
#include <string>
#include <iostream>
#include <vector>
#include <set>
// modif
#include <cassert>
#include <map>
// modif
#include <sstream>
#include <limits>

#include "util/configuration.hh"
#include "DDD.h"
#include "UniqueTableId.hh"
#include "DED.h"

#ifdef REENTRANT
#include "tbb/atomic.h"
#include "tbb/mutex.h"
#endif


/******************************************************************************/
/*                             class _GDDD                                     */
/******************************************************************************/

typedef  UniqueTableId<_GDDD,GDDD::id_t> DDDutable;


class _GDDD
{
  friend class GDDD;
  friend void saveDDD(std::ostream&, std::vector<DDD>);

  /// useful typedefs
  typedef GDDD::edge_t edge_t; 
  typedef GDDD::const_iterator const_iterator;

  /// attributes
  const int variable;
  const unsigned short valuation_size;

  /// get the address of the valuation
  edge_t *
  alpha_addr () const
  {
    return reinterpret_cast<edge_t *> (reinterpret_cast<char *> (const_cast<_GDDD *> (this)) + sizeof (_GDDD) );
  }

  /// constructor
  _GDDD (int var, const GDDD::Valuation & val)
  : variable (var)
  , valuation_size (val.size ())
  {
    GDDD::Valuation::const_iterator jt = val.begin();
    for (edge_t * it = this->alpha_addr(); it != this->end() ; ++it, ++jt) {
      // placement new
      new (it) edge_t (*jt);
    }
  }

  /// constructor (with iterators)
  _GDDD (int var, const_iterator begin, const_iterator end)
  : variable (var)
  , valuation_size (end-begin)
  {
    const_iterator jt = begin;
    for (edge_t * it = this->alpha_addr(); it != this->end() ; ++it, ++jt) {
      // placement new
      new (it) edge_t (*jt);
    }
  }

  /// cannot copy
  /// these two operations are deliberately private and UNIMPLEMENTED
  _GDDD (const _GDDD &);
  _GDDD & operator= (const _GDDD &);

public:
  /// destructor
  ~_GDDD ()
  {
     for (const_iterator it = begin ();
          it != end (); ++it)
     {
       it->~edge_t ();
     }
  }

  /// iterator API
  const_iterator
  begin () const
  {
    return alpha_addr ();
  }

  const_iterator
  end () const
  {
    return alpha_addr () + valuation_size;
  }


//   /// compare
  bool
  operator== (const _GDDD & g) const
  {
    if (variable != g.variable)
      return false;
    if (valuation_size != g.valuation_size)
      return false;

//     return ! memcmp (begin(), g.begin(), valuation_size * sizeof(edge_t));
    const_iterator it = begin (), jt = g.begin ();
    for (; it != end (); ++it, ++jt)
      {
	if (*it != *jt)
	  return false;
      }
    return true;
  }

  bool
  operator< (const _GDDD & g) const
  {
    if (variable != g.variable)
      return variable < g.variable;
    
    size_t n1 = valuation_size;
    size_t n2 = g.valuation_size;
    if (n1 < n2) return true;
    if (n1 > n2) return false;
//    return memcmp (begin(), g.begin(), n1*sizeof(edge_t));
    for (const_iterator it = begin (), jt = g.begin ();
         it != end (); ++it, ++jt)
      {
	if (*it == *jt)
	  continue;
	if (*it < *jt)
	  return true;
	return false;
      }
    return false;
  }

  /// hash
  size_t
  hash () const
  {
    size_t res = ddd::wang32_hash (variable);
//    int i=1;
    for(const_iterator vi = begin (); vi != end (); ++vi)
      res += (size_t)(ddd::int32_hash(vi->first)+1011) * vi->second.hash();
//      res ^= ddd::int32_hash(vi->first+1011*i++) ^ ddd::int32_hash(vi->second.hash());
    return res;
  }

  /// Memory Manager and reference counting
  void mark() const {
    for(const_iterator vi=begin();vi!=end();++vi){
      vi->second.mark();
    }
  }

  /// factory operation
  static
  GDDD::id_t
  create_unique_GDDD (int var, const GDDD::Valuation & val)
  {
    // a memory cell to store the temporary _GDDD to check unicity in unique table
    // this is reallocated only if it is too small (see maxsize)
    // this avoids repeated allocation/deallocation
    static _GDDD * res = new (custom_new_t (), 0) _GDDD (var, {});
    static size_t maxsize = 0;

    // if the arguments is too large for the memory cell
    if (val.size () > maxsize)
    {
      // deallocate
      delete res;
      // update size
      maxsize = val.size ();
      // reallocate to the appropriate size
      res = new (custom_new_t (), maxsize) _GDDD (var, val);
    }
    else // there is already enough room
    {
      // an object built with a placement new should be explicitly deleted
      res->~_GDDD ();
      // placement new
      new (res) _GDDD (var, val);
    }

    GDDD::id_t ret = DDDutable::instance() (*res);
    return ret;
  }

  static const _GDDD * resolve(GDDD::id_t id) 
  {
    return DDDutable::instance().resolve(id);
  }
  
  /// cloning
  _GDDD *
  clone () const
  {
    return new (custom_new_t (), valuation_size) _GDDD (variable, begin (), end ());
  }

private:
  /// an empty struct tag type used to disambiguate between different variants of the operator new
  /// for _GDDD. The overload that do not use 'custom_new_t' is the classical placement new.
  struct custom_new_t {};
  /// custom operator new
  /// WARNING:
  ///     the expected arguments are not standard
  ///         - the first one (actually sizeof(_GDDD)) is ignored
  ///         - the second one is 'custom_new_t', for disambiguation
  ///         - the third one is the number of successors
  /// syntax: new (custom_new_t(), nb_sons) _GDDD (constructor arguments)
  ///     it looks like a placement new, but this syntax
  ///     is only used to pass arguments to operator new
  /// _GDDD should only be constructed by create_unique_GDDD or clone
  /// please refer to these two functions for invokation examples
  static
  void *
  operator new (size_t, custom_new_t, size_t length)
  {
    // allocate enough memory to store the successors
    // with the global (default) operator new
    size_t siz = sizeof(_GDDD) + length*sizeof(edge_t);
    return ::operator new (siz);
  }

  /// classical placement new
  /// NB: it is the responsibility of the caller that there is enough room to build the _GDDD.
  ///   e.g.    new (ad) _GDDD(v, val)
  ///     the memory chunk at 'ad' must be of size at least sizeof(_GDDD)+sizeof(edge_t)*val.size()
  static
  void *
  operator new (size_t, void * addr)
  {
    return addr;
  }

public:
  /// custom operator delete
  static
  void
  operator delete (void * addr)
  {
    // free the memory allocated by operator new
    // with the global (default) operator delete
    ::operator delete (addr);
  }
};

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

// now stored singleton in table
// static size_t Max_DDD = 0;

#endif

/******************************************************************************/
/*                             class GDDD                                     */
/******************************************************************************/

/* Memory manager */
unsigned int GDDD::statistics() {
  return DDDutable::instance().size();
}

// Todo
void GDDD::mark()const{
  DDDutable::instance().mark(concret);
}


size_t GDDD::peak() {
  return DDDutable::instance().peak_size();
}


void GDDD::pstats(bool)
{
  std::cout << "Peak number of DDD nodes in unicity table :" << peak() << std::endl; 
  std::cout << "sizeof(_GDDD):" << sizeof(_GDDD) << std::endl;
  std::cout << "sizeof(DDD::edge_t):" << sizeof(GDDD::edge_t) << std::endl;
  std::cout << "sizeof(DDD::val_t):" << sizeof(GDDD::val_t) << std::endl;

  
#ifdef HASH_STAT
  std::cout << std::endl << "DDD Unicity table stats :" << std::endl;
  print_hash_stats(DDDutable::instance().get_hits(), DDDutable::instance().get_misses(), DDDutable::instance().get_bounces());
#endif // HASH_STAT
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
    const_iterator end = this->end();
    for(GDDD::const_iterator vi=begin();vi!=end;++vi){
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

GDDD::GDDD(const id_t &_g):concret(_g){}


GDDD::GDDD(int variable,const Valuation & value){
#ifdef EVDDD
  if (variable != DISTANCE) {
#endif
    concret=(value.size()!=0)? _GDDD::create_unique_GDDD(variable, value): null.concret;
#ifdef EVDDD
  } else {
    assert(value.size() == 1);
    Valuation::iterator it = value.begin();
    new(this) GDDD(variable,it->first,it->second);
  }
#endif
}

bool GDDD::operator< (const GDDD & g) const { return concret < g.concret; }

/* Accessors */
int GDDD::variable() const{
  return _GDDD::resolve(concret)->variable;
}

size_t GDDD::nbsons () const { 
  return _GDDD::resolve(concret)->valuation_size;
}

GDDD::const_iterator GDDD::begin() const{
  return _GDDD::resolve(concret)->begin();
}

GDDD::const_iterator GDDD::end() const{
  return _GDDD::resolve(concret)->end();
}

/* Visualisation */

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
	  GDDD::const_iterator end = g.end();
	  for(GDDD::const_iterator gi=g.begin();gi!=end;++gi)
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
	    GDDD::const_iterator end = g.end();
	    for(GDDD::const_iterator gi=g.begin();gi!=end;++gi)
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
  MyNbStates::clear();
  // mark terminals
  null.mark();
  one.mark();
  top.mark();
  DDDutable::instance().garbage();
}




/* Constants */
const GDDD GDDD::one(_GDDD::create_unique_GDDD(1,GDDD::Valuation()));
const GDDD GDDD::null(_GDDD::create_unique_GDDD(0,GDDD::Valuation()));
const GDDD GDDD::top(_GDDD::create_unique_GDDD(-1,GDDD::Valuation()));

/******************************************************************************/
/*                   class DDD:public GDDD                                    */
/******************************************************************************/

DDD::DDD(const DDD &g):GDDD(g.concret),DataSet(){
  DDDutable::instance().ref(concret);
}

DDD::DDD(const GDDD &g):GDDD(g.concret){
  DDDutable::instance().ref(concret);
}

GDDD::GDDD(int var,int val,const GDDD &d):concret(null.concret){ //var-val->d
  if(d!=null){
    GDDD::Valuation tmp;
#ifdef EVDDD
    GDDD succ = d;
    if (var == DISTANCE) {
	int minsucc=succ.getMinDistance();
	if (minsucc != 0) {
	  val += minsucc;
	  succ = succ.normalizeDistance(-minsucc);
	}
    }
    edge_t x(val,succ);
#else
    edge_t x(val,d);
#endif
    tmp.push_back(x);
    concret=_GDDD::create_unique_GDDD(var,tmp);
  }
  //  concret->refCounter++;
}

GDDD::GDDD(int var,int val1,int val2,const GDDD &d):concret(null.concret){ //var-[val1,val2]->d
  if(val1<=val2 && null!=d){
    GDDD::Valuation tmp;
    for(int val=val1;val<=val2;++val){
      edge_t x(val,d);
      tmp.push_back(x);
    }
    concret=_GDDD::create_unique_GDDD(var,tmp);
  }
  //  concret->refCounter++;
}

DDD::DDD(int var,int val,const GDDD &d):GDDD(var,val,d){
  DDDutable::instance().ref(concret);
}

DDD::DDD(int var,int val1,int val2,const GDDD &d):GDDD(var,val1,val2,d){
    DDDutable::instance().ref(concret);
}

DDD::~DDD(){
  DDDutable::instance().deref(concret);
}

DDD &DDD::operator=(const GDDD &g){
  DDDutable::instance().deref(concret);
  concret=g.concret;
  DDDutable::instance().ref(concret);
  return *this;
}

DDD &DDD::operator=(const DDD &g){
  DDDutable::instance().deref(concret);
  concret=g.concret;
  DDDutable::instance().ref(concret);
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
    const_iterator end = end();
    for (GDDD::const_iterator it = begin() ; it != end ; ++it) {
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

unsigned long int GDDD::nodeIndex(const std::vector<GDDD::id_t> & list) const{
    assert(this);
    assert(concret!=0);
    unsigned long int i=0;
    for (i=0; i<list.size();++i)
      if (concret==list[i]) return i;
    return std::numeric_limits<unsigned long>::max();
}


void GDDD::saveNode(std::ostream& os, std::vector<GDDD::id_t>& list)const {
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
	  GDDD::const_iterator end = this->end();
	  for (GDDD::const_iterator vi=begin();vi!=end;++vi) 
	    vi->second.saveNode(os, list);
	  list.push_back(concret);
        }
    }
}



void saveDDD(std::ostream& os, std::vector<DDD> list) {
  std::vector<GDDD::id_t> SavedDDD;
  for (unsigned int i= 0; i<list.size(); ++i) {
    list[i].saveNode(os, SavedDDD);
  }
  os<<SavedDDD.size()<<std::endl;
  for (unsigned long int i=0; i<SavedDDD.size();++i) {
    GDDD d = SavedDDD[i];
    if (d==GDDD::one) 
      os<<i<<" one"<<std::endl;
    else if (d==GDDD::null) 
      os<<i<<" null"<<std::endl;
    else if (d==GDDD::top) 
      os<<i<<" top"<<std::endl;
    else {
      os<<i<<"[ "<< d.variable();
      for (GDDD::const_iterator vi=d.begin();vi!=d.end();++vi)
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
    GDDD::Valuation valuation;
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

#include "MemoryManager.h"

// for lack of a better place to put it...
size_t MemoryManager::last_mem = 1300000;
// for lack of a better place to put it...
MemoryManager::hooks_t MemoryManager::hooks_ = MemoryManager::hooks_t();


