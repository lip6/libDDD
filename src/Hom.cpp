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

#include <typeinfo>
#include <iostream>
#include <cassert>
#include <map>
#include <set>


#include "Hom.h"
#include "DDD.h"
#include "DED.h"
#include "UniqueTable.h"
#include "MLHom.h"
#include "util/hash_support.hh"
#include "util/configuration.hh"
#include "Cache.hh"

namespace d3 { namespace util {
  template<>
  struct equal<_GHom*>{
    bool operator()(_GHom * _h1,_GHom * _h2){
      return (typeid(*_h1)==typeid(*_h2)?(*_h1)==(*_h2):false);
    }
  };
}}

static UniqueTable<_GHom> canonical;

typedef Cache<GHom,GDDD> HomCache;

static HomCache cache;


/*************************************************************************/
/*                         Class _GHom                                   */
/*************************************************************************/

/************************** Identity */
class Identity:public _GHom{
public:
  /* Constructor */
  Identity(int ref=0):_GHom(ref,true){}

  /* Compare */
  bool operator==(const _GHom&) const{return true;}
  size_t hash() const{return 17;}
  _GHom * clone () const {  return new Identity(*this); }
    bool
    skip_variable(int) const 
    {
        return true;
    }
  bool is_selector () const {
    return true; 
  }
  void print (std::ostream & os) const {
    os << "Id";
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
  _GHom * clone () const {  return new Constant(*this); }
  /* Eval */
  GDDD eval(const GDDD &d)const{
    return d==GDDD::null?GDDD::null:value;
  }

  /* Memory Manager */
  void mark() const{
    value.mark();
  }

  void print (std::ostream & os) const {
    os << "(Constant:" << value << ")";
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
  _GHom * clone () const {  return new Mult(*this); }

  /* Eval */
  GDDD eval(const GDDD &d)const{
    return left(d)*right;
  }

  bool is_selector () const {
    // intersection is a natural selector (if we forget about TOP)
    return left.is_selector() ;
  }


  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }

  void print (std::ostream & os) const {
    os << "(Mult:" << left << "*" << right << ")";
  }
  
};

// negator for a selector

class NotCond
	:
	public _GHom
{
  // selector hom
  GHom cond_;
public :
  NotCond (const GHom & cond): cond_(cond) {};

  // skip if every argument skips.
  bool skip_variable (int var) const {
    return get_concret(cond_)->skip_variable(var);
  }

  bool is_selector () const {
    return true;
  }
  
  GDDD eval(const GDDD &d) const {
   if (d == GDDD::one || d == GDDD::null || d == GDDD::top )
      return d;
   
   GDDD condtrue = cond_ (d);
   return (d - condtrue);
  }

  void mark() const {
    cond_.mark();
  }  
  
  size_t hash() const {
    return  cond_.hash() * 6353; 
  }

  bool operator==(const _GHom &s) const {
    const NotCond* ps = (const NotCond *)&s;
    return cond_ == ps->cond_ ;
  }  

  _GHom * clone () const {  return new NotCond(*this); }

  void print (std::ostream & os) const {
    os << "(NOT: ! " << cond_  << ")";
  }
};


/************************** Add */
class Add
	:
    public _GHom
{
public:

    typedef std::pair< GHom , std::set<GHom> > partition;
    typedef std::map<int,partition> partition_cache_type;


private:

    std::set<GHom> parameters;
    mutable partition_cache_type partition_cache;
    bool have_id;
       
public:
  
    /* Constructor */
    Add( const std::set<GHom> &param, int ref=0)
  		:
  		_GHom(ref,false),
  		parameters(),
		have_id(false)
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

    bool
    get_have_id() const
    {
      return parameters.find(GHom::id) != parameters.end();
    }
   
    std::set<GHom>&
    get_parameters()
    {
        return this->parameters;
    }
    
    partition
    get_partition(int var)
    {
        this->skip_variable(var);
        return partition_cache.find(var)->second;
    }

    /* Compare */
    bool
    operator==(const _GHom &h) const
    {
        return parameters==((Add*)&h )->parameters;
    }
  
  
    size_t
    hash() const
    {
        size_t res=0;
        for(std::set<GHom>::const_iterator gi=parameters.begin();gi!=parameters.end();++gi)
        {
            res ^= gi->hash();
        }
        return res;
    }

  bool is_selector () const {
    for (std::set<GHom>::const_iterator gi=parameters.begin();gi!=parameters.end();++gi)
      if (! gi->is_selector() )
	return false;
    return true;
  }

  _GHom * clone () const {  return new Add(*this); }
  
    bool
    skip_variable(int var) const
    {
        partition_cache_type::iterator part_it = partition_cache.find(var);
        if( part_it == partition_cache.end() )
        {
            partition& part = partition_cache.insert(std::make_pair(var,partition())).first->second;

            std::set<GHom> F;
            for(std::set<GHom>::const_iterator gi=parameters.begin();gi!=parameters.end();++gi)
            {
                if( get_concret(*gi)->skip_variable(var) )
                {
                    // F part
                    F.insert(*gi);
                }
                else
                {
                    // G part
                    part.second.insert(*gi);
                }
            }
            part.first = GHom::add(F);
            return part.second.empty();
        }
        return part_it->second.second.empty();
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
          int var = d.variable();
                    
          partition_cache_type::iterator part_it = partition_cache.find(var);
          if( part_it == partition_cache.end() )
          {
              this->skip_variable(var);
              part_it = partition_cache.find(var);
          }              
          
          GHom& F = part_it->second.first;
          std::set<GHom>& G = part_it->second.second;
          
          for( std::set<GHom>::iterator it = G.begin() ; 
              it != G.end();
              ++it)
              {
                  s.insert((*it)(d));                  
              } 
          
          
          GDDD v = F(d);
          if( v != GDDD::null )
          {
              s.insert(v);
          }

          return DED::add(s);
      }
  }
    
