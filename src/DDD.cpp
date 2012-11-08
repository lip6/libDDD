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
  friend class GDDD;
  friend void saveDDD(std::ostream&, std::vector<DDD>);

  /// useful typedefs
  typedef GDDD::edge_t edge_t;
  typedef GDDD::const_iterator const_iterator;

  /// attributes
  const int variable;
  const size_t valuation_size;
#ifdef REENTRANT
	mutable tbb::atomic<unsigned long int> _refCounter;
#else
  	mutable unsigned long int _refCounter;
#endif

  /// get the address of the valuation
  char *
  alpha_addr () const
  {
    return reinterpret_cast<char *> (const_cast<_GDDD *> (this)) + sizeof (_GDDD);
  }

  /// constructor
  _GDDD (int var, const GDDD::Valuation & val, int cpt)
  : variable (var)
  , valuation_size (val.size ())
  , _refCounter (2*cpt)
  {
    edge_t * base = reinterpret_cast<edge_t *> (alpha_addr ());
    size_t i = 0;
    for (GDDD::Valuation::const_iterator it = val.begin ();
         it != val.end (); ++it)
    {
      // placement new
      new (base + i++) edge_t (*it);
    }
  }

  /// constructor (with iterators)
  _GDDD (int var, const_iterator begin, const_iterator end, int cpt)
  : variable (var)
  , valuation_size (end-begin)
  , _refCounter (2*cpt)
  {
    edge_t * base = reinterpret_cast<edge_t *> (alpha_addr ());
    size_t i = 0;
    while (begin != end)
    {
      // placement new
      new (base + i++) edge_t (*(begin++));
    }
  }

  /// cannot copy
  /// these two operations are deliberately private and UNIMPLEMENTED
  _GDDD (const _GDDD &);
  _GDDD & operator= (const _GDDD &);

  /// destructor
  ~_GDDD ()
  {
    for (const_iterator it = begin ();
         it != end (); ++it)
    {
      it->~edge_t ();
    }
  }
public:
  /// iterator API
  const_iterator
  begin () const
  {
    return reinterpret_cast<const_iterator> (alpha_addr ());
  }

  const_iterator
  end () const
  {
    return reinterpret_cast<const_iterator> (alpha_addr ()) + valuation_size;
  }

  /// compare
  bool
  operator== (const _GDDD & g) const
  {
    if (variable != g.variable)
      return false;
    if (valuation_size != g.valuation_size)
      return false;

    const_iterator it = begin (), jt = g.begin ();
    for (; it != end (); ++it, ++jt)
    {
      if (*it != *jt)
        return false;
    }
    return true;
  }

  /// hash
  size_t
  hash () const
  {
    size_t res = ddd::wang32_hash (variable);
    for(const_iterator vi = begin (); vi != end (); ++vi)
      res += (size_t)(vi->first+1011) * vi->second.hash();
    return res;
  }

  /// Memory Manager and reference counting
  void mark()const;

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

  /// unicity table
  static
  UniqueTable<_GDDD> &
  get_canonical ()
  {
    static UniqueTable<_GDDD> canonical = UniqueTable<_GDDD> ();
    return canonical;
  }

  /// factory operation
  static
  const _GDDD *
  create_unique_GDDD (int var, const GDDD::Valuation & val, int cpt)
  {
    _GDDD * res = new (val.size ()) _GDDD (var, val, cpt);
    const _GDDD * ret = get_canonical () (*res);
    delete res;
    return ret;
  }

  /// cloning
  _GDDD *
  clone () const
  {
    return new (valuation_size) _GDDD (variable, begin (), end (), _refCounter);
  }

private:  
  /// custom operator new
  /// WARNING:
  ///     the expected arguments are not standard
  ///         - the first one (actually sizeof(_GDDD)) is ignored
  ///         - the second one is the number of successors
  /// syntax: new (nb_sons) _GDDD (constructor arguments)
  ///     it looks like a placement new, but this syntax
  ///     is only used to pass arguments to operator new
  /// _GDDD should only be constructed by create_unique_GDDD or clone
  /// please refer to these two functions for invokation examples
  static
  void *
  operator new (size_t, size_t length)
  {
    // allocate enough memory to store the successors
    // with the global (default) operator new
    return ::operator new (sizeof(_GDDD) + length*sizeof(edge_t));
  }

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

