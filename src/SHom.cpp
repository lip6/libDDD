#include <typeinfo>
#include <cassert>
using namespace std;
#include "Hom.h"
#include "DDD.h"
#include "DED.h"
#include "SHom.h"
#include "SDD.h"
#include "SDED.h"
#include "UniqueTable.h"
#include "DataSet.h"

#ifdef INST_STL
MapJumps  _GShom::HomJumps;
#endif


namespace namespace_SHom {
#ifdef INST_STL
static string TryDemangle(string in)
{
  string res=string("");
  unsigned int idx=0;
  int state=0;
  int len=0;
  bool err=false;
  int depth=0;
  
  while ((idx<in.length())&&(!err)){
    switch(state){
    case 0 : //beginning
      if ((in[idx]>='0')&&(in[idx]<='9')){
	// decode length
	len=10*len+(in[idx]-'0');
      }
      else {
	// first char after length
	res=res+in[idx];
	len--;
	state=1;
      }
      break;


    case 10 : //beginning next (handle 'L')
      if ((in[idx]>='0')&&(in[idx]<='9')){
	// decode length
	len=10*len+(in[idx]-'0');
      }
      else 
	if (in[idx]=='L'){
	  state=11;
	  depth++;
	}
      else
	{
	  // first char after length
	  res=res+in[idx];
	  len--;
	  state=1;
	}
      break;
      
      
    case 1:
      if (len==0){
	// of name 
	if ((idx<in.length())&&(in[idx]!='I')&&(in[idx]!='E')){
	  err=true;
	}
	else 
	  if (in[idx]=='I') {
	    res=res+'<';
	    state=10;
	    len=0;
	    depth++;
	  }
	  else if (in[idx]=='E') {
	    depth--;
	    res=res+'>';
	  }
	  else {
	    err=true;
	  }
      }
      else {
	res=res+in[idx];
	len--;
      }

      break;

    case 11:
      if (in[idx]=='i'){
	// ok
      }
      else 
      if (in[idx]=='E'){
	// end 
	depth--;
	state=1;
      }
      else 
      if ((in[idx]>='0')&&(in[idx]<='9')) {
	res=res+in[idx];
      }
      else {
	err=true;
      }
      break;

    }
    idx++;
  }
  
  if ((depth!=0)||(err)){
    return in;
  }
  else {
    return res;
  }
   
}
#endif


void PrintMapJumps(double ratemin=0){
#ifdef INST_STL
  MapJumps::iterator ii;
  for (ii=_GHom::HomJumps.begin(); ii!=_GHom::HomJumps.end(); ii++){
    double rate= double(ii->second.second)/double (ii->second.first);
    if (rate>ratemin){
      cout << "SHom " << TryDemangle(ii->first) << "\t-->\t\t" << ii->second.second  <<"/" ;
      cout <<  ii->second.first << "= " << rate  << endl; 
    }
  }
#endif
}

} //end namespace namespace_SHom 
using namespace namespace_SHom ;

/* Unique Table */
namespace __gnu_cxx {
  template<>  struct hash<_GShom*>{
    size_t operator()(_GShom * _h) const{
      return _h->hash();
    }
  };
}

namespace std {
  template<>  struct equal_to<_GShom*>{
    bool operator()(_GShom * _h1,_GShom * _h2){
      return (typeid(*_h1)==typeid(*_h2)?(*_h1)==(*_h2):false);
    }
  };
}

static UniqueTable<_GShom> canonical;
/*************************************************************************/
/*                         Class _GShom                                   */
/*************************************************************************/

namespace S_Homomorphism {
/************************** Identity */
class Identity:public _GShom{
public:
  /* Constructor */
  Identity(int ref=0):_GShom(ref,true){}

  /* Compare */
  bool operator==(const _GShom &h) const{return true;}
  size_t hash() const{return 17;}

  /* Eval */
  GSDD eval(const GSDD &d)const{return d;}
};

/************************** Constant */
class Constant:public _GShom{
private:
  GSDD value;
public:
  /* Constructor */
  Constant(const GSDD &d,int ref=0):_GShom(ref,true),value(d){}

