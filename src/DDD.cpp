/* -*- C++ -*- */
#include <string>
#include <iostream>
#include <vector>
#include <set>
// modif
#include <cassert>
//#include <ext/hash_map>
#include <map>
// modif
#include <sstream>

#include "DDD.h"
#include "UniqueTable.h"
#include "DED.h"

#ifdef PARALLEL_DD
#include "tbb/atomic.h"
#endif

#include "Cache.h"

/******************************************************************************/
/*                             class _GDDD                                     */
/******************************************************************************/

#ifdef INST_STL
static long long NBJumps=0;
static long long NBAccess=0;
#endif

class _GDDD{
public:
  /* Attributs*/
  int variable;
  GDDD::Valuation valuation;
  mutable unsigned long int refCounter;
  mutable bool marking;

  /* Constructor */
  _GDDD(int var,int cpt=0):variable(var),refCounter(cpt),marking(false){}; 
  _GDDD(int var,GDDD::Valuation val,int cpt=0):variable(var),valuation(val),refCounter(cpt),marking(false){}; 

  /* Compare */
  bool operator==(const _GDDD& g) const{return variable==g.variable && valuation==g.valuation;};

  /* Memory Manager */
  void mark()const;


#ifdef INST_STL
  //for use with instrumented hash tables in STL
  static void InstrumentNbJumps(int nbjmp){NBJumps+=(1+nbjmp);NBAccess++;}
  static void ResetNbJumps(){NBJumps=0; NBAccess=0;}
  static double StatJumps() {if (NBAccess!=0)  return double(NBJumps) / double(NBAccess); return -1;}
#endif
};

/******************************************************************************/
// to be revised !!!

namespace __gnu_cxx {
  template<>
  struct hash<_GDDD*> {
    size_t operator()(_GDDD *g) const{
      size_t res=(size_t) g->variable;
      for(GDDD::const_iterator vi=g->valuation.begin();vi!=g->valuation.end();++vi)
        res+=(size_t)(vi->first+1011)* vi->second.hash();
      return res;
    }
  };
}

namespace std {
  template<>
  struct equal_to<_GDDD*> {
    bool operator()(_GDDD *g1,_GDDD *g2) const{
      return *g1==*g2;
    }
  };
}

static UniqueTable<_GDDD> canonical;
std::map<int,std::string> mapVarName;

#ifdef PARALLEL_DD

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


void GDDD::pstats(bool reinit)
{
  std::cout << "Peak number of DDD nodes in unicity table :" << peak() << std::endl; 
#ifdef INST_STL
  std::cout << "*\nGDDS : size unicity table =" << canonical.size() << std::endl;
  std::cout << "  Average nb jump in hash table : " << _GDDD::StatJumps() << std::endl;
  if (reinit){
    _GDDD::ResetNbJumps();
  }
  canonical.pstat(reinit);
#endif
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
    std::string val;
    for(GDDD::const_iterator vi=begin();vi!=end();++vi){
      // modif strstream -> std::stringstream
      std::stringstream tmp;
      tmp << getvarName(variable())<<'('<<vi->first<<")";
      tmp >> val;
      vi->second.print(os,s+val+" ");
    }
  }
}

std::ostream& operator<<(std::ostream &os,const GDDD &g){
  std::string s;
  g.print(os,s);
  return(os);
}

GDDD::GDDD(_GDDD *_g):concret(_g){}

GDDD::GDDD(int variable,Valuation value){
#ifdef EVDDD
  if (variable != DISTANCE) {
#endif
    concret=(value.size()!=0)? canonical(new _GDDD(variable,value)): null.concret;
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
  unsigned long int res;
  std::set<GDDD> s;
  void mysize(const GDDD& g)
  {
    if(s.find(g)==s.end())
	{
		s.insert(g);
		res++;
		for(GDDD::const_iterator gi=g.begin();gi!=g.end();++gi)
			mysize(gi->second);
    }
  }

public:
  MySize()
#ifdef PARALLEL_DD
	:
	res(0),
	s()
#endif	
	{};
  unsigned long int operator()(const GDDD& g){
#ifndef PARALLEL_DD
    res=0;
    s.clear();
#endif
    mysize(g);
    return res;
  }
};

unsigned long int GDDD::size() const{
#ifndef PARALLEL_DD
 static	
#endif
  MySize mysize;

  return mysize(*this);
}

class MyNbStates{
private:
  int val; // val=0 donne nbState , val=1 donne noSharedSize
//  static hash_map<GDDD,long double> s;
  //static __gnu_cxx::hash_map<GDDD,long double> s;
	typedef __Cache<GDDD, long double> s_t;
	static s_t s;

long double nbStates(const GDDD& g){

	if(g==GDDD::one)
		return 1;
	else if(g==GDDD::top || g==GDDD::null)
		return 0;
	else{
//		__gnu_cxx::hash_map<GDDD,long double>::const_iterator i=s.find(g);
		s_t::const_iterator i = s.find(g);
		if(i==s.end()){
			long double res=0;
			for(GDDD::const_iterator gi=g.begin();gi!=g.end();++gi)
				res+=nbStates(gi->second)+val;
			s[g]=res;
			return res;
		}
		else{
			return i->second;
		}
// end of lock
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
    s.clear();
  }
};

//__gnu_cxx::hash_map<GDDD,long double> MyNbStates::s = __gnu_cxx::hash_map<GDDD,long double> ();
__Cache<GDDD,long double> MyNbStates::s = __Cache<GDDD,long double> ();

long double GDDD::nbStates() const{
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
      _GDDD *g=(*ci);
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
const GDDD GDDD::one(canonical(new _GDDD(1,1)));
const GDDD GDDD::null(canonical(new _GDDD(0,1)));
const GDDD GDDD::top(canonical(new _GDDD(-1,1)));

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
    _GDDD *_g = new _GDDD(var,0);
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
    _g->valuation.push_back(x);
    concret=canonical(_g);
  }
  //  concret->refCounter++;
}

GDDD::GDDD(int var,int val1,int val2,const GDDD &d):concret(null.concret){ //var-[val1,val2]->d
  if(val1<=val2 && null!=d){
    _GDDD *_g = new _GDDD(var,0);
    for(int val=val1;val<=val2;++val){
      std::pair<int,GDDD> x(val,d);
      _g->valuation.push_back(x);
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
  
long double DDD::set_size() const { return nbStates(); }

size_t DDD::set_hash() const {
  return hash();
}




//My funs

unsigned long int GDDD::nodeIndex(std::vector<_GDDD*> list) const{
    assert(this);
    assert(concret);
    unsigned long int i=0;
    for (i=0; i<list.size();++i)
      if (concret==list[i]) return i;
    return ULONG_MAX;
    assert (false);
}


void GDDD::saveNode(std::ostream& os, std::vector<_GDDD*>& list)const {
    assert(this);
    //assert(concret);
    unsigned long int index = nodeIndex(list);
    if (index==ULONG_MAX) {

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
  std::vector<_GDDD*> SavedDDD;
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