static size_t Max_DDD = 0;

#endif

/******************************************************************************/
/*                             class GDDD                                     */
/******************************************************************************/

/* Memory manager */
unsigned int GDDD::statistics() {
  return _GDDD::get_canonical().size();
}

// Todo
void GDDD::mark()const{
  concret->mark();
}

void _GDDD::mark()const{
  if(! is_marked()){
    set_mark(true);
    for(const_iterator vi=begin();vi!=end();++vi){
      vi->second.mark();
    }
  }
}

size_t GDDD::peak() {
  if (_GDDD::get_canonical().size() > Max_DDD) 
    Max_DDD=_GDDD::get_canonical().size();  

  return Max_DDD;
}


void GDDD::pstats(bool)
{
  std::cout << "Peak number of DDD nodes in unicity table :" << peak() << std::endl; 
  std::cout << "sizeof(_GDDD):" << sizeof(_GDDD) << std::endl;
  
#ifdef HASH_STAT
  std::cout << std::endl << "DDD Unicity table stats :" << std::endl;
  print_hash_stats(canonical.get_hits(), canonical.get_misses(), canonical.get_bounces());
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

GDDD::GDDD(const _GDDD *_g):concret(_g){}


GDDD::GDDD(int variable,const Valuation & value){
#ifdef EVDDD
  if (variable != DISTANCE) {
#endif
    concret=(value.size()!=0)? _GDDD::create_unique_GDDD(variable, value, 0): null.concret;
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
  return concret->valuation_size;
}

GDDD::const_iterator GDDD::begin() const{
  return concret->begin();
}

GDDD::const_iterator GDDD::end() const{
  return concret->end();
}

/* Visualisation */
unsigned int GDDD::refCounter() const{
  return concret->refCounter();
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
  if (_GDDD::get_canonical().size() > Max_DDD) 
    Max_DDD=_GDDD::get_canonical().size();  

  MyNbStates::clear();

  for(UniqueTable<_GDDD>::Table::iterator di=_GDDD::get_canonical().table.begin();di!=_GDDD::get_canonical().table.end();++di){
    (*di)->mark_if_refd();
  }

  // sweep phase
  
  for(UniqueTable<_GDDD>::Table::iterator di=_GDDD::get_canonical().table.begin();di!=_GDDD::get_canonical().table.end();){
    if(!((*di)->is_marked())){
      UniqueTable<_GDDD>::Table::iterator ci=di;
      di++;
      const _GDDD *g=(*ci);
      _GDDD::get_canonical().table.erase(ci);
      delete g;
    }
    else{
      (*di)->set_mark(false);
      di++;
    }
  }
}




/* Constants */
const GDDD GDDD::one(_GDDD::create_unique_GDDD(1,GDDD::Valuation(),1));
const GDDD GDDD::null(_GDDD::create_unique_GDDD(0,GDDD::Valuation(),1));
const GDDD GDDD::top(_GDDD::create_unique_GDDD(-1,GDDD::Valuation(),1));

/******************************************************************************/
/*                   class DDD:public GDDD                                    */
/******************************************************************************/

DDD::DDD(const DDD &g):GDDD(g.concret),DataSet(){
  concret->ref();
}

DDD::DDD(const GDDD &g):GDDD(g.concret){
  concret->ref();
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
    concret=_GDDD::create_unique_GDDD(var,tmp,0);
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
    concret=_GDDD::create_unique_GDDD(var,tmp,0);
  }
  //  concret->refCounter++;
}

DDD::DDD(int var,int val,const GDDD &d):GDDD(var,val,d){
  concret->ref();
}

DDD::DDD(int var,int val1,int val2,const GDDD &d):GDDD(var,val1,val2,d){
  concret->ref();
}

DDD::~DDD(){
  assert(concret->refCounter()>0);
  concret->ref();
}

DDD &DDD::operator=(const GDDD &g){
  concret->deref();
  concret=g.concret;
  concret->ref();
  return *this;
}

DDD &DDD::operator=(const DDD &g){
  concret->deref();
  concret=g.concret;
  concret->ref();
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
                for (GDDD::const_iterator vi=begin();vi!=end();++vi) 
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


