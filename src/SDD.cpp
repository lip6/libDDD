/* -*- C++ -*- */
#include <string>
#include <iostream>
#include <vector>
#include <set>
// modif
#include <ext/hash_map>
#include <map>
// modif
#include <sstream>
// ajout
#include <cassert>

using namespace std;
using namespace __gnu_cxx;

#include "SDED.h"
#include "SDD.h"
#include "UniqueTable.h"

/******************************************************************************/
/*                             class _GSDD                                     */
/******************************************************************************/

#ifdef INST_STL
static long long NBJumps=0;
static long long NBAccess=0;
#endif

class _GSDD{
public:
  /* Attributs*/
  int variable;
  GSDD::Valuation valuation;
  mutable unsigned long int refCounter;
  mutable bool marking;

  /* Constructor */
  _GSDD(int var,int cpt=0):variable(var),refCounter(cpt),marking(false){}; 
  _GSDD(int var,GSDD::Valuation val,int cpt=0):variable(var),valuation(val),refCounter(cpt),marking(false){}; 

  /* Compare */
  bool operator==(const _GSDD& g) const 
  { 
    if (variable!=g.variable || valuation.size()!= g.valuation.size()) 
      return false;  
   
    for (GSDD::Valuation::const_iterator it = valuation.begin(),jt=g.valuation.begin(); it != valuation.end() && jt != g.valuation.end() ; it++,jt++ )
      if (!(it->first->set_equal(*jt->first) && it->second == jt->second))
	return false;
    return true;
  }


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
  struct hash<_GSDD*> {
    size_t operator()(_GSDD *g) const{
      size_t res=(size_t) g->variable;
      for(GSDD::const_iterator vi=g->valuation.begin();vi!=g->valuation.end();vi++)
        res ^=   vi->first->set_hash()  
	      +  hash<GSDD>()(vi->second)  ;
      return res;
    }
  };
}

namespace std {
  struct equal_to<_GSDD*> {
    bool operator()(_GSDD *g1,_GSDD *g2) const{
      return *g1==*g2;
    }
  };
}

// map<int,string> mapVarName;
static size_t Max_SDD=0;
static UniqueTable<_GSDD> canonical;

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
  if(!marking){
    marking=true;
    for(GSDD::Valuation::const_iterator vi=valuation.begin();vi!=valuation.end();vi++){
      vi->second.mark();
    }
  }
}

size_t GSDD::peak() {
 if (canonical.size() > Max_SDD) 
    Max_SDD=canonical.size();  

  return Max_SDD;
}

void GSDD::pstats(bool reinit)
{
#ifdef INST_STL
  cout << "*\nGSDD : size unicity table =" << canonical.size() << endl;
  cout << "  Average nb jump in hash table : " << _GSDD::StatJumps() << endl;
  if (reinit){
    _GSDD::ResetNbJumps();
  }
  canonical.pstat(reinit);
#endif
}



/* Visualisation*/
void GSDD::print(ostream& os,string s) const{
  if (*this == one)
    os << "[ " << s << "]"<<endl;
  else if(*this == top)
      os << "[ " << s << "T ]"<<endl;
  else if(*this == null)
      os << "[ " << s << "0 ]"<<endl;
  else{
    // should not happen
    assert ( begin() != end());

    for(GSDD::const_iterator vi=begin();vi!=end();vi++){
      stringstream tmp;
      // Fixme  for pretty print variable names
//      string varname = GDDD::getvarName(variable());
      tmp<<"var" << variable() <<  " " ;
      vi->first->set_print(tmp); 
      vi->second.print(os,s+tmp.str()+" ");
    }
  }
}
 

ostream& operator<<(ostream &os,const GSDD &g){
  string s;
  g.print(os,s);
  return(os);
}

GSDD::GSDD(_GSDD *_g):concret(_g){}

GSDD::GSDD(int variable,Valuation value){
  concret=(value.size()!=0)? canonical(new _GSDD(variable,value)): null.concret;
}



/* Accessors */
int GSDD::variable() const{
  return concret->variable;
}

size_t GSDD::nbsons () const { 
  return concret->valuation.size();
}

GSDD::const_iterator GSDD::begin() const{
  return concret->valuation.begin();
}

GSDD::const_iterator GSDD::end() const{
  return concret->valuation.end();
}

/* Visualisation */
unsigned int GSDD::refCounter() const{
  return concret->refCounter;
}

class SddSize{
private:
  set<GSDD> s;
  // Was used to compute number of nodes in referenced datasets as well
  // but dataset doesn't define what we need as it is not necessarily 
  // a decision diagram implementation => number of nodes = ??
//   set<DataSet &> sd3;
  // trying to repair it : consider we reference only SDD or DDD for now, corresponds to current usage patterns
  set<GDDD> sd3;

  bool firstError;

  void sddsize(const GSDD& g){
    if(s.find(g)==s.end()){
      s.insert(g);
      res++;
      for(GSDD::const_iterator gi=g.begin();gi!=g.end();gi++) 
	sddsize(gi->first);
      
      for(GSDD::const_iterator gi=g.begin();gi!=g.end();gi++)
	sddsize(gi->second);
      
    }
  }