  /* Compare */
  bool operator==(const _GShom &h) const{
    return value==((Constant*)&h )->value;
  }
  size_t hash() const{
    return ::hash<GSDD>()(value);
  }

  /* Eval */
  GSDD eval(const GSDD &d)const{
    return d==GSDD::null?GSDD::null:value;
  }

  /* Memory Manager */
  void mark() const{
    value.mark();
  }
};

/************************** Mult */
class Mult:public _GShom{
private:
  GShom left;
  GSDD right;
public:
  /* Constructor */
  Mult(const GShom &l,const GSDD &r,int ref=0):_GShom(ref),left(l),right(r){}
  /* Compare */
  bool operator==(const _GShom &h) const{
    return left==((Mult*)&h )->left && right==((Mult*)&h )->right;
  }
  size_t hash() const{
    return 83*::hash<GShom>()(left)+53*::hash<GSDD>()(right);
  }

  /* Eval */
  GSDD eval(const GSDD &d)const{
    return left(d)*right;
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }
};

/************************** Add */
/************************** Add */
class Add:public _GShom{
private:
//  set<GHom> parameters;
  GShom left;
  GShom right;
public:
  /* Constructor */
  Add(const GShom &l, const GShom &r, int ref=0):_GShom(ref,true),left(l),right(r){}
  /* Compare */
  bool operator==(const _GShom &h) const{
    return left==((Add*)&h )->left && right==((Add*)&h )->right;
  }
  size_t hash() const{
    return 1039*::hash<GShom>()(left)+1049*::hash<GShom>()(right);
  }

  /* Eval */
  GSDD eval(const GSDD &d)const{
    int variable=d.variable();
    set<GSDD> s;
    GSDD cur;
    for(GSDD::const_iterator vi=d.begin();vi!=d.end();vi++){
      cur= GSDD(variable,*vi->first,vi->second);
      s.insert(left(cur));
      s.insert(right(cur));
    }
    return SDED::add(s);
//    return left(d) + right(d);
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }
};

// class Sum:public _GShom{
// private:
//   set<GShom> parameters;
// public:
//   /* Constructor */
//   Sum(const set<GShom> &param,int ref=0):_GShom(ref,true),parameters(param){}
//   /* Compare */
//   bool operator==(const _GShom &h) const{
//     return parameters==((Sum*)&h )->parameters;
//   }
//   size_t hash() const{
//     size_t res=0;
//     for(set<GShom>::const_iterator gi=parameters.begin();gi!=parameters.end();gi++)
//       res^=::hash<GShom>()(*gi);
//     return res;
//   }

//   /* Eval */
//   GSDD eval(const GSDD &d)const{
//      set<GSDD> s;
//      for(set<GShom>::const_iterator gi=parameters.begin();gi!=parameters.end();gi++)
//        s.insert((*gi)(d));
//      return HDED::add(s);
//   }

//   /* Memory Manager */
//   void mark() const{
//     for(set<GShom>::const_iterator gi=parameters.begin();gi!=parameters.end();gi++)
//       gi->mark();
//   }
// };
/************************** Compose */
class Compose:public _GShom{
private:
  GShom left;
  GShom right;
public:
  /* Constructor */
  Compose(const GShom &l,const GShom &r,int ref=0):_GShom(ref,true),left(l),right(r){}
  /* Compare */
  bool operator==(const _GShom &h) const{
    return left==((Compose*)&h )->left && right==((Compose*)&h )->right;
  }
  size_t hash() const{
    return 13*::hash<GShom>()(left)+7*::hash<GShom>()(right);
  }

  /* Eval */
  GSDD eval(const GSDD &d)const{
    return left(right(d));
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }

};

/************************** LeftConcat */
class LeftConcat:public _GShom{
private:
  GSDD left;
  GShom right;
public:
  /* Constructor */
  LeftConcat(const GSDD &l,const GShom &r,int ref=0):_GShom(ref,true),left(l),right(r){}
  /* Compare */
  bool operator==(const _GShom &h) const{
    return left==((LeftConcat*)&h )->left && right==((LeftConcat*)&h )->right;
  }
  size_t hash() const{
    return 23*::hash<GSDD>()(left)+47*::hash<GShom>()(right);
  }