  /* Memory Manager */
  void mark() const{
    for(std::set<GHom>::const_iterator gi=parameters.begin();gi!=parameters.end();++gi)
      gi->mark();
    partition_cache.clear();
  }

 void print (std::ostream & os) const {
    os << "(Add:" ;
    std::set<GHom>::const_iterator gi=parameters.begin();
    os << *gi ;
    for( ++gi;
	 gi!=parameters.end();
	 ++gi)
      {
	os << " + " << *gi ;
      }
    os << ")";
  }
};
/************************** Compose */
class Compose
	:
    public _GHom
{

private:

    GHom left;
    GHom right;

public:

  /* Constructor */
    Compose(const GHom &l,const GHom &r,int ref=0)
		:
	    _GHom(ref,true),
    	left(l),
	    right(r)
    {
    }

    /* Compare */
    bool
    operator==(const _GHom &h) const
    {
        return left==((Compose*)&h )->left && right==((Compose*)&h )->right;
    }
    
    size_t
    hash() const
    {
        return 13*left.hash() + 7*right.hash();
    }

  _GHom * clone () const {  return new Compose(*this); }
    
    bool
    skip_variable(int var) const
    {
        return get_concret(left)->skip_variable(var) && get_concret(right)->skip_variable(var);
    }

  bool is_selector () const {
    return left.is_selector() && right.is_selector();
  }
    /* Eval */
    GDDD
    eval(const GDDD &d) const
    {
        return left(right(d));
    }
    
    /* Memory Manager */
    void mark() const
    {
        left.mark();
        right.mark();
    }

  void print (std::ostream & os) const {
    os << "(Compose:" << left << " & " << right << ")";
  }

};

