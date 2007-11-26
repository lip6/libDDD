#include <typeinfo>
#include <iostream>
#include "Hom.h"
#include "DDD.h"
#include "DED.h"
#include "UniqueTable.h"

#include <cassert>

#ifdef PARALLEL_DD
#include <tbb/concurrent_vector.h>
#include <tbb/parallel_reduce.h>
#endif

void PrintMapJumps(double ratemin=0){
}





/* Unique Table */
namespace __gnu_cxx {
  template<> 
  struct hash<_GHom*>{
    size_t operator()(_GHom * _h) const{
      return _h->hash();
    }
  };
}

namespace std {
  template<>
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

    bool
    skip_variable(int var) const 
    {
        return true;
    }

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
    return value.hash();
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
    return 83*left.hash()+53*right.hash();
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
class Add:public _GHom
{

private:

  std::set<GHom> parameters;
  
public:
  
  /* Constructor */
  Add( const std::set<GHom> &param, int ref=0)
  	:
  	_GHom(ref,false),
  	parameters()
  {
    for( std::set<GHom>::const_iterator it = param.begin(); it != param.end(); ++it)
    {
      if( typeid( *get_concret(*it) ) == typeid(Add) )
      {
        std::set<GHom>& local_param = ((Add*)get_concret(*it))->parameters;
        parameters.insert( local_param.begin() , local_param.end());
      }
      else
      {
        parameters.insert(*it);
      }
    }
  }
  
  

/* Compare */
  bool operator==(const _GHom &h) const
  {
    return parameters==((Add*)&h )->parameters;
  }
  
  size_t hash() const
  {
    size_t res=0;
    for(std::set<GHom>::const_iterator gi=parameters.begin();gi!=parameters.end();++gi)
      res^= gi->hash();
    return res;
  }

  /* Eval */
  GDDD
  eval(const GDDD &d) const
  {
      if( d == GDDD::null )
      {
          return GDDD::null;
      }
      else if( d == GDDD::one || d == GDDD::top )
      {
          std::set<GDDD> s;
          
          for(std::set<GHom>::const_iterator gi=parameters.begin();gi!=parameters.end();++gi)
          {
              s.insert((*gi)(d));
          }
          return DED::add(s);
      }
      else
      {
          std::set<GDDD> s;
          std::set<GHom> constant_hom;
          
          int var = d.variable();
          
          for(std::set<GHom>::const_iterator gi=parameters.begin();gi!=parameters.end();++gi)
          {
              if( get_concret(*gi)->skip_variable(var) )
              {
                  constant_hom.insert(*gi);
              }
              else
              {
                  s.insert((*gi)(d));
              }
          }
          
          GDDD::Valuation v ;
          
          if( not constant_hom.empty() )
          {
              
              GHom all = GHom::add(constant_hom);
              
              for( GDDD::const_iterator it = d.begin() ; it != d.end() ; ++it )
              {
                  GDDD son = all(it->second);
                  if( son != GDDD::null )
                  {
                      v.push_back(std::make_pair(it->first, son));
                  }
              }
              
              if( not v.empty() )
                  s.insert(GDDD(var,v));
          }
          
          return DED::add(s);
      }
  }
    
  /* Memory Manager */
  void mark() const{
    for(std::set<GHom>::const_iterator gi=parameters.begin();gi!=parameters.end();++gi)
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
    return 13*left.hash()+7*right.hash();
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
    return 23*left.hash()+47*right.hash();
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
    return 47*left.hash()+19*right.hash();
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
    return 5*left.hash()+61*right.hash();
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

/************************** Fixpoint */
class Fixpoint
    	:
        public _GHom
{

private:
    GHom arg;

public:

    /* Constructor */

    Fixpoint(const GHom &a,int ref=0)
    	:
        _GHom(ref),
        arg(a)
    {
    }

    /* Compare */
    bool operator==(const _GHom &h) const
    {
        return arg==((Fixpoint*)&h )->arg ;
    }

    size_t
    hash() const {
        return 17*arg.hash();
    }
    
    /* Eval */
    GDDD
    eval(const GDDD &d) const
    {
        GDDD d1 = d;
        GDDD d2 = d;
    
        do
        {
            d1 = d2;
            d2 = arg(d2);
        } 
        while (d1 != d2);
        
        return d1;
    }
    
    /* Memory Manager */
    void mark() const{
        arg.mark();
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
GDDD 
StrongHom::eval(const GDDD &d) const
{
  if( d == GDDD::null )
  {
      return GDDD::null;
  }
  else if( d == GDDD::one )
  {
      return phiOne();
  }
  else if( d == GDDD::top )
  {
      return GDDD::top;
  }
  else if( this->skip_variable(d.variable()) )
  {
      // The homorphism propagates itself without any modification on the GDDD
      GDDD::Valuation v;
      for( GDDD::const_iterator it = d.begin() ; it != d.end() ; ++it )
      {
          GDDD son = GHom(this)(it->second);
          if( son != GDDD::null )
          {
              v.push_back(std::make_pair(it->first, son));
          }
      }

      if( v.empty() )
          return GDDD::null;
      else
          return GDDD(d.variable(),v);
  }
  else
  {
    int variable=d.variable();
    std::set<GDDD> s;
    for(GDDD::const_iterator vi=d.begin();vi!=d.end();++vi)
    {
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
GDDD
GHom::operator()(const GDDD &d) const
{
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

GHom GHom::add(const std::set<GHom>& s){
  return(canonical(new Add(s)));
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
  for(UniqueTable<_GHom>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();++di){
    if((*di)->refCounter!=0){
      (*di)->marking=true;
      (*di)->mark();
    }
  }
  // sweep phase
  for(UniqueTable<_GHom>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();){
    if(!((*di)->marking)){
      UniqueTable<_GHom>::Table::iterator ci=di;
      di++;
      _GHom *g=(*ci);
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
GHom fixpoint (const GHom &h) {
  if (h != GHom::id)
    return GHom(canonical(new Fixpoint(h)));
  else
    return GHom::id;
}

GHom operator&(const GHom &h1,const GHom &h2){
  return GHom(canonical(new Compose(h1,h2)));
}

GHom operator+(const GHom &h1,const GHom &h2){
  std::set<GHom> s;
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

/*************************************************************************/
/*                         Class MyGHom                                    */  
/*************************************************************************/

/* Constructor */
GHom::GHom(const MyGHom *h):concret(h){}

GHom::GHom(MyGHom *h):concret(canonical(h)){}



void GHom::pstats(bool reinit)
{
  std::cout << "*\nGHom Stats : size unicity table = " <<  canonical.size() << std::endl;
}