  /* Eval */
  GSDD eval(const GSDD &d)const{
    return left^right(d);
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }

};

/************************** RightConcat */
class RightConcat:public _GShom{
private:
  GShom left;
  GSDD right;
public:
  /* Constructor */
  RightConcat(const GShom &l,const GSDD &r,int ref=0):_GShom(ref,true),left(l),right(r){}
  /* Compare */
  bool operator==(const _GShom &h) const{
    return left==((RightConcat*)&h )->left && right==((RightConcat*)&h )->right;
  }
  size_t hash() const{
    return 47*::hash<GShom>()(left)+19*::hash<GSDD>()(right);
  }

  /* Eval */
  GSDD eval(const GSDD &d)const{
    return left(d)^right;
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }
};

/************************** Minus */
class Minus:public _GShom{
private:
  GShom left;
  GSDD right;
public:
  /* Constructor */
  Minus(const GShom &l,const GSDD &r,int ref=0):_GShom(ref),left(l),right(r){}
  /* Compare */
  bool operator==(const _GShom &h) const{
    return left==((Minus*)&h )->left && right==((Minus*)&h )->right;
  }
  size_t hash() const{
    return 5*::hash<GShom>()(left)+61*::hash<GSDD>()(right);
  }

  /* Eval */
  GSDD eval(const GSDD &d)const{
    return left(d)-right;
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }
};

/************************** Fixpoint */
class Fixpoint:public _GShom{
private:
  GShom arg;
public:
  /* Constructor */
  Fixpoint(const GShom &a,int ref=0):_GShom(ref),arg(a){}
  /* Compare */
  bool operator==(const _GShom &h) const{
    return arg==((Fixpoint*)&h )->arg ;
  }
  size_t hash() const{
    return 17*::hash<GShom>()(arg);
  }

  /* Eval */
  GSDD eval(const GSDD &d)const{
    int variable=d.variable();
    set<GSDD> s;
    GSDD cur,cur2;
    for(GSDD::const_iterator vi=d.begin();vi!=d.end();vi++){
      cur= GSDD(variable,*vi->first,vi->second);
      do { 
	cur2=cur; 
	cur=arg(cur);
      } while (cur != cur2);
      s.insert(cur);
    }

    return SDED::add(s);

//     GSDD d1=d,d2=d;
//     do {
//       d1=d2;
//       d2=arg(d2);
//     } while (d1 != d2);

//     return d1;
  }

  /* Memory Manager */
  void mark() const{
    arg.mark();
  }
};




} // end namespace H_Homomorphism

using namespace S_Homomorphism;

/*************************************************************************/
/*                         Class StrongShom                               */
/*************************************************************************/

/* Compare */
bool StrongShom::operator==(const _GShom &h) const{
  return typeid(*this)==typeid(h)?*this==*(StrongShom*)&h:false;
}


/* Eval */
GSDD StrongShom::eval(const GSDD &d)const{
  if(d==GSDD::null)
    return GSDD::null;
  else if(d==GSDD::one)
    return phiOne();
  else if(d==GSDD::top)
    return GSDD::top;
  else{
    int variable=d.variable();
    set<GSDD> s;

    for(GSDD::const_iterator vi=d.begin();vi!=d.end();vi++){
      s.insert(phi(variable,*vi->first)(vi->second));
    }
    return SDED::add(s);
  }
}

/*************************************************************************/
/*                         Class GShom                                    */
/*************************************************************************/

/* Constructor */
GShom::GShom(const StrongShom *h):concret(h){}

GShom::GShom(StrongShom *h):concret(canonical(h)){}

GShom::GShom(const GSDD& d):concret(canonical(new Constant(d))){}

GShom::GShom(int var,const DataSet & val, const GShom &h):concret(canonical(new LeftConcat(GSDD(var,val),h))){}

