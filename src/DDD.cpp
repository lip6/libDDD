/* -*- C++ -*- */
#include <string>
#include <iostream>
#include <vector>
#include <set>
// modif
#include <assert.h>
#include <ext/hash_map>
#include <map>
// modif
#include <sstream>
// ajout
using namespace std;
using namespace __gnu_cxx;

#include "DDD.h"
#include "UniqueTable.h"

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
  struct hash<_GDDD*> {
    size_t operator()(_GDDD *g) const{
      size_t res=(size_t) g->variable;
      for(GDDD::const_iterator vi=g->valuation.begin();vi!=g->valuation.end();vi++)
        res+=(size_t)(vi->first+1011)* hash<GDDD>()(vi->second);
      return res;
    }
  };
}

namespace std {
  struct equal_to<_GDDD*> {
    bool operator()(_GDDD *g1,_GDDD *g2) const{
      return *g1==*g2;
    }
  };
}

map<int,string> mapVarName;

static UniqueTable<_GDDD> canonical;

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
    for(GDDD::Valuation::const_iterator vi=valuation.begin();vi!=valuation.end();vi++){
      vi->second.mark();
    }
  }
}



void GDDD::pstats(bool reinit)
{
#ifdef INST_STL
  cout << "*\nGDDS : size unicity table =" << canonical.size() << endl;
  cout << "  Average nb jump in hash table : " << _GDDD::StatJumps() << endl;
  if (reinit){
    _GDDD::ResetNbJumps();
  }
  canonical.pstat(reinit);
#endif
}



void GDDD::garbage(){
  // mark phase



  for(UniqueTable<_GDDD>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();di++){
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

/* Visualisation*/
void GDDD::print(ostream& os,string s) const{
  if (*this == one)
    os << "[ " << s << "]"<<endl;
  else if(*this == top)
      os << "[ " << s << "T ]"<<endl;
  else if(*this == null)
      os << "[ " << s << "0 ]"<<endl;
  else{
    string val;
    for(GDDD::const_iterator vi=begin();vi!=end();vi++){
      // modif strstream -> stringstream
      stringstream tmp;
      map<int,string>::iterator i=mapVarName.find(variable());
      if (i==mapVarName.end())
	tmp<<"var"<<variable()<<'('<<vi->first<<")";
      else
	tmp<<i->second<<'('<<vi->first<<")";
      tmp>>val;
      vi->second.print(os,s+val+" ");
    }
  }
}

ostream& operator<<(ostream &os,const GDDD &g){
  string s;
  g.print(os,s);
  return(os);
}

GDDD::GDDD(_GDDD *_g):concret(_g){}

GDDD::GDDD(int variable,Valuation value){
  concret=(value.size()!=0)? canonical(new _GDDD(variable,value)): null.concret;
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
  set<GDDD> s;
  void mysize(const GDDD& g){
    if(s.find(g)==s.end()){
      s.insert(g);
      res++;
      for(GDDD::const_iterator gi=g.begin();gi!=g.end();gi++)
	mysize(gi->second);
    }
  }
public:
  MySize(){};
  unsigned long int operator()(const GDDD& g){
    res=0;
    s.clear();
    mysize(g);
    return res;
  }
};

unsigned long int GDDD::size() const{
  static MySize mysize;
  return mysize(*this);
}

class MyNbStates{
private:
  int val; // val=0 donne nbState , val=1 donne noSharedSize
  hash_map<GDDD,long double> s;

  long double nbStates(const GDDD& g){
    if(g==GDDD::one)
      return 1;
    else if(g==GDDD::top || g==GDDD::null)
      return 0;
    else{
      hash_map<GDDD,long double>::const_iterator i=s.find(g);
      if(i==s.end()){
	long double res=0;
	for(GDDD::const_iterator gi=g.begin();gi!=g.end();gi++)
	  res+=nbStates(gi->second)+val;
	s[g]=res;
	return res;
      }
      else{
	return i->second;
      }
    }
  }

public:
  MyNbStates(int v):val(v){};
  long double operator()(const GDDD& g){
    long double res=nbStates(g);
    s.clear();
    return res;
  }
};

long double GDDD::nbStates() const{
  static MyNbStates myNbStates(0);
  return myNbStates(*this);
}

long double GDDD::noSharedSize() const{
  static MyNbStates myNbStates(1);
  return myNbStates(*this);
}

/* Constants */
const GDDD GDDD::one(canonical(new _GDDD(1,1)));
const GDDD GDDD::null(canonical(new _GDDD(0,1)));
const GDDD GDDD::top(canonical(new _GDDD(-1,1)));

/******************************************************************************/
/*                   class DDD:public GDDD                                    */
/******************************************************************************/

DDD::DDD(const DDD &g):GDDD(g.concret){
  (concret->refCounter)++;
}

DDD::DDD(const GDDD &g):GDDD(g.concret){
  (concret->refCounter)++;
}

GDDD::GDDD(int var,int val,const GDDD &d):concret(null.concret){ //var-val->d
  if(d!=null){
    _GDDD *_g = new _GDDD(var,0);
    pair<int,GDDD> x(val,d);
    _g->valuation.push_back(x);
    concret=canonical(_g);
  }
  //  concret->refCounter++;
}

GDDD::GDDD(int var,int val1,int val2,const GDDD &d):concret(null.concret){ //var-[val1,val2]->d
  if(val1<=val2 && null!=d){
    _GDDD *_g = new _GDDD(var,0);
    for(int val=val1;val<=val2;val++){
      pair<int,GDDD> x(val,d);
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
void GDDD::varName(int var,const string &name){
  ::mapVarName[var]=name;
}

const string GDDD::getvarName(int var)
{
  stringstream tmp;
  map<int,string>::iterator i=mapVarName.find(var);
  if (i==mapVarName.end())
    tmp<<"var"<< var ;
  else
    tmp<<i->second;

  string res;

  tmp >> res;
  return res;

}







