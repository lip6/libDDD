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
#include <algorithm>
#include "util/set.hh"


#include "Hom.h"
#include "DDD.h"
#include "DED.h"
#include "UniqueTable.h"
#include "MLHom.h"
#include "util/hash_support.hh"
#include "util/configuration.hh"
#include "Cache.hh"
#include "MemoryManager.h"
#include "FixObserver.hh"

namespace d3 { namespace util {
  template<>
  struct equal<_GHom*>{
    bool operator()(_GHom * _h1,_GHom * _h2){
      return (typeid(*_h1)==typeid(*_h2)?(*_h1)==(*_h2):false);
    }
  };
}}

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
  GDDD has_image (const GDDD & d) const {
    return d;
  }
  /// returns a negation of a selector homomorphism h, such that h.negate() (d) = d - h(d)
  GHom negate () const {
    return GHom(GDDD::null);
  }  

 GHom invert  (const GDDD & pot) const {
   return this;
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

 GHom invert  (const GDDD & pot) const {
   return pot;
 }
  
  bool is_selector () const {
    // in any other case, it might return a value outside the entry set, but empty set is always a legal subset of any DDD
    return (value == GDDD::null) ;
  }

  /* Memory Manager */
  void mark() const{
    value.mark();
  }

  void print (std::ostream & os) const {
    os << "(Constant:" << value << ")";
  }

  
};

// Apply a 2 level DDD to current value
class Apply2k:public _GHom{
private:
  // should be strictly 2 variables high, variable identities are irrelevant.
  GDDD value;
public:
  /* Constructor */
  Apply2k(const GDDD &d,int ref=0):_GHom(ref,false),value(d){}

  /* Compare */
  bool operator==(const _GHom &h) const{
    return value==((Apply2k*)&h )->value;
  }
  size_t hash() const{
    return value.hash() * 13;
  }
  _GHom * clone () const {  return new Apply2k(*this); }
  /* Eval */
  GDDD eval(const GDDD &d)const{

//    std::cerr << "Apply transition " << value << "\n to value :" << d << std::endl;

    if ( d==GDDD::null || value==GDDD::null)
      return GDDD::null; 
    
    // joint traversal
    GDDD::const_iterator it1 = d.begin();
    GDDD::const_iterator it2 = value.begin();
    GDDD::const_iterator it1end = d.end();
    GDDD::const_iterator it2end = value.end();
    d3::set<GDDD>::type toadd;

    for ( ; it1 != it1end && it2 != it2end ; ) {
      if (it1->first == it2->first) {
	// We have a match
	const GDDD & son = it2->second;
	GDDD::const_iterator sonend = son.end();
	for (GDDD::const_iterator it3 = son.begin() ; it3 != sonend ; ++it3) {
	  toadd.insert( GDDD(d.variable(),it3->first,it1->second) );
	}
	++it1;
	++it2;
      } else if (it1->first > it2->first) {
	// the value in transition => no such current value in d
	// shift tr
	++it2;
      } else {
	// so no such arc in tr => no successor for this arc
	++it1;
      }
    }

//    std::cerr << " obtained "<< DED::add(toadd) << std::endl;

    return DED::add(toadd);

  }

 GHom invert  (const GDDD & pot) const {
   return pot;
 }


  /* Memory Manager */
  void mark() const{
    value.mark();
  }

  void print (std::ostream & os) const {
    os << "(Apply:" << value << ")";
  }

  
};


  /** Extractor of variable domains for invert computations */
  class DomExtract
    :
    public _GHom
  {

  public:

    int target;

    DomExtract()
      :
      target(0)
    {}

    DomExtract (int t) :target(t) {}


    // this hom is a heavy modifier
    bool skip_variable (int var) const {
      return false;
    }

    bool is_selector () const {
      return false;
    }

  
    GDDD eval(const GDDD &d)const {
      if (d == GDDD::one || d == GDDD::null || d == GDDD::top )
	return d;

      d3::set<GDDD>::type sum;

      if (d.variable() != target) {
	// destroy/propagate
	GDDD::const_iterator dend = d.end();
	for ( GDDD::const_iterator it = d.begin(); it != dend; ++it)
	  sum.insert( GHom(this) (it->second) );
      } else {
	// grab all arc values and fuse them
	GDDD::const_iterator dend = d.end();
	for ( GDDD::const_iterator it = d.begin(); it != dend; ++it)
	  sum.insert( GDDD (target,it->first) );
      }

      return DED::add(sum);
    }
  
    size_t hash() const {
      return  (target-273) * 2196727; 
    }

    bool operator==(const _GHom &s) const {
      const DomExtract* ps = (const DomExtract *)&s;
      return target == ps->target ;
    }  

    _GHom * clone () const {  return new DomExtract(*this); }

    void print (std::ostream & os) const {
      os << "(DomExtract:" << target << ")";
    }

  };