/* Eval */
GSDD GShom::operator()(const GSDD &d) const{
  if(concret->immediat)
    return concret->eval(d);
  else
    return SDED::Shom(*this,d);
}

GSDD GShom::eval(const GSDD &d) const{
  return concret->eval(d);
}

const GShom GShom::id(canonical(new Identity(1)));

int GShom::refCounter() const{return concret->refCounter;}

/* Sum */

// GShom GShom::add(const set<GShom>& s){
//    return(new Sum(s));
// }


/* Memory Manager */
unsigned int GShom::statistics(){
  return canonical.size();
}

// Todo
void GShom::mark()const{
  if(!concret->marking){
    concret->marking=true;
    concret->mark();
  }
};

void GShom::garbage(){
  // mark phase
  for(UniqueTable<_GShom>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();di++){
    if((*di)->refCounter!=0){
      (*di)->marking=true;
      (*di)->mark();
    }
  }
  // sweep phase
  for(UniqueTable<_GShom>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();){
    if(!(*di)->marking){
      UniqueTable<_GShom>::Table::iterator ci=di;
      di++;
      _GShom *g=*ci;
      canonical.table.erase(ci);
      delete g;
    }
    else{
      (*di)->marking=false;
      di++;
    }
  }
}

/*************************************************************************/
/*                    Class Shom                                          */
/*************************************************************************/
/* Constructor */
Shom::Shom(const Shom &h):GShom(h.concret){
  concret->refCounter++;
}

Shom::Shom(const GShom &h):GShom(h.concret){
  concret->refCounter++;
}

Shom::Shom(const GSDD& d):GShom(d){
  concret->refCounter++;
}

Shom::Shom(int var,const DataSet &  val, const GShom &h):GShom(var,val,h){
  concret->refCounter++;
}

Shom::~Shom(){
  assert(concret->refCounter>0);
  concret->refCounter--;
}

/* Set */

Shom &Shom::operator=(const Shom &h){
  assert(concret->refCounter>0);
  concret->refCounter--;
  concret=h.concret;
  concret->refCounter++;
  return *this;
}

Shom &Shom::operator=(const GShom &h){
  assert(concret->refCounter>0);
  concret->refCounter--;
  concret=h.concret;
  concret->refCounter++;
  return *this;
}

/* Operations */
GShom fixpoint (const GShom &h) {
  return GShom(canonical(new Fixpoint(h)));
}

GShom operator&(const GShom &h1,const GShom &h2){
  return GShom(canonical(new Compose(h1,h2)));
}

GShom operator+(const GShom &h1,const GShom &h2){
  if (h1 < h2) 
    return GShom(canonical(new Add(h1,h2)));
  else
    return GShom(canonical(new Add(h2,h1)));
//   set<GShom> s;
//   s.insert(h1);
//   s.insert(h2);
// //  return(new Add(s));
//   return GShom(canonical(new Add(s)));
}

GShom operator*(const GSDD &d,const GShom &h){
  return GShom(canonical(new Mult(h,d)));
}

GShom operator*(const GShom &h,const GSDD &d){
  return GShom(canonical(new Mult(h,d)));
}

GShom operator^(const GSDD &d,const GShom &h){
  return GShom(canonical(new LeftConcat(d,h)));
}

GShom operator^(const GShom &h,const GSDD &d){
  return GShom(canonical(new RightConcat(h,d)));
}

GShom operator-(const GShom &h,const GSDD &d){
  return GShom(canonical(new Minus(h,d)));
}


void GShom::pstats(bool reinit)
{
  cout << "*\nGSHom Stats : size unicity table = " <<  canonical.size() << endl;
#ifdef INST_STL
  canonical.pstat(reinit);
  
  cout << "\n ----------------  MAPPING Jumps on GHOM --------------" << endl;
  PrintMapJumps();
  if (reinit){
    cout << "\n -----  END MAPPING Jumps on GHOM, reseting table -----" << endl;
    _GShom::HomJumps.clear();
  }
  else
    {
      cout << "\n -----  END MAPPING Jumps on GHOM   --------" << endl; 
    }
#endif
}
