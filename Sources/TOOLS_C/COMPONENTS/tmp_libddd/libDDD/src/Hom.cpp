#include <typeinfo>
#include "Hom.h"
#include "DDD.h"
#include "DED.h"
#include "UniqueTable.h"

/* Unique Table */
namespace __gnu_cxx {
  struct hash<_GHom*>{
    size_t operator()(_GHom * _h) const{
      return _h->hash();
    }
  };
}

namespace std {
  struct equal_to<_GHom*>{
    bool operator()(_GHom * _h1,_GHom * _h2){
      return (typeid(*_h1)==typeid(*_h2)?(*_h1)==(*_h2):false);
    }
  };
}

static UniqueTable<_GHom> canonical;

/*************************************************************************/
/*                         Class _GHom                                   */
/*************************************************************************/

/************************** Identity */
class Identity:public _GHom{
public:
  /* Constructor */
  Identity(int ref=0):_GHom(ref,true){}

  /* Compare */
  bool operator==(const _GHom &h) const{return true;}
  size_t hash() const{return 17;}

  /* Eval */
  GDDD eval(const GDDD &d)const{return d;}
};

/************************** Constant */
class Constant:public _GHom{
private:
  GDDD value;
public:
  /* Constructor */
  Constant(const GDDD &d,int ref=0):_GHom(ref,true),value(d){}

  /* Compare */
  bool operator==(const _GHom &h) const{
    return value==((Constant*)&h )->value;
  }
  size_t hash() const{
    return ::hash<GDDD>()(value);
  }

  /* Eval */
  GDDD eval(const GDDD &d)const{
    return d==GDDD::null?GDDD::null:value;
  }

  /* Memory Manager */
  void mark() const{
    value.mark();
  }
};

/************************** Mult */
class Mult:public _GHom{
private:
  GHom left;
  GDDD right;
public:
  /* Constructor */
  Mult(const GHom &l,const GDDD &r,int ref=0):_GHom(ref),left(l),right(r){}
  /* Compare */
  bool operator==(const _GHom &h) const{
    return left==((Mult*)&h )->left && right==((Mult*)&h )->right;
  }
  size_t hash() const{
    return 83*::hash<GHom>()(left)+53*::hash<GDDD>()(right);
  }

  /* Eval */
  GDDD eval(const GDDD &d)const{
    return left(d)*right;
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }
};

/************************** Add */
class Add:public _GHom{
private:
  set<GHom> parameters;
public:
  /* Constructor */
  Add(const set<GHom> &param,int ref=0):_GHom(ref,true),parameters(param){}
  /* Compare */
  bool operator==(const _GHom &h) const{
    return parameters==((Add*)&h )->parameters;
  }
  size_t hash() const{
    size_t res=0;
    for(set<GHom>::const_iterator gi=parameters.begin();gi!=parameters.end();gi++)
      res^=::hash<GHom>()(*gi);
    return res;
  }

  /* Eval */
  GDDD eval(const GDDD &d)const{
    set<GDDD> s;
    for(set<GHom>::const_iterator gi=parameters.begin();gi!=parameters.end();gi++)
      s.insert((*gi)(d));
    return DED::add(s);
  }

  /* Memory Manager */
  void mark() const{
    for(set<GHom>::const_iterator gi=parameters.begin();gi!=parameters.end();gi++)
      gi->mark();
  }
};
/************************** Compose */
class Compose:public _GHom{
private:
  GHom left;
  GHom right;
public:
  /* Constructor */
  Compose(const GHom &l,const GHom &r,int ref=0):_GHom(ref,true),left(l),right(r){}
  /* Compare */
  bool operator==(const _GHom &h) const{
    return left==((Compose*)&h )->left && right==((Compose*)&h )->right;
  }
  size_t hash() const{
    return 13*::hash<GHom>()(left)+7*::hash<GHom>()(right);
  }

  /* Eval */
  GDDD eval(const GDDD &d)const{
    return left(right(d));
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }

};

/************************** LeftConcat */
class LeftConcat:public _GHom{
private:
  GDDD left;
  GHom right;
public:
  /* Constructor */
  LeftConcat(const GDDD &l,const GHom &r,int ref=0):_GHom(ref,true),left(l),right(r){}
  /* Compare */
  bool operator==(const _GHom &h) const{
    return left==((LeftConcat*)&h )->left && right==((LeftConcat*)&h )->right;
  }
  size_t hash() const{
    return 23*::hash<GDDD>()(left)+47*::hash<GHom>()(right);
  }

  /* Eval */
  GDDD eval(const GDDD &d)const{
    return left^right(d);
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }

};

/************************** RightConcat */
class RightConcat:public _GHom{
private:
  GHom left;
  GDDD right;
public:
  /* Constructor */
  RightConcat(const GHom &l,const GDDD &r,int ref=0):_GHom(ref,true),left(l),right(r){}
  /* Compare */
  bool operator==(const _GHom &h) const{
    return left==((RightConcat*)&h )->left && right==((RightConcat*)&h )->right;
  }
  size_t hash() const{
    return 47*::hash<GHom>()(left)+19*::hash<GDDD>()(right);
  }