/************************** LeftConcat */
class LeftConcat
	:
    public _GHom
{

private:

    GDDD left;
    GHom right;

public:

    /* Constructor */
    LeftConcat(const GDDD &l,const GHom &r,int ref=0)
    	:
        _GHom(ref,true),
        left(l),
        right(r)
    {
    }

    /* Compare */
    bool
    operator==(const _GHom &h) const
    {
        return left==((LeftConcat*)&h )->left && right==((LeftConcat*)&h )->right;
    }

    size_t
    hash() const
    {
        return 23*left.hash()+47*right.hash();
    }

  _GHom * clone () const {  return new LeftConcat(*this); }

    /* Eval */
    GDDD
    eval(const GDDD &d) const
    {
        return left ^ right(d);
    }
           
    /* Memory Manager */
    void
    mark() const
    {
        left.mark();
        right.mark();
    }

  void print (std::ostream & os) const {
    os << "(LeftConcat:" << left << " ^ " << right << ")";
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
  _GHom * clone () const {  return new RightConcat(*this); }


    bool
    skip_variable(int var) const
    {
        return get_concret(left)->skip_variable(var);
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

  void print (std::ostream & os) const {
    os << "(RightConcat:" << left << " ^ " << right << ")";
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
  _GHom * clone () const {  return new Minus(*this); }

  /* Eval */
  GDDD eval(const GDDD &d)const{
    return left(d)-right;
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }

 bool is_selector () const {
    // set difference is a natural selector
    return left.is_selector() ;
  }

  void print (std::ostream & os) const {
    os << "(Minus:" << left << " - " << right << ")";
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
        _GHom(ref,false),
        arg(a)
    {
    }

    /* Compare */
    bool operator==(const _GHom &h) const
    {
        return arg==((Fixpoint*)&h )->arg ;
    }

    size_t
    hash() const 
    {
        return 17 * arg.hash();
    }
    
  _GHom * clone () const {  return new Fixpoint(*this); }

    bool
    skip_variable(int var) const
    {
        return get_concret(arg)->skip_variable(var);
    }
   
  bool is_selector () const {
    // wow ! why build a fixpoint of a selector ??
    return arg.is_selector();
  }

 
    /* Eval */
    GDDD
    eval(const GDDD &d) const
    {
        if( d == GDDD::null )
        {
            return GDDD::null;
        }
        else if( d == GDDD::one or d == GDDD::top )
        {
            return arg(d);
        }
        else
        {
            int variable = d.variable();
            
            GDDD d1 = d;
            GDDD d2 = d;
            
            // is it the fixpoint of an union ?
            if( typeid( *get_concret(arg) ) == typeid(Add) )
            {
                // Check if we have ( Id + F + G )* where F can be forwarded to the next variable
                
                // Rewrite ( Id + F + G )*
                // into ( (G + Id) o (F + Id)* )* 
                Add* add = ((Add*)get_concret(arg));

                if( add->get_have_id() )
                {
		    Add::partition partition = add->get_partition(variable);
                    
		    // operations that can be forwarded to the next variable
                    GHom F_part = fixpoint(partition.first);

                    // operations that have to be applied at this level
                    // std::set<GHom> G = partition.second;

                    do
                    {
                        d1 = d2;

			// Apply ( Id + F )* on all sons
			d2 = F_part(d2);
                          
                        // Apply ( G + Id )
                        for (std::set<GHom>::const_iterator it = partition.second.begin() ; it != partition.second.end() ; ++it ) {
			  d2 = (*it) (d2) + d2;
			}
                    }
                    while (d1 != d2);
                    return d1;
                }
            }
            
            do
            {
                d1 = d2;
                d2 = arg(d2);
            } 
            while (d1 != d2);

            return d1;
        }
    }
    
    /* Memory Manager */
    void mark() const{
        arg.mark();
    }

  void print (std::ostream & os) const {
    os << "(Fix:" << arg << " *)";
  }

  
};

GHom _GHom::compose (const GHom &r) const { 
  return GHom(this) & r; 
}

GDDD 
_GHom::eval_skip(const GDDD& d) const
{
    if( d == GDDD::null )
    {
        return GDDD::null;
    }
    else if( d == GDDD::one )
    {
        // basic case, mustn't call d.variable()
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

    return eval(d);
}




/*************************************************************************/
/*                         MLHom  adapter class */
/*************************************************************************/

class MLHomAdapter :public _GHom{
private:
  MLHom h;
public:
  /* Constructor */
  MLHomAdapter(const MLHom &hh):h(hh){}
  /* Compare */
  bool operator==(const _GHom &other) const{
    return h==((MLHomAdapter*)& other )->h;
  }
  size_t hash() const{
    return 19751*h.hash();
  }
  _GHom * clone () const {  return new MLHomAdapter(*this); }

  /* Eval */
  GDDD eval(const GDDD &d)const{
    HomNodeMap m = h (d);
    std::set<GDDD> sum;
    for (HomNodeMap::const_iterator it = m.begin() ; it != m.end() ; ++it) {
      sum.insert(it->first (it->second));
    }
    return DED::add(sum);
  }

  /* Memory Manager */
  void mark() const{
    /// ???????
    // h.mark();
  }

  void print (std::ostream & os) const {
    os << "MLHom";
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
  if( d== GDDD::null)
  {
    return GDDD::null;
  }
  else if( d == GDDD::one)
  {
    return phiOne();
  }
  else if( d== GDDD::top)
  {
    return GDDD::top;
  }
  else
  {

    int variable = d.variable();
    std::set<GDDD> s;
    for( GDDD::const_iterator vi = d.begin();
         vi!=d.end();
         ++vi)
    {
        s.insert(phi(variable,vi->first)(vi->second));
    }
    return DED::add(s);
  }
}

void StrongHom::print (std::ostream & os) const {
  os << "(StrongHom)";
}


/*************************************************************************/
/*                         Class GHom                                    */
/*************************************************************************/

/* Constructor */
GHom::GHom(const _GHom &_h):concret(canonical(_h)) {};

GHom::GHom(const MLHom &h):concret(canonical(MLHomAdapter(h))) {};


GHom::GHom(const GDDD& d):concret(canonical(Constant(d))){}

GHom::GHom(int var, int val, const GHom &h):concret(canonical(LeftConcat(GDDD(var,val),h))){}

bool GHom::skip_variable(int var) const {
  return concret->skip_variable(var);
}

GHom GHom::compose (const GHom &r) const { 
  return concret->compose(r); 
}

/* Eval */
GDDD
GHom::operator()(const GDDD &d) const
{
    if(concret->immediat)
    {
        return concret->eval(d);
    }
    else
    {
        if (d == GDDD::null) 
        {
            return d;
        }
        else
        {
            return (cache.insert(*this,d)).second;
        }
    }
}

GDDD GHom::eval(const GDDD &d) const{
  return concret->eval_skip(d);
}

const GHom GHom::id(canonical( Identity(1)));

int GHom::refCounter() const{return concret->refCounter;}

/* Sum */

GHom GHom::add(const std::set<GHom>& set){
    if( set.empty() )
        return GDDD::null;
    if( set.size() == 1 )
      return *(set.begin());
    else {
      std::set<GHom> s = set;
      s.erase(GHom(GDDD::null));
      if( s.size() == 1 )
	return *(s.begin());
      return(canonical( Add(s)));
    }
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
  cache.clear();

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
      const _GHom *g=(*ci);
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

/// This predicate is true if the homomorphism global behavior is only to prune some paths.
bool GHom::is_selector() const {
  return concret->is_selector();
}


/* Operations */
GHom fixpoint (const GHom &h) {
  if (h != GHom::id)
    return GHom(canonical( Fixpoint(h)));
  else
    return GHom::id;
}
static void printCondError (const GHom & cond) {

  std::cerr << " but the homomorphism passed :" << std::endl;
  std::cerr << cond << std::endl ;
  std::cerr << "Does not have selector flag set to true. If your logic is correct, check that"
	    << " you implement : bool is_selector() const { return true; }\n"
	    << " in all selector inductive homomorphisms." << std::endl ;
}

GHom ITE (const GHom & cond, const GHom & iftrue, const GHom & iffalse) {
  if (! cond.is_selector() ) {
    std::cerr << "Creating a complement condition in an ITE construct : " << std::endl;
    printCondError(cond);
    assert(false);
  }  
  // let optimizations and rewritings do their job.
  return (iftrue & cond) + (iffalse & (!cond));
}

GHom operator! (const GHom & cond) {
  if (! cond.is_selector() ) {
    std::cerr << "Creating a complement condition with operator! :  ! cond" << std::endl;
    printCondError(cond);
    assert(false);
  }  
  return NotCond(cond);
}

GHom operator&(const GHom &h1,const GHom &h2){
  GHom nullHom = GDDD::null;
  if (h1 == nullHom || h2 == nullHom)
    return nullHom;

	if( h1 == GHom::id )
		return h2;

	if( h2 == GHom::id )
		return h1;

	return GHom(canonical( Compose(h1,h2)));
}


GHom operator+(const GHom &h1,const GHom &h2){
  std::set<GHom> s;
  s.insert(h1);
  s.insert(h2);
//  return(new Add(s));
  return GHom::add(s);
}

GHom operator*(const GDDD &d,const GHom &h){
  return GHom(canonical( Mult(h,d)));
}

GHom operator*(const GHom &h,const GDDD &d){
  return GHom(canonical( Mult(h,d)));
}

GHom operator^(const GDDD &d,const GHom &h){
  return GHom(canonical( LeftConcat(d,h)));
}

GHom operator^(const GHom &h,const GDDD &d){
  return GHom(canonical( RightConcat(h,d)));
}

GHom operator-(const GHom &h,const GDDD &d){
  return GHom(canonical( Minus(h,d)));
}

/*************************************************************************/
/*                         Class MyGHom                                    */  
/*************************************************************************/


void GHom::pstats(bool)
{
  std::cout << "*\nGHom Stats : size unicity table = " <<  canonical.size() << std::endl;

  std::ostream & os = std::cout;
  int i = 0;
  for (UniqueTable<_GHom>::Table::const_iterator it= canonical.table.begin() ;
       it != canonical.table.end();
       ++it ){
    os << i++ << " : " ;
    (*it)->print(os);
    os << std::endl;
  }

}


// pretty print
std::ostream & operator << (std::ostream & os, const GHom & h) {
  h.concret->print(os);
  return os;
}