  virtual void sddsize(const DataSet* g){
    // Used to work for referenced DDD
    if (typeid(*g) == typeid(SDD) ) {
      sddsize( GSDD ((SDD &) *g) );
    } else if (typeid(*g) == typeid(DDD)) {
      sddsize( GDDD ((DDD &) *g) );
    } else {
      if (firstError) {
	cerr << "Warning : unkown referenced dataset type on arc, node count is inacurate"<<endl;
	cerr << "Read type :" << typeid(*g).name() <<endl ;
	firstError = false;
      }
    }
  }

  virtual void sddsize(const GDDD& g){
      if (sd3.find(g)==sd3.end()) {
	sd3.insert(g);
	d3res ++;
	for(GDDD::const_iterator gi=g.begin();gi!=g.end();gi++)
	  sddsize(gi->second);
      }
  }

public:
  unsigned long int res;
  unsigned long int d3res;

  SddSize():firstError(true){};
//  pair<unsigned long int,unsigned long int> operator()(const GSDD& g){
  unsigned long int operator()(const GSDD& g){
    res=0;
    d3res=0;
    sd3.clear();
    s.clear();
    sddsize(g);
    // we used to return a pair : number of nodes in SDD, number of nodes in referenced data structures
//    return make_pair(res,d3res);
    return res;
  }
};


pair<unsigned long int,unsigned long int> GSDD::node_size() const{
  static SddSize sddsize;
  sddsize(*this);
  return make_pair(sddsize.res,sddsize.d3res);
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
  static hash_map<GSDD,long double> s;

  long double nbStates(const GSDD& g){
    if(g==GSDD::one)
      return 1;
    else if(g==GSDD::top || g==GSDD::null)
      return 0;
    else{
      hash_map<GSDD,long double>::const_iterator i=s.find(g);
      if(i==s.end()){
	long double res=0;
	for(GSDD::const_iterator gi=g.begin();gi!=g.end();gi++)
	  res+=(gi->first->set_size())*nbStates(gi->second)+val;
	s[g]=res;
	return res;
      } else {
	return i->second;
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
    s.clear();
  }
};

hash_map<GSDD,long double> MySDDNbStates::s = hash_map<GSDD,long double> ();

long double GSDD::nbStates() const{
  static MySDDNbStates myNbStates(0);
  return myNbStates(*this);
}


void GSDD::garbage(){
  if (canonical.size() > Max_SDD) 
    Max_SDD=canonical.size();  

  MySDDNbStates::clear();


  // mark phase
  for(UniqueTable<_GSDD>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();di++){
    if((*di)->refCounter!=0)
      (*di)->mark();
  }

  // sweep phase  
  for(UniqueTable<_GSDD>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();){
    if(! (*di)->marking){
      UniqueTable<_GSDD>::Table::iterator ci=di;
      di++;
      _GSDD *g=(*ci);
      canonical.table.erase(ci);
      delete g;
    }
    else{
      (*di)->marking=false;
      di++;
    }
  }
}


// FIXME
// long double GSDD::noSharedSize() const{
//   static MyNbStates myNbStates(1);
//   return myNbStates(*this);
// }

/* Constants */
const GSDD GSDD::one(canonical(new _GSDD(1,1)));
const GSDD GSDD::null(canonical(new _GSDD(0,1)));
const GSDD GSDD::top(canonical(new _GSDD(-1,1)));

/******************************************************************************/
/*                   class SDD:public GSDD                                    */
/******************************************************************************/

SDD::SDD(const SDD &g):GSDD(g.concret){
  (concret->refCounter)++;
}

SDD::SDD(const GSDD &g):GSDD(g.concret){
  (concret->refCounter)++;
}

GSDD::GSDD(int var,const DataSet &val,const GSDD &d):concret(null.concret){ //var-val->d
  if(d!=null && ! val.empty() ){
    _GSDD *_g = new _GSDD(var,0);
    // cast to (DataSet*) to lose "const" type
    pair<DataSet *, GSDD> x( val.newcopy(),d);
    _g->valuation.push_back(x);
    concret=canonical(_g);
  }
  //  concret->refCounter++;
}


SDD::SDD(int var,const DataSet& val,const GSDD &d):GSDD(var,val,d){
  concret->refCounter++;
}
SDD::~SDD(){
  assert(concret->refCounter>0);
  concret->refCounter--;
}

SDD &SDD::operator=(const GSDD &g){
  concret->refCounter--;
  concret=g.concret;
  concret->refCounter++;
  return *this;
}

SDD &SDD::operator=(const SDD &g){
  concret->refCounter--;
  concret=g.concret;
  concret->refCounter++;
  return *this;
}


// DataSet interface

DataSet *SDD::set_intersect (const DataSet & b) const {
  return new SDD((*this) * (SDD&)b );
}
DataSet *SDD::set_union (const DataSet & b)  const {
  return new SDD(*this + (SDD&)b);
}
DataSet *SDD::set_minus (const DataSet & b) const {
  return new SDD(*this - (SDD&)b);
}

bool SDD::empty() const {
  return *this == GSDD::null;
}

DataSet * SDD::empty_set() const {
  return new SDD();
}

bool SDD::set_equal(const DataSet & b) const {
  return *this == (SDD&) b;
}

long double SDD::set_size() const { return nbStates(); }

size_t SDD::set_hash() const {
  return size_t (concret);
}