  /* Eval */
  GDDD eval(const GDDD &d)const{
    return left(d)^right;
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }
};

/************************** Minus */
class Minus:public _GHom{
private:
  GHom left;
  GDDD right;
public:
  /* Constructor */
  Minus(const GHom &l,const GDDD &r,int ref=0):_GHom(ref),left(l),right(r){}
  /* Compare */
  bool operator==(const _GHom &h) const{
    return left==((Minus*)&h )->left && right==((Minus*)&h )->right;
  }
  size_t hash() const{
    return 5*::hash<GHom>()(left)+61*::hash<GDDD>()(right);
  }

  /* Eval */
  GDDD eval(const GDDD &d)const{
    return left(d)-right;
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }
};

/*************************************************************************/
/*                         Class StrongHom                               */
/*************************************************************************/

/* Compare */
bool StrongHom::operator==(const _GHom &h) const{
  return typeid(*this)==typeid(h)?*this==*(StrongHom*)&h:false;
}


/* Eval */
GDDD StrongHom::eval(const GDDD &d)const{
  if(d==GDDD::null)
    return GDDD::null;
  else if(d==GDDD::one)
    return phiOne();
  else if(d==GDDD::top)
    return GDDD::top;
  else{
    int variable=d.variable();
    set<GDDD> s;
    for(GDDD::const_iterator vi=d.begin();vi!=d.end();vi++){
      s.insert(phi(variable,vi->first)(vi->second));
    }
    return DED::add(s);
  }
}

/*************************************************************************/
/*                         Class GHom                                    */
/*************************************************************************/

/* Constructor */
GHom::GHom(const StrongHom *h):concret(h){}

GHom::GHom(StrongHom *h):concret(canonical(h)){}

GHom::GHom(const GDDD& d):concret(canonical(new Constant(d))){}

GHom::GHom(int var, int val, const GHom &h):concret(canonical(new LeftConcat(GDDD(var,val),h))){}

/* Eval */
GDDD GHom::operator()(const GDDD &d) const{
  if(concret->immediat)
    return concret->eval(d);
  else
    return DED::hom(*this,d);
}

GDDD GHom::eval(const GDDD &d) const{
  return concret->eval(d);
}

const GHom GHom::id(canonical(new Identity(1)));

int GHom::refCounter() const{return concret->refCounter;}

/* Sum */

GHom GHom::add(const set<GHom>& s){
  return(new Add(s));
}


/* Memory Manager */
unsigned int GHom::statistics(){
  return canonical.size();
}

// Todo
void GHom::mark()const{
  if(!concret->marking){
    concret->marking=true;
    concret->mark();
  }
};

void GHom::garbage(){
  // mark phase
  for(UniqueTable<_GHom>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();di++){
    if(di->second->refCounter!=0){
      di->second->marking=true;
      di->second->mark();
    }
  }
  // sweep phase
  for(UniqueTable<_GHom>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();){
    if(!di->second->marking){
      UniqueTable<_GHom>::Table::iterator ci=di;
      di++;
      _GHom *g=ci->second;
      canonical.table.erase(ci);
      delete g;
    }
    else{
      di->second->marking=false;
      di++;
    }
  }
}

/*************************************************************************/
/*                    Class Hom                                          */
/*************************************************************************/
/* Constructor */
Hom::Hom(const Hom &h):GHom(h.concret){
  concret->refCounter++;
}

Hom::Hom(const GHom &h):GHom(h.concret){
  concret->refCounter++;
}

Hom::Hom(const GDDD& d):GHom(d){
  concret->refCounter++;
}

Hom::Hom(int var, int val, const GHom &h):GHom(var,val,h){
  concret->refCounter++;
}

Hom::~Hom(){
  assert(concret->refCounter>0);
  concret->refCounter--;
}

/* Set */

Hom &Hom::operator=(const Hom &h){
  assert(concret->refCounter>0);
  concret->refCounter--;
  concret=h.concret;
  concret->refCounter++;
  return *this;
}

Hom &Hom::operator=(const GHom &h){
  assert(concret->refCounter>0);
  concret->refCounter--;
  concret=h.concret;
  concret->refCounter++;
  return *this;
}

/* Operations */
GHom operator&(const GHom &h1,const GHom &h2){
  return GHom(canonical(new Compose(h1,h2)));
}

GHom operator+(const GHom &h1,const GHom &h2){
  set<GHom> s;
  s.insert(h1);
  s.insert(h2);
//  return(new Add(s));
  return GHom(canonical(new Add(s)));
}

GHom operator*(const GDDD &d,const GHom &h){
  return GHom(canonical(new Mult(h,d)));
}

GHom operator*(const GHom &h,const GDDD &d){
  return GHom(canonical(new Mult(h,d)));
}

GHom operator^(const GDDD &d,const GHom &h){
  return GHom(canonical(new LeftConcat(d,h)));
}

GHom operator^(const GHom &h,const GDDD &d){
  return GHom(canonical(new RightConcat(h,d)));
}

GHom operator-(const GHom &h,const GDDD &d){
  return GHom(canonical(new Minus(h,d)));
}