GDDD computeDomain (int var, const GDDD& d) {
  return GHom(DomExtract(var)) (d);
}

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
    //  (h * c) (s) = h(s) * c
    return left(d)*right;
  }

  GHom invert  (const GDDD & pot) const {
    // (h * c)^-1 (s) = h^-1 ( s * c) = h^-1 & ( id * c ) (s)
    return left.invert(pot) &  ( GHom::id * right) ;
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

class Inter:public _GHom{
private:
  GHom left;
  GHom right;
public:
  /* Constructor */
  Inter(const GHom &l,const GHom &r,int ref=0):_GHom(ref),left(l),right(r){}
  /* Compare */
  bool operator==(const _GHom &h) const{
    return left==((Inter*)&h )->left && right==((Inter*)&h )->right;
  }
  size_t hash() const{
    return 83*left.hash()+ 153*right.hash();
  }
  
  _GHom * clone () const {  return new Inter(*this); }
  /* Eval */
  GDDD eval(const GDDD &d)const{
    return left(d) * right(d);
  }
  
  GHom invert (const GDDD & pot) const { 
    // (h * h')^-1 (s) =( h^-1 ( pot ) +  h^-1 (pot) ) (s) 
    return (left.invert(pot) + right.invert(pot)) ;
  }
  
  bool is_selector () const {
    // intersection is always a selector
    // indeed, either left or right is a selector
    // (h * sel)(d) = h(d) * sel(d) \subset sel(d) \subset d
    return true;
  }
  
  bool
  skip_variable(int var) const 
  {
    return get_concret(left)->skip_variable(var)
    && get_concret(right)->skip_variable(var);
  }
  /// returns a negation of a selector homomorphism h, such that h.negate() (d) = d - h(d)
  GHom negate () const {
    return left.negate() + right.negate();
  }  


  GDDD has_image (const GDDD & d) const {    
    GDDD imgl = left.has_image(d);
    if (imgl == GDDD::null) {
      return imgl;
    }
    GDDD imgr = right.has_image(d);
    if (imgr == GDDD::null) {
      return imgr;
    }
    // lucky ?
    GDDD inter = imgl * imgr ;
    if (! ( inter == GDDD::null ) ) {
      return inter;
    } 

    // default
    return _GHom::has_image(d);
  }
  
  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }
  
  void print (std::ostream & os) const {
    os << "(Inter:" << left << "*" << right << ")";
  }
  
};

// negator for a selector

class NotCond
	:
	public _GHom
{
  // selector hom
  GHom cond_;
    
    friend GHom operator!(const GHom &);
public :
  NotCond (const GHom & cond): cond_(cond) {};

  // skip if every argument skips.
  bool skip_variable (int var) const {
    return get_concret(cond_)->skip_variable(var);
  }

	const GHom::range_t  get_range () const {
		return cond_.get_range() ;
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

  /// returns a negation of a selector homomorphism h, such that h.negate() (d) = d - h(d)
  GHom negate () const {
    return cond_;
  }  


  GDDD has_image (const GDDD & d) const {
    if (d==GDDD::null)
      return d;
    return cond_.negate().has_image(d);
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

  typedef std::vector<GHom> param_t;
  typedef param_t::const_iterator param_it;

  typedef std::pair< GHom , std::set<GHom> > partition;
  typedef std::map<int,partition> partition_cache_type;


private:

  param_t parameters;
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
    std::set<GHom> tmp;
    for( std::set<GHom>::const_iterator it = param.begin(); it != param.end(); ++it) {
      // fuse internal Add
      if( typeid( *get_concret(*it) ) == typeid(Add) )	{
	std::vector<GHom>& local_param = ((Add*)get_concret(*it))->parameters;
	tmp.insert( local_param.begin() , local_param.end());

      } else {
	tmp.insert(*it);
      }
    }
    parameters = param_t(tmp.begin(), tmp.end());
  }

  bool
  get_have_id() const
  {
    return find(parameters.begin(), parameters.end(), GHom::id) != parameters.end();
  }

  /// returns a negation of a selector homomorphism h, such that h.negate() (d) = d - h(d)
  GHom negate () const {
    GHom toadd = GHom::id ;
    for( param_it it = parameters.begin(); it != parameters.end(); ++it) {
      toadd = toadd & it->negate();
    }
    return toadd;
  }  


  GDDD has_image (const GDDD & d) const {
    if( d == GDDD::null )
      {
	return GDDD::null;
      }
    else if( d == GDDD::one || d == GDDD::top )
      {
	std::set<GDDD> s;
        
	for(param_it gi=parameters.begin();gi!=parameters.end();++gi) {
	  GDDD img = gi->has_image(d);
	  if (! (img == GDDD::null)) {
	    return img;
	  }
	}
	return GDDD::null; 
      } 
    else 
      {
	int var = d.variable();
	
	partition_cache_type::iterator part_it = partition_cache.find(var);
	if( part_it == partition_cache.end() )
          {
	    this->skip_variable(var);
	    part_it = partition_cache.find(var);
          }              
	
	GHom& F = part_it->second.first;
        std::set<GHom> & G = part_it->second.second;
        
	for( std::set<GHom>::iterator it = G.begin() ; it != G.end(); ++it)
	  {
	    GDDD img = it->has_image(d);
	    if (! (img == GDDD::null)) {
	      return img;
	    }
	  }
	    
	GDDD v = F.has_image(d);
	
	return v;
      }
    
  }

   
  param_t &
  get_parameters()
  {
    return this->parameters;
  }
   
	const GHom::range_t  get_range () const {
		GHom::range_t ret;
		for(param_it gi = parameters.begin(); gi != parameters.end(); ++gi ) {	     
			GHom::range_t pret = gi->get_range();
			if ( pret.empty() )
				return pret;
			ret.insert(pret.begin() , pret.end()) ;
		}
		return ret;
    }
	
  partition
  get_partition(int var)
  {
        this->skip_variable(var);
        return partition_cache.find(var)->second;
  }

  GHom invert  (const GDDD & pot) const {
    // (\ADD_i h_i)^-1  = \ADD_i h_i^-1
    std::set<GHom> ops;
    for(param_it gi = parameters.begin(); gi != parameters.end(); ++gi ) {
      ops.insert(  gi->invert(pot) );
    }
    return GHom::add(ops);
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
        for(param_it gi=parameters.begin();gi!=parameters.end();++gi)
        {
            res ^= gi->hash();
        }
        return res;
    }

  bool is_selector () const {
    for (param_it gi=parameters.begin();gi!=parameters.end();++gi)
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
	for(param_it gi=parameters.begin();gi!=parameters.end();++gi)
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
          
          for(param_it gi=parameters.begin();gi!=parameters.end();++gi)
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
        std::set<GHom> & G = part_it->second.second;
          
          for( std::set<GHom>::iterator it = G.begin() ; it != G.end(); ++it)
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
    for(param_it gi=parameters.begin();gi!=parameters.end();++gi)
      gi->mark();
    partition_cache.clear();
  }

 void print (std::ostream & os) const {
    os << "(Add:" ;
    param_it gi=parameters.begin();
    os << *gi ;
    for( ++gi; gi!=parameters.end(); ++gi)
      {
	os << " + " << *gi ;
      }
    os << ")";
  }
};
/************************** Monotonic< fixpoint */
class Monotonic
	:
    public _GHom
{
public:

  typedef std::set<GHom> param_t;
  typedef param_t::const_iterator param_it;

  typedef std::pair< GHom , param_t > partition;
  typedef std::map<int,partition> partition_cache_type;


private:

  param_t parameters;
  mutable partition_cache_type partition_cache;
       
public:
  
    /* Constructor */
  Monotonic( const std::set<GHom> &param, int ref=0)
    :
    _GHom(ref,false),
    parameters(param)
  {
  }
   
  param_t &
  get_parameters()
  {
    return this->parameters;
  }
   
  const GHom::range_t  get_range () const {
    GHom::range_t ret;
    for(param_it gi = parameters.begin(); gi != parameters.end(); ++gi ) {	     
      GHom::range_t pret = gi->get_range();
      if ( pret.empty() )
	return pret;
      ret.insert(pret.begin() , pret.end()) ;
    }
    return ret;
  }
	
  partition
  get_partition(int var)
  {
        this->skip_variable(var);
        return partition_cache.find(var)->second;
  }

  GHom invert  (const GDDD & pot) const {
    // There is no real point to inverting a monotonous<
    // particularly if this is in a reverse transition relation, we would rather keep
    // the canonicity when firing backwards.
    return this;
  }

  /* Compare */
  bool
  operator==(const _GHom &h) const
  {
    return parameters==((Monotonic*)&h )->parameters;
  }
  
  
  size_t
  hash() const
  {
    size_t res=5468731;
    for(param_it gi=parameters.begin();gi!=parameters.end();++gi)
      {
	res ^= gi->hash();
      }
    return res;
  }

  bool is_selector () const {
    return false;
  }

  _GHom * clone () const {  return new Monotonic(*this); }
  
  bool
  skip_variable(int var) const
  {
    partition_cache_type::iterator part_it = partition_cache.find(var);
    if( part_it == partition_cache.end() )
      {
	partition& part = partition_cache.insert(std::make_pair(var,partition())).first->second;
	
	std::set<GHom> F;
	for(param_it gi=parameters.begin();gi!=parameters.end();++gi)
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
	part.first = monotonic(F);
	return part.second.empty();
      }
    return part_it->second.second.empty();
  }
  
  /* Eval */
  GDDD
  eval(const GDDD &d) const
  {
    if( d == GDDD::null || d == GDDD::one || d == GDDD::top )
      {
	return d;
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
	param_t & G = part_it->second.second;
	
	
	GDDD d1 = d;
	GDDD d2 = d;
	
	
	do
	  {
	    d1 = d2;
	    
            
	    // Apply ( G ) as a composition
	    for (param_it it = G.begin() ; it != G.end() ; ++it ) {
	      d2 = (*it) (d2);
	    }

	    // Apply ( F )* on all sons
	    d2 = F (d2);
	  }
	while (d1 != d2);
	return d1;
	
      }
  }
  
  /* Memory Manager */
  void mark() const{
    for(param_it gi=parameters.begin();gi!=parameters.end();++gi)
      gi->mark();
    partition_cache.clear();
  }

 void print (std::ostream & os) const {
    os << "(Monotonic:" ;
    param_it gi=parameters.begin();
    os << *gi ;
    for( ++gi; gi!=parameters.end(); ++gi)
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
    {}

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

  GHom invert  (const GDDD & pot) const {
    // (h & h')^-1  =  h'^-1 & h^-1
    //return  right.invert(pot) & left.invert(pot);
    GDDD newpot = right(pot);
    if (newpot != DDD::null) {
      return  right.invert(pot) & left.invert( newpot );
    } else {
      return GHom(newpot);
    }
  }


    
    bool
    skip_variable(int var) const
    {
        return get_concret(left)->skip_variable(var) && get_concret(right)->skip_variable(var);
    }
	
	const GHom::range_t  get_range () const {
		GHom::range_t ret = left.get_range();
		if ( ret.empty() )
			return ret;
		GHom::range_t pret = right.get_range();
		if ( pret.empty() )
			return pret;
		
		ret.insert(pret.begin() , pret.end()) ;
		return ret;
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

/************************** And */
/** A commutative composition of n homomorphisms */
class And
:
public _GHom
{
public :
    typedef std::vector<GHom> parameters_t;
    typedef parameters_t::const_iterator parameters_it;
    /// PLEASE DONT HURT ME !!
    parameters_t parameters;
	
public:
	
	/* Constructor */
    And(const parameters_t & p, int ref=0)
	:
	_GHom(ref,false),
	parameters(p) {
		//      assert (! p.empty());
		//      assert(p.size() > 1);
    }
	
	/* Compare */
    bool
    operator==(const _GHom &h) const
    {
      return parameters == ((And*)&h )->parameters;	
    }

  GHom negate () const {
    d3::set<GHom>::type toadd ;
    for(parameters_it gi=parameters.begin();gi!=parameters.end();++gi) {
      toadd.insert( gi->negate());
    }
    return GHom::add(toadd);
  }
  
  GDDD has_image (const GDDD & d) const {
    GDDD optimist = d;
    for(parameters_it gi=parameters.begin();gi!=parameters.end();++gi) {
      optimist = gi->has_image(optimist);
      if ( ! ( optimist == GDDD::null ) ) {
	continue ;
      } else if ( gi->has_image(d) == GDDD::null ) {
	return GDDD::null;
      }
    }
    if ( ! ( optimist == GDDD::null ) ) {
      return optimist;
    }    
    return _GHom::has_image(d);
  }
  
  size_t
  hash() const
  {
		size_t res = 40693 ;
		for(parameters_it gi=parameters.begin();gi!=parameters.end();++gi)
			res^=gi->hash();
		return res;
    }
	
    _GHom * clone () const {  return new And(*this); }
	
    bool is_selector () const {
		for (parameters_it gi=parameters.begin();gi!=parameters.end();++gi)
			if (! gi->is_selector() )
				return false;
		return true;
    }
    
    bool
    skip_variable( int var ) const
    {
		for(parameters_it gi = parameters.begin(); gi != parameters.end(); ++gi ) {	     
			if( ! gi->skip_variable(var) )
			{
				return false;
			}
		}
		return true;
    }
	
    const GHom::range_t  get_range () const {
		GHom::range_t ret;
		for(parameters_it gi = parameters.begin(); gi != parameters.end(); ++gi ) {	     
			GHom::range_t pret = gi->get_range();
			if ( pret.empty() )
				return pret;
			ret.insert(pret.begin() , pret.end()) ;
		}
		return ret;
    }
	
    /* Eval */
    GDDD
    eval(const GDDD& d)const {
		if( d == GDDD::null ) {
			return GDDD::null;
			
		} else if (  d == GDDD::one || d == GDDD::top ) {
			GDDD res = d;
			// simply apply composition semantics
			for(parameters_it gi = parameters.begin(); gi != parameters.end(); ++gi ) {	     
				res = (*gi) (res);
			}
			return res;
		} else {
			GDDD res = d;
			parameters_t F;
			int var = d.variable() ;
			for(parameters_it gi = parameters.begin(); gi != parameters.end(); ++gi ) {	     
				if ( gi->skip_variable(var) ) {
					F.push_back(*gi);
				} else {
					res = (*gi) (res);
				}
			}
			GHom nextSel = GHom::id ;
			if (! F.empty()) {
				if ( F.size() > 1 ) 
					nextSel = And (F);
				else 
					nextSel = *F.begin();
			}
			
			return  nextSel (res)  ;	
		}
    }
	
	/* Memory Manager */
    void
    mark() const
    {
		for(parameters_it gi = parameters.begin(); gi != parameters.end(); ++gi ) {	     
			gi->mark();
		}
    }
	
	GHom invert (const GDDD & pot) const { 
		// (\AND_i h_i)^-1  = \AND_i h_i^-1
		GHom ret = GHom::id;
		for(parameters_it gi = parameters.begin(); gi != parameters.end(); ++gi ) {
			ret = ret & ( gi->invert(pot) );
		}
		return ret;
    }
	
	void print (std::ostream & os) const {
		os << "(And:" ;
		parameters_it gi=parameters.begin();
		os << *gi ;
		
		for( ++gi;
			gi!=parameters.end();
			++gi)
		{
			os << " && " << *gi ;
		}
		os << ")";
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

     
  GDDD
  has_image(const GDDD &d) const {
    return left ^ right.has_image(d);
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


  GDDD
  has_image(const GDDD &d) const {
    return left.has_image(d) ^ right;
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

  GHom invert  (const GDDD & pot) const {
    // (h - cte)^-1 (s) = h^-1 (s + cte) =  h^-1 & ( id + cte) 
    return left.invert(pot) & (GHom::id + right);
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
    bool can_garbage;
public:

    /* Constructor */

    Fixpoint(const GHom &a,int ref=0,bool is_top_level=false)
    	:
        _GHom(ref,false),
        arg(a),
        can_garbage(is_top_level)
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
	
	const GHom::range_t  get_range () const {
		return arg.get_range();
    }
   
  GHom invert  (const GDDD & pot) const {
    // (h^*)^-1 (s) = (h^-1 + id)^*  
    return fixpoint ( arg.invert(pot) + GHom::id );
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
                          
                      if (! can_garbage && fobs::get_fixobserver ()->was_interrupted ())
                      {
                        return d2;
                      }
                      if (can_garbage && fobs::get_fixobserver ()->was_interrupted ())
                      {
                        fobs::get_fixobserver ()->update (d2, d1);
                        if (fobs::get_fixobserver ()->should_interrupt (d2, d1))
                        {
                          return d2;
                        }
                      }
                        // Apply ( G + Id )
                      std::set<GDDD> tmp;
                      for (std::set<GHom>::const_iterator it = partition.second.begin() ; it != partition.second.end() ; ++it ) {
                        tmp.insert ((*it) (d2));
                      }
                      tmp.insert (d2);
                      d2 = DED::add (tmp);
                      
                      if (! can_garbage && fobs::get_fixobserver ()->should_interrupt (d2, d1))
                      {
                        return d2;
                      }
                      if (can_garbage) {
                        if (fobs::get_fixobserver ()->should_interrupt (d2, d1))
                        {
                          fobs::get_fixobserver ()->update (d2, d1);
                          if (fobs::get_fixobserver ()->should_interrupt (d2, d1))
                          {
                            return d2;
                          }
                        }
                        
                        //		trace << "could trigger 2!!" << std::endl ;
                        if (MemoryManager::should_garbage()) {
                          //		  trace << "triggered !!" << std::endl ;
                          // ensure d1 and d2 and argument are preserved
                          d1.mark();
                          d2.mark();
                          //arg.mark();
                          F_part.mark();
                          for (std::set<GHom>::const_iterator it = partition.second.begin() ; it != partition.second.end() ; ++it ) {
                            it->mark();
                          }
                          Hom tt = Hom(this);
                          //std::cerr << "garbage saturation" << std::endl;
                          MemoryManager::garbage();
                        }
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
              if (! can_garbage && fobs::get_fixobserver ()->should_interrupt (d2, d1))
              {
                return d2;
              }
              if (can_garbage) {
                if (fobs::get_fixobserver ()->should_interrupt (d2, d1))
                {
                  fobs::get_fixobserver ()->update (d2, d1);
                  if (fobs::get_fixobserver ()->should_interrupt (d2, d1))
                  {
                    return d2;
                  }
                }
                //		trace << "could trigger 2!!" << std::endl ;
                if (MemoryManager::should_garbage()) {
                  //		  trace << "triggered !!" << std::endl ;
                  // ensure d1 and d2 and argument are preserved
                  d1.mark();
                  d2.mark();
                  arg.mark();
                  Hom tt = Hom(this);
                  //std::cerr << "garbage" << std::endl;
                  MemoryManager::garbage();
                }
              }
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
  //return GHom(this) & r; 
  
  // Note: code duplicated unfortunately
  GHom nullHom = GDDD::null;
  GHom thisH (this);
  if (thisH == nullHom || r == nullHom)
    return nullHom;

  if( thisH == GHom::id )
    return r;

  if( r == GHom::id )
    return thisH;

  return Compose(thisH, r);
}

GHom
_GHom::negate() const
{
  return ! GHom(this) ; 
}


GDDD
_GHom::has_image(const GDDD& d) const
{
  return GHom(this)(d) ; 
}
GDDD
_GHom::has_image_skip(const GDDD& d) const
{
  if( d == GDDD::null ||  d == GDDD::top) {
    return d;
  } else if( d == GDDD::one ) {
    // basic case, mustn't call d.variable()
  } else if( this->skip_variable(d.variable()) ) {

    // The homorphism propagates itself without any modification on the GDDD
    const GHom ghom(this);

    if (ghom == GHom::id) {
      return d;
    }
    
    for( GDDD::const_iterator it = d.begin() ; it != d.end() ; ++it )
      {
	GDDD son = ghom.has_image(it->second);
	if( son != GDDD::null )
	  {
	    return GDDD(d.variable(), it->first, son) ;
	  }
      }
    return GDDD::null;
  }

  return has_image(d);
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
      const GHom ghom(this);
      
      if (ghom == GHom::id) {
	return d;
      }
        GDDD::Valuation v;
        for( GDDD::const_iterator it = d.begin() ; it != d.end() ; ++it )
        {
            GDDD son = ghom (it->second);
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
StrongHom::has_image(const GDDD &d) const
{
  if( d == GDDD::null || d == GDDD::top)
  {
    return d;
  }
  else if( d == GDDD::one)
  {
    return  phiOne();
  }
  else
  {
    int variable = d.variable();
    
    GDDD::const_iterator dend = d.end();
    for( GDDD::const_iterator vi = d.begin();
         vi!=dend;
         ++vi)
    {
      GDDD res = phi(variable,vi->first)(vi->second);
      if (! (res == GDDD::null ) ) {
	return res;
      }
    }
    return GDDD::null;
  }
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
    GDDD::const_iterator dend = d.end();
    for( GDDD::const_iterator vi = d.begin();
         vi!=dend;
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

bool GHom::operator< (const GHom & h) const {
  return *concret < *h.concret;
}

bool GHom::skip_variable(int var) const {
  return concret->skip_variable(var);
}

const GHom::range_t  GHom::get_range() const {
	return concret->get_range();
}

// a test used in commutativity assesment
static bool notInRange (const GHom::range_t & h1r, const GHom & h2) {
	GHom::range_t h2r = h2.get_range();
	// ALL variables range
	if ( h2r.empty() )
		return false;
	
    // Test empty intersection relying on sorted property of sets.
	GHom::range_it h1it = h1r.begin();
	GHom::range_it h2it = h2r.begin();
	while ( h1it != h1r.end() && h2it != h2r.end() ) {
		if ( *h1it == *h2it ) 
			return false;
		else if ( *h1it < *h2it ) 
			++h1it;
		else
			++h2it;
    }
	
	return true;
}

static bool commutative (const GHom & h1, const GHom & h2) {
	if ( h1.is_selector() && h2.is_selector() ) 
		return true;
	
	GHom::range_t h1r = h1.get_range();
	// ALL variables range
	if ( h1r.empty() )
		return false;
	
	return notInRange (h1r , h2);
}

GHom GHom::compose (const GHom &r) const { 
  return concret->compose(r); 
}
 /// returns the predescessor homomorphism, using pot to determine variable domains
 GHom GHom::invert  (const GDDD & pot) const {
   return concret->invert(pot);
 }

GHom GHom::negate () const {
  return concret->negate();
}

typedef Cache<GHom,GDDD,GDDD> HomCache;

template<>
bool
HomCache::should_insert (const GHom & h) const
{
  if (fobs::get_fixobserver ()->was_interrupted () && typeid (*_GHom::get_concret (h)) == typeid (Fixpoint))
    return false;
  return true;
}

static HomCache cache;
typedef Cache<GHom,GDDD,GDDD,char> ImgHomCache;

template <>
GDDD ImgHomCache::eval(const GHom & func, const GDDD  & param) const {
  return _GHom::get_concret(func)->has_image_skip(param);
}

static ImgHomCache imgcache;

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

GDDD GHom::has_image(const GDDD &d) const {
    if(concret->immediat)
    {
      return concret->has_image(d);
    }
    else
    {
      if (d == GDDD::null) 
        {
	  return d;
        }
      else
        {
	  return (imgcache.insert(*this,d)).second;
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
  imgcache.clear();
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
GHom fixpoint (const GHom &h, bool is_top_level) {
	if( typeid( *_GHom::get_concret(h) ) == typeid(Fixpoint)
	   || h == GHom::id  || h.is_selector() || h == GHom(GDDD::null))
		return h;
	
    return Fixpoint(h, 0, is_top_level);
}


GHom monotonic (const d3::set<GHom>::type & set) {
  return Monotonic(set);
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
    if (cond == GHom::id) {
        return GDDD::null;
    } else if (cond == GDDD::null) {
        return GHom::id;
    } else if (const NotCond * hNot = dynamic_cast<const NotCond *> ( _GHom::get_concret(cond) )) {
        std::cerr << "double not simplification" << std::endl;
        return hNot->cond_;
    }
  return NotCond(cond);
}

// add an operand to a commutative composition of hom
static void addCompositionParameter (const GHom & h, And::parameters_t & args) {
	// associativity : a && (b && c) = a && b && c
	if ( const And * hAnd = dynamic_cast<const And*> ( _GHom::get_concret(h) ) ) {
		// recursively add each parameter
		for (And::parameters_it it = hAnd->parameters.begin() ; it != hAnd->parameters.end() ; ++it ) {
			addCompositionParameter (*it, args) ;
		}
	} else {
		// partition args into elements that commute with h or not
		And::parameters_t argsC, argsNOTC;
		for (And::parameters_it it = args.begin() ; it != args.end() ; ++it ) {
			if ( commutative (*it, h) ) {
				argsC.push_back(*it);
			} else {
				argsNOTC.push_back(*it);
			}
		}
		args = argsC;
		
		if ( argsNOTC.empty() ) {
			args.push_back ( h );
		} else if ( argsNOTC.size() == 1 ) {
			GHom h1 = *argsNOTC.begin();
			// let the user-defined semantic composition apply
			args.push_back ( Compose ( h1, h ) );
		} else {
			args.push_back ( Compose ( And (argsNOTC), h) );    
		}
	}
}

GHom operator&(const GHom &h1,const GHom &h2){
  GHom nullHom = GDDD::null;
  if (h1 == nullHom || h2 == nullHom)
    return nullHom;

  if( h1 == GHom::id )
    return h2;

  if( h2 == GHom::id )
    return h1;

  GHom h = h1.compose(h2);
  if (typeid(*_GHom::get_concret(h)) != typeid(Compose)) {
    return h;
  }
  // Test commutativity of h1 and h2
  And::parameters_t args;
  addCompositionParameter (h1, args);
  addCompositionParameter (h2, args);
  if ( args.empty() ) {
    std::cerr << " WTF ?? (SHOM.cpp in operator&)" << std::endl;
    return GHom(GDDD::null);
  } else if ( args.size() == 1 ) {
    return *args.begin();
  } else {
    return And (args);
  }
  //return GHom(canonical( Compose(h1,h2)));
}


GHom operator+(const GHom &h1,const GHom &h2){
  d3::set<GHom>::type s;
  s.insert(h1);
  s.insert(h2);
//  return(new Add(s));
  return GHom::add(s);
}

GHom operator*(const GDDD &d,const GHom &h){
  return Mult(h,d);
}

GHom operator*(const GHom &h,const GDDD &d){
  return Mult(h,d);
}

GHom operator*(const GHom & h,const GHom & cond) {
  if (! cond.is_selector() ) {
    std::cerr << "Creating a * intersection between homomorphisms, but second argument is not a selector. " << std::endl;
    //printCondError(cond);
    assert(false);
  }
  if ( h == GDDD::null || cond == GDDD::null )
    return GDDD::null;
  
  // trivial case
  if ( h == cond )
    return h;
  return Inter(h,cond);
}

GHom operator^(const GDDD &d,const GHom &h){
  return LeftConcat(d,h);
}

GHom operator^(const GHom &h,const GDDD &d){
  return RightConcat(h,d);
}

GHom operator-(const GHom &h,const GDDD &d){
  return Minus(h,d);
}

GHom apply2k (const GDDD & d) {
  if (d == GDDD::null || d == GDDD::one || d == GDDD::top ) {
    return GDDD::null;
  }
  return Apply2k(d);
}

/*************************************************************************/
/*                         Class MyGHom                                    */  
/*************************************************************************/

bool _GHom::operator< (const _GHom &h) const {
  return creation_counter > h.creation_counter;
}

void GHom::pstats(bool)
{
  std::cout << "*\nGHom Stats : size unicity table = " <<  canonical.size() << std::endl;
  
#ifdef HASH_STAT
  std::cout << std::endl << "GHom Unicity table stats :" << std::endl;
  print_hash_stats(canonical.get_hits(), canonical.get_misses(), canonical.get_bounces());

  std::cout << "GHom cache stats : " << std::endl;
  print_hash_stats(cache.get_hits(), cache.get_misses(), cache.get_bounces());
#endif // HASH_STAT
  
  std::cout << "sizeof(_GHom):" << sizeof(_GHom) << std::endl; 
  std::cout << "sizeof(Identity):" << sizeof(Identity) << std::endl; 
   
  
  /*
  std::ostream & os = std::cout;
  int i = 0;
  for (UniqueTable<_GHom>::Table::const_iterator it= canonical.table.begin() ;
       it != canonical.table.end();
       ++it ){
    os << i++ << " : " ;
    (*it)->print(os);
    os << std::endl;
  }
*/
}


// pretty print
std::ostream & operator << (std::ostream & os, const GHom & h) {
  h.concret->print(os);
  return os;
}
