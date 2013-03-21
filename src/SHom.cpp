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
#include <cassert>
#include <iostream>

#include "MemoryManager.h"
#include "util/configuration.hh"
#include "util/hash_support.hh"
#include "util/ext_hash_map.hh"
#include "Cache.hh"
#include "MLSHom.h"

#include "FixObserver.hh"

#ifdef PARALLEL_DD
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/concurrent_vector.h>
#endif

#define trace while(0) std::cerr
// #define trace std::cerr

namespace d3 { namespace util {
  template<>
  struct equal<_GShom*>{
    bool operator()(_GShom * _h1,_GShom * _h2){
      return (typeid(*_h1)==typeid(*_h2)?(*_h1)==(*_h2):false);
    }
  };
}}

/*************************************************************************/
/*                         Class _GShom                                   */
/*************************************************************************/

typedef std::map<GSDD,DataSet*> GSDD_DataSet_map;

// Shom NameSpace
namespace sns {

  extern UniqueTable<_GShom> canonical;

  /**
   * Static definition of the Observer Fixpoint
   */
  static const IFixpointObserver* __fixpointObs = NULL;


  /************************** Identity */
  class Identity:public _GShom{
  public:
    /* Constructor */
    Identity(int ref=0):_GShom(ref){}

	// Overloaded
	bool immediat () const  { return true; }

	
    /* Compare */
    bool operator==(const _GShom&) const{return true;}
    size_t hash() const{return 17;}
    _GShom * clone () const {  return new Identity(*this); }

    bool is_selector () const {
      return true; 
    }

    bool
    skip_variable(int) const 
    {
      return true;
    }

    /* Eval */
    GSDD eval(const GSDD &d)const{
		return d;
    }

    GShom invert (const GSDD & pot) const { 
      return this;
    }

    bool has_image (const GSDD & d) const {
      return true;
    }

    void print (std::ostream & os) const {
      os << "SId";
    }
  };

  const _GShom * getIdentity() { return canonical(Identity(1)); }

  


  /************************** Constant */
  class Constant:public _GShom{
  private:
    GSDD value;
  public:
    /* Constructor */
    Constant(const GSDD &d,int ref=0):_GShom(ref),value(d){}

	// Overloaded
	bool immediat () const  { return true; }
	
    /* Compare */
    bool operator==(const _GShom &h) const{
      return value==((Constant*)&h )->value;
    }

    size_t hash() const{
      return value.hash();
    }

    _GShom * clone () const {  return new Constant(*this); }

    /* Eval */
    GSDD eval(const GSDD &d)const{
      return d==GSDD::null?GSDD::null:value;
    }

    /* Memory Manager */
    void mark() const{
      value.mark();
    }

    GShom invert (const GSDD & pot) const { 
      return pot;
    }

    bool has_image (const GSDD & d) const {
      return true;
    }

    bool is_selector () const {
      // the empty set is a kind of "false" selector
      return value == SDD::null ; 
    }

    void print (std::ostream & os) const {
      os << "(SConstant:" << value << ")";
    }

  };

  /************************** Apply 2k level SDD */
  class SApply2k:public _GShom{
  private:
    GSDD value;
  public:
    /* Constructor */
    SApply2k(const GSDD &d,int ref=0):_GShom(ref),value(d){}

    /* Compare */
    bool operator==(const _GShom &h) const{
      return value==((SApply2k*)&h )->value;
    }

    size_t hash() const{
      return value.hash() * 13;
    }

    _GShom * clone () const {  return new SApply2k(*this); }

    /* Eval */
    GSDD eval(const GSDD &d)const{
      if (d==GSDD::null || value == GSDD::null)
	return GSDD::null;

      d3::set<GSDD>::type toadd;
      // cant avoid quadratic complexity :(
      for (GSDD::const_iterator it1 = d.begin(); it1 != d.end() ; ++it1 ){
	for (GSDD::const_iterator it2 = value.begin(); it2 != value.end() ; ++it2 ){

	  // downcast this garbage to DDD
	  DDD newval = apply2k (* (const DDD*) it2->first ) ( * (const DDD *) it1->first );
	  if (newval != DDD::null) {
	    toadd.insert( GSDD(d.variable(), newval, apply2k(it2->second) (it1->second)));
	  }
	}
      }
      return SDED::add(toadd);
    }
      
    bool
    skip_variable(int var) const 
    {
      return var != value.variable();
    }

    /* Memory Manager */
    void mark() const{
      value.mark();
    }

    GShom invert (const GSDD & pot) const { 
      return pot;
    }

    bool has_image (const GSDD & d) const {
      return true;
    }

    bool is_selector () const {
      // the empty set is a kind of "false" selector
      return value == SDD::null ; 
    }

    void print (std::ostream & os) const {
      os << "(SApply2k:" << value << ")";
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
      return 83*left.hash()+53*right.hash();
    }

    _GShom * clone () const {  return new Mult(*this); }
    /* Eval */
    GSDD eval(const GSDD &d)const{
      /** optimized evaluation when left.skip : 
       *  prune at current level and drop a recursive call
       *  i.e. do OTF intersection with the constant */
      int variable=d.variable();
      if (left.skip_variable(variable)) {

	    /** CODE COPY PASTE FROM SDED_MULT OF 2 SDD : TODO : REFACTOR THIS ALGO TO SHARE CODE */
	    std::map<GSDD,DataSet *> res;
	    
	    GSDD s1inters2 ;
	    // for each son of p1 :   v - a -> s1 
	    for (GSDD::Valuation::const_iterator it = d.begin();it != d.end() ; ++it) {
	      // for each son of p2 the constant :   v - b -> s2 
	      for (GSDD::Valuation::const_iterator jt = right.begin();jt != right.end() ; ++jt) {
		// test for equality first, fastest test
		if ( it->first->set_equal(*jt->first) ) {
		  // Recurse : left skips so is unchanged, it is the parameter, jt is the son of the constant
		  square_union(res, (left * jt->second)  (it->second), it->first);
		  // break out of inner loop
		  break;
		}
		// compute a*b
		DataSet *ainterb = it->first->set_intersect(*jt->first);
		// if a*b = 0, skip
		if (ainterb->empty() ) {
		  delete ainterb;
		  continue;
		}
		// Recurse since we have a non empty intersection case
		s1inters2 = (left * jt->second)  (it->second) ;
		square_union(res,s1inters2,ainterb);
		
		// b contains a
		if ( it->first->set_equal(*ainterb) ) {
		  delete ainterb;
		  // we can stop exploring for this operand
		  break;
		}
		
		delete ainterb;
	      }
	    }
	    
	    GSDD::Valuation value;
	    std::map<GSDD,DataSet *>::iterator nullmap = res.find(GSDD::null);
	    if (nullmap != res.end()){
	      delete nullmap->second;
	      res.erase(nullmap);
	    }
	    value.reserve(res.size());  
	    for (std::map<GSDD,DataSet *>::iterator it =res.begin() ;it!= res.end();++it)
	      value.push_back(std::make_pair(it->second,it->first));
	    
	    return GSDD(variable,value);
	    
	  }
      return left(d)*right;
    }

    bool is_selector () const {
      // intersection is a natural selector (if we forget about TOP)
      return left.is_selector() ;
    }

    GShom invert (const GSDD & pot) const { 
      // (h * c) (s) = h(s) * c
      // (h * c)^-1 (s) = h^-1 ( s * c) = ( h^-1 & ( id * c ) ) (s)
      return left.invert(pot) &  ( GShom::id * right ) ;
    }

    /* Memory Manager */
    void mark() const{
      left.mark();
      right.mark();
    }

    void print (std::ostream & os) const {
      os << "(SMult:" << left << "*" << right << ")";
    }

  };



  class Inter:public _GShom{
  private:
    GShom left;
    GShom right;
  public:
    /* Constructor */
    Inter(const GShom &l,const GShom &r,int ref=0):_GShom(ref),left(l),right(r){}
    /* Compare */
    bool operator==(const _GShom &h) const{
      return left==((Inter*)&h )->left && right==((Inter*)&h )->right;
    }
    size_t hash() const{
      return 83*left.hash()+ 153*right.hash();
    }

    _GShom * clone () const {  return new Inter(*this); }
    /* Eval */
    GSDD eval(const GSDD &d)const{
      return left(d) * right(d);
    }

    GShom invert (const GSDD & pot) const { 
      // (h * h')^-1 (s) =( h^-1 ( pot ) +  h^-1 (pot) ) (s) 
      return (left.invert(pot) + right.invert(pot)) ;
    }

    bool has_image (const GSDD & d) const {
      if (is_selector())
	return left.has_image(d) && right.has_image(d);
      else
	return _GShom::has_image(d);
    }


    bool is_selector () const {
      // intersection is a natural selector (if we forget about Top)
      // if either left or tight is a selector.
      return left.is_selector() && right.is_selector();
    }

    bool
    skip_variable(int var) const 
    {
      return get_concret(left)->skip_variable(var)
	&& get_concret(right)->skip_variable(var);
    }


    /* Memory Manager */
    void mark() const{
      left.mark();
      right.mark();
    }

    void print (std::ostream & os) const {
      os << "(SInter:" << left << "*" << right << ")";
    }

  };

  /** Extractor of variable domains for invert computations */
  class SDomExtract
    :
    public _GShom
  {

  public:

    int target;

    SDomExtract()
      :
      target(0)
    {}

    SDomExtract (int t) :target(t) {}


    // this hom is a heavy modifier
    bool skip_variable (int var) const {
      return false;
    }

    bool is_selector () const {
      return false;
    }
  
    GSDD eval(const GSDD &d)const {
      if (d == GSDD::one || d == GSDD::null || d == GSDD::top )
	return d;

      d3::set<GSDD>::type sum;

      if (d.variable() != target) {
	// destroy/propagate
	for ( GSDD::const_iterator it = d.begin(); it != d.end(); ++it)
	  sum.insert( GShom(this) (it->second) );
      } else {
	// grab all arc values and fuse them
	for ( GSDD::const_iterator it = d.begin(); it != d.end(); ++it)
	  sum.insert( GSDD (target,* it->first) );
      }

      return SDED::add(sum);
    }
  
    size_t hash() const {
      return  (target-253) * 2196727; 
    }

    bool operator==(const _GShom &s) const {
      const SDomExtract* ps = (const SDomExtract *)&s;
      return target == ps->target ;
    }  

    _GShom * clone () const {  return new SDomExtract(*this); }

    void print (std::ostream & os) const {
      os << "(SDomExtract:" << target << ")";
    }

  };
  /*************************************************************************/
  /*                         Class LocalApply : Hom version                */
  /*************************************************************************/

  class LocalApply
    :
    public _GShom
  {

  public:

    GHom h;
    int target;

    LocalApply()
      :
      h(), target(0)
    {}

    LocalApply (const GHom& hh,int t) :h(hh),target(t) {}


    // optimize away needless exploration of upstream modules that dont contain the place
    bool skip_variable (int var) const {
      return var != target;
    }

    bool is_selector () const {
      return h.is_selector();
    }

    const GShom::range_t  get_range () const {
      GShom::range_t range;
      range.insert(target);
      return range;
    }
  
    GSDD eval(const GSDD &d)const{
      GSDD_DataSet_map res;
      if (d == GSDD::one || d == GSDD::null || d == GSDD::top )
	return d;
      // for square union
      d3::set<GSDD>::type sum;

      // add application of h(arcval)
      for( GSDD::const_iterator it = d.begin();
	   it != d.end();
	   ++it)
	{
	  assert( typeid(*it->first) == typeid(const DDD&) );
	  DDD v2 = h((const DDD &)*it->first);
	  if( ! (v2 == GDDD::null) )
	    {
	      sum.insert(GSDD(d.variable(), v2, it->second));
	    }
	}
      if (sum.empty())
	return SDD::null;
      else
	return SDED::add(sum);
    }

    void mark() const {
      h.mark();
    }  
  
    size_t hash() const {
      return  h.hash() ^ target * 21727; 
    }

    GShom invert (const GSDD & pot) const { 
      GSDD localpot = (extractPotential(target)) (pot);
      GSDD::const_iterator gi = localpot.begin();
      return localApply ( h.invert( *  ((const DDD *) gi->first) ), target)  ;
    }


    bool operator==(const _GShom &s) const {
      const LocalApply* ps = (const LocalApply *)&s;
      return target == ps->target && h ==  ps->h;
    }  

    _GShom * clone () const {  return new LocalApply(*this); }

    void print (std::ostream & os) const {
      os << "(Local:" << h << "," << target << ")";
    }



  };

  /*************************************************************************/
  /*                         Class LocalApply : SHom version                */
  /*************************************************************************/

  class SLocalApply
    :
    public _GShom
  {

  public:

    GShom h;
    int target;

    SLocalApply()
      :
      h(), target(0)
    {}

    SLocalApply (const GShom& hh,int t) :h(hh),target(t) {}

    // optimize away needless exploration of upstream modules that dont contain the place
    bool skip_variable (int var) const {
      return var != target;
    }
  
    const GShom::range_t  get_range () const {
      GShom::range_t range;
      range.insert(target);
      return range;
    }
  

    GSDD eval(const GSDD &d) const {
      if (d == GSDD::one || d == GSDD::null || d == GSDD::top )
	return d;
      // for square union
      d3::set<GSDD>::type sum;

      // add application of h(arcval)
      for( GSDD::const_iterator it = d.begin();
	   it != d.end();
	   ++it)
	{
	  assert( typeid(*it->first) == typeid(const GSDD&) );
	  GSDD v2 = h((const GSDD &)*it->first);
	  if( ! (v2 == GSDD::null) )
	    {
	      sum.insert(GSDD(d.variable(), v2, it->second));
	    }
	}
      if (sum.empty())
	return SDD::null;
      else
	return SDED::add(sum);
    }

    void mark() const {
      h.mark();
    }  
  
    bool is_selector () const {
      return h.is_selector();
    }

    size_t hash() const {
      return  h.hash() ^ target * 2177; 
    }

    GShom invert (const GSDD & pot) const {
      GSDD localpot = (extractPotential(target)) (pot);
      GSDD::const_iterator gi = localpot.begin();
      return localApply ( h.invert( *  ((const SDD *) gi->first) ), target)  ;
    }

    bool has_image (const GSDD & d) const {
      for (SDD::const_iterator it = d.begin() ; it != d.end() ; ++it) 
	if (h.has_image( * ((const SDD *) it->first)))
	  return true;
      return false;
    }

    bool operator==(const _GShom &s) const {
      const SLocalApply* ps = (const SLocalApply *)&s;
      return target == ps->target && h ==  ps->h;
    }  

    _GShom * clone () const {  return new SLocalApply(*this); }

    void print (std::ostream & os) const {
      os << "(SLocal:" << h << "," << target << ")";
    }


  };

  // negator for a selector

  class SNotCond
    :
    public _GShom
  {
  public :
    // selector hom
    GShom cond_;


    SNotCond (const GShom & cond): cond_(cond) {};

    // skip if every argument skips.
    bool skip_variable (int var) const {
      return cond_.skip_variable(var);
    }

    const GShom::range_t  get_range () const {
      return cond_.get_range() ;
    }

    //    GShom invert (const GSDD & pot) const { 
      // (! sel)^-1 = pot - !sel(pot) + s = ( (pot - !sel(pot)) + Id )
      // return  (pot - (GShom(this)(pot))) + GShom::id ;
      // NEW version : (!sel)^-1 = !sel already default behavior
    // }


    bool is_selector () const {
      return true;
    }
  
    GSDD eval(const GSDD &d) const {
      if (d == GSDD::one || d == GSDD::null || d == GSDD::top )
	return d;
   
      GSDD condtrue = cond_ (d);
      return (d - condtrue);
    }


    void mark() const {
      cond_.mark();
    }  
  
    size_t hash() const {
      return  cond_.hash() * 6353; 
    }

    bool operator==(const _GShom &s) const {
      const SNotCond* ps = (const SNotCond *)&s;
      return cond_ == ps->cond_ ;
    }  

    _GShom * clone () const {  return new SNotCond(*this); }

    void print (std::ostream & os) const {
      os << "(NOT: ! " << cond_  << ")";
    }
  };

  /************************** And */
  /** A commutative composition of n homomorphisms */
  class And
    :
    public _GShom
  {
  public :
    typedef std::vector<GShom> parameters_t;
    typedef parameters_t::const_iterator parameters_it;
    /// PLEASE DONT HURT ME !!
    parameters_t parameters;

  public :
    And(const parameters_t & p, int ref=0)
      :
      _GShom(ref),
      parameters(p) {
//      assert (! p.empty());
//      assert(p.size() > 1);
    }
      
    /* Compare */
    bool
    operator==(const _GShom &h) const
    {
      return parameters == ((And*)&h )->parameters;	
    }

    size_t
    hash() const
    {
      size_t res = 40693 ;
      for(parameters_it gi=parameters.begin();gi!=parameters.end();++gi)
	res^=gi->hash();
      return res;
    }

    _GShom * clone () const {  return new And(*this); }

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

    const GShom::range_t  get_range () const {
      GShom::range_t ret;
      for(parameters_it gi = parameters.begin(); gi != parameters.end(); ++gi ) {	     
	GShom::range_t pret = gi->get_range();
	if ( pret.empty() )
	  return pret;
	ret.insert(pret.begin() , pret.end()) ;
      }
      return ret;
    }


    /* Eval */
    GSDD
    eval(const GSDD& d)const {
      if( d == GSDD::null ) {
	  return GSDD::null;

      } else if (  d == GSDD::one || d == GSDD::top ) {
	GSDD res = d;
	// simply apply composition semantics
	for(parameters_it gi = parameters.begin(); gi != parameters.end(); ++gi ) {	     
	  res = (*gi) (res);
	}
	return res;
      } else {
	GSDD res = d;
	parameters_t F;
	int var = d.variable() ;
	for(parameters_it gi = parameters.begin(); gi != parameters.end(); ++gi ) {	     
	  if ( gi->skip_variable(var) ) {
	    F.push_back(*gi);
	  } else {
	    res = (*gi) (res);
	  }
	}
	GShom nextSel = GShom::id ;
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

    GShom invert (const GSDD & pot) const { 
      // (\AND_i h_i)^-1  = \AND_i h_i^-1
      GShom ret = GShom::id;
      for(parameters_it gi = parameters.begin(); gi != parameters.end(); ++gi ) {
	ret = ret & ( gi->invert(pot) );
      }
      return ret;
    }


    bool has_image (const GSDD & d) const {
      
      if( d == GSDD::null ||  d == GSDD::one || d == GSDD::top ) {	
	return _GShom::has_image(d);
      } else {

	parameters_t F;
	int var = d.variable();
	const LocalApply* ld3 = NULL;
	const SLocalApply* l = NULL;

	GHom local;
	GShom Slocal;
	for(parameters_it gi = parameters.begin(); gi != parameters.end(); ++gi ) {	     
	  if ( gi->skip_variable(var) ) {
	    F.push_back(*gi);
	  } else if ( (ld3 = dynamic_cast<const LocalApply*> ( get_concret(*gi) ) ))  {
	    local = ld3->h;
	    // looks good, l term identified
	    // l = *gi;
	  } else if (( l = dynamic_cast<const SLocalApply*> ( get_concret(*gi) )) )  {
	    Slocal = l->h;
     	    // looks good, l term identified
	    // l = *gi;
	  } else {
	    std::cerr << "Bad has_image case : And containing non local " << std::endl;
	    return _GShom::has_image(d);
	  }
	}

	GShom nextSel = GShom::id ;
	if (! F.empty()) {
	  if ( F.size() > 1 ) 
	    nextSel = And (F);
	  else 
	    nextSel = *F.begin();
	}
	


	for (SDD::const_iterator it = d.begin() ; it != d.end() ; ++it ){
	  if (l != NULL) {
	    if ( Slocal.has_image( *((const SDD *) it->first))  && nextSel.has_image(it->second))
	      return true;	    
	  } else if (ld3 != NULL) {
	    if ( local ( *((const DDD *) it->first)) != DDD::null  && nextSel.has_image(it->second))
	      return true;	  
	  } else {
	    if ( nextSel.has_image(it->second) ) {
	      return true;
	    }
	  }
	}
	return false;

      }
    }

    
    void print (std::ostream & os) const {
      os << "(SAnd:" ;
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

  class Add
    :
    public _GShom {

    // types
  public:
    typedef std::vector<GShom> Gset_t;
    typedef Gset_t::const_iterator Gset_it;
    
	
    typedef
    struct
    {
      GShom F;
      Gset_t G;
      const _GShom* L;
      bool has_local;
    } partition;

    typedef hash_map<int,partition >::type partition_cache_type;

  public:
    typedef std::vector<GShom> parameters_t ;
    typedef parameters_t::const_iterator parameters_it;
    // for direct manipulation in Fixpoint eval
    parameters_t parameters;
  private :
    mutable partition_cache_type partition_cache;
    bool have_id;

    
    /** apply factorization rules adapted to g = l & f */
    void factorizeByLevel ( Gset_t & G, int target) const {
      typedef d3::set<GShom>::type set_t;
      typedef set_t::iterator  set_it;
      set_t ret;
      
      /** First step : for any g1 = l1 & f1 and any g2 = l2 & f2, if l1 == l2 then rewrite into g' = g1 + g2 = l1 & (f1+f2) */
      typedef std::map<GShom, d3::set<GShom>::type> map_t;
      typedef map_t::iterator map_it;
      // map local to f terms
      map_t map_ltof;

      /** load the map */
      // traverse the set G, if g = l & f, place into map_ltof, else place in ret 
      for (Gset_it it = G.begin() ; it != G.end() ; ++it ) {
	// test if *it of the form l & f
	if (const And * hand = dynamic_cast<const And*> ( get_concret(*it) ) )  {
	  // to compute and store the f part of the composition
	  And::parameters_t newAnd;
	  // newAnd.reserve(hand->parameters.size());
	  // local part
	  GShom l;

	  bool niceform = false;
	  for (And::parameters_it gi= hand->parameters.begin();gi!=hand->parameters.end();++gi) {
	    if ( ! gi->skip_variable(target) ) {
	      if (  dynamic_cast<const LocalApply*> ( get_concret(*gi) ) )  {
		// looks good, l term identified
		l = *gi;
		niceform = true;

	      } else if ( dynamic_cast<const SLocalApply*> ( get_concret(*gi) ) )  {
		
		// looks good, l term identified
		l = *gi;
		niceform = true;

	      } else {
		// not a local, skip this g term
		// looks bad : we have a term in this composition which does not skip, and is not a local. Danger here, so revert to basic implementation.
		niceform = false;
		break;
	      }
	    } else {
	      newAnd.push_back(*gi);
	    }
	  }
	  if ( niceform) {
	    // most conditions seem ok; this g term is of the form  l & And(newAnd)
	    
	    // try to add into local to f mapping
	    GShom fterm;
	    if (newAnd.size() > 1)
	      fterm = And(newAnd);
	    else
	      fterm = *newAnd.begin();
	    d3::set<GShom>::type sf;
	    sf.insert(fterm);

	    std::pair<map_it, bool> insertion = map_ltof.insert( map_t::value_type(l, sf) );
	    if (insertion.second) {
	      // did not exist, continue;
	    } else {
	      // already in map : Apply factorization rule
	      insertion.first->second.insert(fterm) ;
//	      trace << "factorization rule 1 successful ! \n" ;
	    }
	    
	  } else {
	    // not a nice l&f composition, skip this g term
	    ret.insert(*it);
	  }
	} else {
	  // not a nice l&f composition, skip this g term
	    ret.insert(*it);
	}
      }

      /** We have now obtained a partition of G into the terms of the form l & f, stored by l key in the map_ltof,
       *  and the "normal" or default g terms, that are still in G. */

      /** Now apply second factorization wrt. the f value */
      map_t map_ftol;
      for (map_it it = map_ltof.begin() ; it != map_ltof.end(); ++it ) {
	/** look if the term exists */
	d3::set<GShom>::type tmp;
	tmp.insert(it->first); 
	std::pair<map_it, bool> insertion = map_ftol.insert( map_t::value_type(GShom::add(it->second), tmp) );
	if (insertion.second) {
	  // did not exist, continue;
	} else {
	  // already in map : Apply factorization rule
	  insertion.first->second.insert(it->first) ;
//	  trace << "factorization rule 2 successful ! \n" ;
	}
      }

      /** Finally reinsert into the G set */
      for (map_it it = map_ftol.begin() ; it != map_ftol.end(); ++it ) {
	ret.insert( GShom::add(it->second) & it->first );
      }
      // reassign into G
      G = Gset_t ( ret.begin(), ret.end());
    }


  public:
        

    Add(const d3::set<GShom>::type& p, bool have_id): parameters(p.begin(),p.end()), have_id(have_id)
    {}

    bool
    get_have_id() const
    {
      return have_id;
    }

    /* Compare */
    bool
    operator==(const _GShom &h) const
    {
      return parameters == ((Add*)&h )->parameters;	
    }
    
    size_t
    hash() const
    {
      size_t res = 3821;
      for(parameters_it gi=parameters.begin();gi!=parameters.end();++gi)
	res^=gi->hash();
      return res;
    }

    _GShom * clone () const {  return new Add(*this); }

    bool has_image (const GSDD & d) const {
      for (parameters_it gi = parameters.begin(); gi != parameters.end(); ++gi ) 
	if ( gi->has_image(d))
	  return true;
      
      return false;
    }


    bool is_selector () const {
      for (parameters_it gi=parameters.begin();gi!=parameters.end();++gi)
	if (! gi->is_selector() )
	  return false;
      return true;
    }

    GShom invert (const GSDD & pot) const { 
      // (\ADD_i h_i)^-1  = \ADD_i h_i^-1
      d3::set<GShom>::type ops;
      for(parameters_it gi = parameters.begin(); gi != parameters.end(); ++gi ) {
	ops.insert(  gi->invert(pot) );
      }
      return GShom::add(ops);
    }



    bool
    skip_variable( int var ) const
    {
      partition_cache_type::const_accessor caccess;  
      if (!  partition_cache.find(caccess,var) ) {
	// miss
	partition_cache_type::accessor access ;
	partition_cache.insert(access,var);
	partition& part = access->second;
	part.has_local = false;
	part.L  = NULL;
	d3::set<GShom>::type F;
	d3::set<GShom>::type partG;
	for(parameters_it gi = parameters.begin();
		gi != parameters.end();
		++gi )
	  {
	    if( get_concret(*gi)->skip_variable(var) )
	      {
		// F part
		F.insert(*gi);
	      }
	    else if( typeid(*get_concret(*gi) ) == typeid(LocalApply) )
	      {
		// L part
		assert (!part.has_local);
		part.L = (const LocalApply*)(get_concret(*gi));
		part.has_local = true;
	      }
	    else if( typeid(*get_concret(*gi) ) == typeid(SLocalApply) )
	      {
		// L part
		assert (!part.has_local);
		part.L = (const SLocalApply*)(get_concret(*gi));
		part.has_local = true;
	      }
	    else
	      {
		// G part
		partG.insert(*gi);
	      }
	  }
	part.F = GShom::add(F);
	part.G = Gset_t (partG.begin(), partG.end());
	factorizeByLevel (part.G, var);
	return part.G.empty() && !part.has_local;
      }
      // cache hit
      return caccess->second.G.empty() && !caccess->second.has_local;
    }

    const GShom::range_t  get_range () const {
      GShom::range_t ret;
      for(parameters_it gi = parameters.begin(); gi != parameters.end(); ++gi ) {	     
	GShom::range_t pret = gi->get_range();
	if ( pret.empty() )
	  return pret;
	ret.insert(pret.begin() , pret.end()) ;
      }
      return ret;
    }


    partition
    get_partition(int var) const
    {
      this->skip_variable(var);
      partition_cache_type::const_accessor caccess;  
      partition_cache.find(caccess,var);
      return caccess->second;
    }


    /* Eval */
    GSDD
    eval(const GSDD& d)const
    {
      if( d == GSDD::null )
	{
	  return GSDD::null;
	}
      else if( d == GSDD::one || d == GSDD::top )
	{
	  d3::set<GSDD>::type s;
		
	  for(parameters_it gi=parameters.begin();gi!=parameters.end();++gi)
	    {
	      s.insert((*gi)(d));
	    }
	  return SDED::add(s);
	}
      else
	{
	  d3::set<GSDD>::type s;
	  int var = d.variable();

	  partition_cache_type::const_accessor part_it;
	  if( ! partition_cache.find(part_it,var) )
	    {
	      this->skip_variable(var);
	      partition_cache.find(part_it,var);
	    }

	  if( part_it->second.L != NULL )
	    {
	      s.insert(GShom(part_it->second.L)(d));
	    }
			
	  s.insert( part_it->second.F(d) );
	
	  const Gset_t & G = part_it->second.G;

	  for( 	Gset_it it = G.begin(); 
		it != G.end();
		++it )
	    {
	      s.insert((*it)(d));                  
	    } 
			
	  return SDED::add(s);
	}

    }

    /* Memory Manager */
    void
    mark() const
    {
      for( parameters_it gi=parameters.begin();
	   gi!=parameters.end();
	   ++gi)
	{
	  gi->mark();
	}
      // ready for garbage collect
      partition_cache.clear();
    }

    void print (std::ostream & os) const {
      os << "(SAdd:" ;
      parameters_it gi=parameters.begin();
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


  /************************** RecFireSat */

  // predeclaration
  GShom recFireSat (const GShom & sat, const GShom & lf) ;

  // TODO class RecFireSat
  class RecFireSat:public _GShom {
    GShom sat;
    GShom lf;
  public:
    /* Constructor */
    RecFireSat (const GShom &sat,const GShom &lf,int ref=0):_GShom(ref),sat(sat),lf(lf){}
    /* Compare */
    bool operator==(const _GShom &h) const{
      return sat==((RecFireSat*)&h )->sat && lf==((RecFireSat*)&h )->lf;
    }
    size_t hash() const{
      return 19381*sat.hash()+19681*lf.hash();
    }
    _GShom * clone () const {  return new RecFireSat(*this); }

    bool is_selector () const {
      return sat.is_selector() && lf.is_selector();
    }

    GShom invert (const GSDD & pot) const { 
      // (h & h')^-1  =  h'^-1 & h^-1
      return  lf.invert(pot) & sat.invert(pot);
    }


    bool
    skip_variable(int var) const 
    {
      return sat.skip_variable(var)
	&& lf.skip_variable(var);
    }

    const GShom::range_t  get_range () const {
      GShom::range_t ret = sat.get_range();
      if ( ret.empty() )
	return ret;
      GShom::range_t pret = lf.get_range();
      if ( pret.empty() )
	return pret;
      
      ret.insert(pret.begin() , pret.end()) ;
      return ret;
    }


    /* Eval */
    GSDD eval(const GSDD &d)const{
      if( d == GSDD::null )
	{
	  return GSDD::null;
	}
      else if( d == GSDD::one || d == GSDD::top )
	{
	  return (fixpoint ( sat ) & lf)(d);
	}
      else
	{


      int variable = d.variable();

	  trace << "Current lf = " << lf << std::endl;
	  trace << "Current sat = " << sat << std::endl;
	  trace << "Current variable = " << variable << std::endl;
      // The decomposition of lf into : lf = SLocal(lterm) & fterm in SDD case
      // or lf =  Local(ltermd3)  & fterm in DDD arc value case
      GShom lterm;
      GHom ltermd3;
      GShom fterm;

      bool isDDDarcval =  (typeid(*d.begin()->first) == typeid(const DDD&));

      // decompose lf into  l & f
      // A priori :  lf IS of the form l & f 
      if (const And * hand = dynamic_cast<const And*> ( get_concret(lf) ) )  {
	// to compute and store the f part of the composition
	And::parameters_t newAnd;
	newAnd.reserve(hand->parameters.size());
	// local part

	for (And::parameters_it gi= hand->parameters.begin();gi!=hand->parameters.end();++gi) {
	  if ( ! gi->skip_variable(variable) ) {
	    if (  dynamic_cast<const LocalApply*> ( get_concret(*gi) ) )  {
//	      isDDDarcval = true;
	      // looks good, l term identified
	      ltermd3 = ((const LocalApply*)get_concret(*gi)) ->h ;
	    } else if ( dynamic_cast<const SLocalApply*> ( get_concret(*gi) ) )  {
	      // looks good, l term identified
	      lterm = ((const SLocalApply*)get_concret(*gi)) ->h ;
	    } else {
	      // not a local, skip this g term
	      // looks bad : we have a term in this composition which does not skip, and is not a local. Danger here, so revert to basic implementation.
	      assert(false);
	    }
	  } else {
	    newAnd.push_back(*gi);
	  }
	}
	// most conditions seem ok; this g term is of the form  l & And(newAnd)
	if (! newAnd.empty())
	  fterm = And(newAnd);
      } else {
	trace<< "Built a RecFireSat with lf = " << lf << std::endl;
	return (fixpoint(sat) & lf) (d);
      }

      trace << "deduce lterm = "<< lterm << std::endl;
      trace << "deduce ltermd3 = "<< ltermd3 << std::endl;
      trace << "deduce fterm = "<< fterm << std::endl;
      trace << "deduce isDDDarcval = "<< isDDDarcval << std::endl;




      // Fsat will be RecFireSat ( sat.F , lf.f)
      GShom Fsat;
      // Lsat will be RecFireSat ( body(sat.L) , Lterm ) if SDD arc value
      // or  (body(sat.L) + id)^* & Ltermd3 : otherwise
      GShom Lsat;
      GHom Lsatd3;

      // True if there are some G transitions in sat
      bool hasGpart = false;

      if (const Add * add = dynamic_cast<const Add*> ( get_concret(sat) ) ) {
	// Check if we have ( Id + F + G )* where F can be forwarded to the next variable
	
	// Rewrite ( Id + F + G )*
	// into ( O(Gn + Id) o (O(Fn + Id)*)* )* 
	if( add->get_have_id() ) {                           
	  Add::partition partition = add->get_partition(variable);

	  hasGpart = ! partition.G.empty();
	  // partition.F contains id by construction
	  Fsat = partition.F ;
	  // Conceptually :
	  // Lsat = fixpoint(partition.L + GShom::id);
	  // but we need to ensure that  : (Local(l) + id)^* = Local( (l +id)^* ) 
	  // so this form for Lsat is built "by hand"
	  if (partition.has_local) {
	    if (const LocalApply * loc = dynamic_cast<const LocalApply*> (get_concret(*partition.L))) {
	      // Hom/DDD case
	      GHom hh = fixpoint(GHom(loc->h));
	      Lsatd3 =  hh  & ltermd3 ;
	    } else if (const SLocalApply* sloc = dynamic_cast<const SLocalApply*> ( get_concret(*partition.L) ) )  {
	      Lsat = sns::recFireSat(GShom(sloc->h), lterm );
	    } else {
	      // looks bad 
	      assert(false);
	    }
	  } else {
	    Lsat = lterm ;
	    Lsatd3 = ltermd3;
	  }
	} else {
	  // No identity in sat part ???
	  assert(false);
	}
      } else {
	// Sat is not a sum ??
	assert(false);
      }

      Fsat = recFireSat( Fsat, fterm);
      // Fsat will be RecFireSat ( sat.F , lf.f)
      trace << "Current Fsat = " << Fsat << std::endl;
      // Lsat will be RecFireSat ( body(sat.L) , Lterm ) if SDD arc value
      // or  (body(sat.L) + id)^* & Ltermd3 : otherwise
      trace << "Current Lsat = " << Lsat << std::endl;
      trace << "Current Lsatd3 = " << Lsatd3 << std::endl;

      // True if there are some G transitions in sat
      trace << "Current hasGpart = " << hasGpart << std::endl;


	
      // We now have collected the various elements for evaluation :
      // (G + F + L + id )^* & l&f (d) =>
      //    for ( (val,son) in arcs(d) ) {
      //            if (l.has_image(val)) {
      //                 son' = Fstar & f (son) = RecFireSat( F + id, f ) (son) = Fsat (son) 
      //                 val' = LStar & Local(l) (d) ~= (L + Id)^* & l (val) =  RecFireSat( L + id , l) (val) = Lsat(val); // or Lsatd3
      //                 sum.add( SDD(var, val', son')
      //            }
      //    }

      d3::set<GSDD>::type sum;      
      for (GSDD::const_iterator it= d.begin() ; it != d.end() ; ++it) {
	if (isDDDarcval) {
	  DDD val = ( DDD &) *it->first;
	  GSDD son = it->second;

	  if ( ltermd3 (val) != DDD::null ) {
	    son = Fsat(son);
	    if (son != SDD::null) {
	      val = Lsatd3 (val);

	      // Should not happen, since ltermd3 leaves some image and Lsatd3 should contain Id
	      assert(val != DDD::null); 

	      sum.insert( GSDD(variable, val, son) );
	    }
	  }

	} else {
	  GSDD val = (SDD &) *it->first;
	  GSDD son = it->second;
	  
// 	  trace << " Current value :" << std::endl;
// 	  trace << val ;
// 	  trace << " Current lterm :" << std::endl;
// 	  trace << lterm << std::endl;
// 	  trace << " Current ltermd3 :" << std::endl;
// 	  trace << lterm << std::endl;

	  if ( lterm.has_image(val)) {
	    son = Fsat(son);
	    if (son != SDD::null) {
	      val = Lsat (val);

	      if (val != SDD::null) {
		sum.insert( GSDD(variable, val, son) );
	      }
	    }
	  }
	       
	}	
      }
      if (sum.empty())
	return SDD::null;
      
      GSDD summed = SDED::add(sum);
      
      if ( hasGpart ) {
	return fixpoint(sat) (summed);
      } else {
	return summed;
      }
	}   
    }

    /* Memory Manager */
    void mark() const{
      sat.mark();
      lf.mark();
    }

    void print (std::ostream & os) const {
      os << "(RecFireSat:" << sat << " , " << lf << ")";
    }


  };



   GShom recFireSat (const GShom & sat, const GShom & lf) {
     if ( dynamic_cast<const Add*> ( _GShom::get_concret(sat) ) ) {       
       return RecFireSat(sat,lf);
     } else {       
       return sat & lf;
     }
   }

  /************************** Compose */
  class Compose:public _GShom{
  public:
    // made public for direct manipulation in Fixpoint::eval()
    GShom left;
    GShom right;
  public:
    /* Constructor */
    Compose(const GShom &l,const GShom &r,int ref=0):_GShom(ref),left(l),right(r){}
    /* Compare */
    bool operator==(const _GShom &h) const{
      return left==((Compose*)&h )->left && right==((Compose*)&h )->right;
    }
    size_t hash() const{
      return 13*left.hash()+7*right.hash();
    }
    _GShom * clone () const {  return new Compose(*this); }

    bool is_selector () const {
      return left.is_selector() && right.is_selector();
    }

    GShom invert (const GSDD & pot) const { 
      // (h & h')^-1  =  h'^-1 & h^-1
      return  right.invert(pot) & left.invert(pot);
    }


    bool
    skip_variable(int var) const 
    {
      return left.skip_variable(var)
	&& right.skip_variable(var);
    }

    const GShom::range_t  get_range () const {
      GShom::range_t ret = left.get_range();
      if ( ret.empty() )
	return ret;
      GShom::range_t pret = right.get_range();
      if ( pret.empty() )
	return pret;
      
      ret.insert(pret.begin() , pret.end()) ;
      return ret;
    }


    /* Eval */
    GSDD eval(const GSDD &d)const{
      return left(right(d));
    }

    bool has_image (const GSDD & d) const {
      if (! right.has_image(d)) {
	return false;
      }
      return _GShom::has_image(d);
    }


    /* Memory Manager */
    void mark() const{
      left.mark();
      right.mark();
    }

    void print (std::ostream & os) const {
      os << "(SCompose:" << left << " & " << right << ")";
    }


  };

  /************************** LeftConcat */
  class LeftConcat:public _GShom{
  private:
    GSDD left;
    GShom right;
  public:
    /* Constructor */
    LeftConcat(const GSDD &l,const GShom &r,int ref=0):_GShom(ref),left(l),right(r){}
    /* Compare */
    bool operator==(const _GShom &h) const{
      return left==((LeftConcat*)&h )->left && right==((LeftConcat*)&h )->right;
    }
    size_t hash() const{
      return 23*left.hash()+47*right.hash();
    }
    _GShom * clone () const {  return new LeftConcat(*this); }

    /* Eval */
    GSDD eval(const GSDD &d)const{
      return left^right(d);
    }

    //  not really sure how to implement this guy : default to assert(false)
    //GShom invert (const GSDD & pot) const ;

    bool has_image (const GSDD & d) const {
      return right.has_image(d);
    }


    /* Memory Manager */
    void mark() const{
      left.mark();
      right.mark();
    }

    void print (std::ostream & os) const {
      os << "(SLeftConcat:" << left << " ^ " << right << ")";
    }

  };

  /************************** RightConcat */
  class RightConcat:public _GShom{
  private:
    GShom left;
    GSDD right;
  public:
    /* Constructor */
    RightConcat(const GShom &l,const GSDD &r,int ref=0):_GShom(ref),left(l),right(r){}
    /* Compare */
    bool operator==(const _GShom &h) const{
      return left==((RightConcat*)&h )->left && right==((RightConcat*)&h )->right;
    }
    size_t hash() const{
      return 47*left.hash()+19*right.hash();
    }
    _GShom * clone () const {  return new RightConcat(*this); }

    bool
    skip_variable(int var) const
    {
      return get_concret(left)->skip_variable(var);
    }

    //  not really sure how to implement this guy : default to assert(false)
    //GShom invert (const GSDD & pot) const ;

    bool has_image (const GSDD & d) const {
      return left.has_image(d);
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

    void print (std::ostream & os) const {
      os << "(SRightConcat:" << left << " ^ " << right << ")";
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
      return 5*left.hash()+61*right.hash();
    }
    _GShom * clone () const {  return new Minus(*this); }

    /* Eval */
    GSDD eval(const GSDD &d)const{
      return left(d)-right;
    }


    GShom invert (const GSDD & pot) const {
      // (h - cte)^-1 (s) = h^-1 (s + cte) =  h^-1 & ( id + cte) 
      return left.invert(pot) & (GShom::id + right);
    }


    bool is_selector () const {
      // set difference is a natural selector
      return left.is_selector() ;
    }
    /* Memory Manager */
    void mark() const{
      left.mark();
      right.mark();
    }

    void print (std::ostream & os) const {
      os << "(SMinus:" << left << " - " << right << ")";
    }

  };


  /************************** HomMinus */
  class HomMinus:public _GShom{
  private:
    GShom left;
    GShom right;
  public:
    /* Constructor */
    HomMinus(const GShom &l,const GShom &r,int ref=0):_GShom(ref),left(l),right(r){}
    /* Compare */
    bool operator==(const _GShom &h) const{
      return left==((HomMinus*)&h )->left && right==((HomMinus*)&h )->right;
    }
    size_t hash() const{
      return 15971*left.hash()+24023*right.hash();
    }
    _GShom * clone () const {  return new HomMinus(*this); }

    /* Eval */
    GSDD eval(const GSDD &d)const{
      return left(d)-right(d);
    }


    GShom invert (const GSDD & pot) const {
      // a bit too large a set... but how to compute a better approximation ??
      // (h1 - h2)^-1 (s) = h1^-1 (s)  + h2^-1 (pot -s) 
      std::cerr << "/!\\ Be careful, you are using invert of a minus between two homomorphisms, and I am not sure the invert is correct. FIXME !" << std::endl;
      return left.invert(pot) + (right.invert(pot) & (pot - GShom::id)) ;
    }


    bool is_selector () const {
      // set difference is a natural selector, if the left hom is a selector
      return left.is_selector() ;
    }
    /* Memory Manager */
    void mark() const{
      left.mark();
      right.mark();
    }

    void print (std::ostream & os) const {
      os << "(SHomMinus:" << left << " - " << right << ")";
    }

  };

  /************************** Fixpoint */


  class Fixpoint
    : public _GShom
  {

  private:
    GShom arg;
    bool can_garbage;
  public:

    /* Constructor */
    Fixpoint(const GShom &a,int ref=0,bool can_garbage=false):_GShom(ref),arg(a),can_garbage(can_garbage){}
    /* Compare */

    bool operator==(const _GShom &h) const{
      return arg==((Fixpoint*)&h )->arg ;
    }

    size_t hash() const {
      return 17*arg.hash();
    }
    _GShom * clone () const {  return new Fixpoint(*this); }

    bool
    skip_variable(int var) const
    {
      return arg.skip_variable(var);
    }

    const GShom::range_t  get_range () const {
      return arg.get_range();
    }

    bool is_selector () const {
      // wow ! why build a fixpoint of a selector ??
      return arg.is_selector();
    }

    GShom invert (const GSDD & pot) const {
      // No correct solution here : many solutions may exist.
      // simply add states that go to the target states (e.g. prefixes of cycles)
      // (h^*)^-1 (s) = (h^-1 + id)^*  
      return fixpoint ( arg.invert(pot) + GShom::id );
    }


    /* Eval */
    GSDD 
    eval(const GSDD &d) const
    {
      if( d == GSDD::null )
	{
	  return GSDD::null;
	}
      else if( d == GSDD::one || d == GSDD::top )
	{
	  return arg(d);
	}
      else
	{
	  int variable = d.variable();

	  GSDD d1 = d;
	  GSDD d2 = d;

	  // is it the fixpoint of an union ?
	  if (const Add * add = dynamic_cast<const Add*> ( get_concret(arg) ) )
	    {
	      // Check if we have ( Id + F + G )* where F can be forwarded to the next variable
		
	      // Rewrite ( Id + F + G )*
	      // into ( O(Gn + Id) o (O(Fn + Id)*)* )* 
	      if( add->get_have_id() )
		{                           
		  Add::partition partition = add->get_partition(variable);

      if (can_garbage && partition.F == GShom::id && partition.G.empty()) {
        Shom L_part = GShom::id;
        if (partition.has_local) {
          if (const LocalApply * loc = dynamic_cast<const LocalApply*> (partition.L)) {
            // Hom/DDD case
            GHom hh = fixpoint(GHom(loc->h), true);
            L_part =  localApply( hh ,variable);
          } else {	
            GShom hh = fixpoint(GShom( ((const SLocalApply*)partition.L) ->h), true);
            L_part =  localApply( hh ,variable);
          }
        }
        d.mark();
        Shom tt = Shom(this);
        return L_part(d);
      }
      
		  // operations that can be forwarded to the next variable
		  GShom F_part = fixpoint(partition.F);

				
		  GShom L_part ;
		  if (partition.has_local) {
		    if (const LocalApply * loc = dynamic_cast<const LocalApply*> (partition.L)) {
		      // Hom/DDD case
		      GHom hh = fixpoint(GHom(loc->h));
		      L_part =  localApply( hh ,variable);
		    } else {	
		      GShom hh = fixpoint(GShom( ((const SLocalApply*)partition.L) ->h));
		      L_part =  localApply( hh ,variable);
		    }
		  } else {
		    L_part = GShom::id ;
		  }

		  if (GShom::getSaturationStrategy() == GShom::RECFIREANDSAT) {

		    /// Step 1 : identify G = G_part + LF_part such that g \in LF_part <=> g = l & f
		    Add::Gset_t G_part;
		    Add::Gset_t LF_part;
		    GShom LF_sat;		    
		    {
		      d3::set<GShom>::type add_param;
		      add_param.insert(L_part);
		      add_param.insert(partition.F);
		      add_param.insert(Shom::id);
		      LF_sat = GShom::add(add_param);
		      // release add_param
		    }
		    trace << "In fixpoint of sum : " << arg << std::endl;
		    trace << "Variable = " << variable << std::endl;
		    trace << "Deduced LF_sat = " << LF_sat << std::endl;

		    // fill G with the updated elements in partition.G
		    for( 	Add::Gset_it G_it = partition.G.begin();
				G_it != partition.G.end();
				++G_it) 
		    {
		      trace << "testing g element : " << *G_it<< std::endl;
		      // find elements of the form l & f
		      // test if *it of the form l & f
		      if (const And * hand = dynamic_cast<const And*> ( get_concret(*G_it) ) )  {
			bool niceform = true;

			for (And::parameters_it gi= hand->parameters.begin();gi!=hand->parameters.end();++gi) {
			  if (  dynamic_cast<const LocalApply*> ( get_concret(*gi) )) {
			    continue;
			  }  else if ( dynamic_cast<const SLocalApply*> ( get_concret(*gi) ) )  {
			    continue;			    
			  } else {
			    // not a local, add this g term to G_part
			    // looks bad : we have a term in this composition which does not skip, and is not a local. Danger here, so revert to basic implementation.
			    niceform = false;
			    break;
			  }
			}

		    
			if (niceform) {
			  trace << "IS nice form"<< std::endl;
			  // Use the recursive fire and sat rewriting
			  // (F + L + id + l&f)^* (d) = (L+F+id)^* & (l&f) (d) = RecFireSat( L+F+id , l&f ) (d);   			  
			  LF_part.push_back( recFireSat(LF_sat,*G_it) );
			} else {
			  // Not nice although it is a And commutative composition
			  // Could still be handled ???
			  trace << "IS almost nice form"<< std::endl;
			  G_part.push_back(*G_it);
			
			}

		      } else {
			  trace << "IS NOT  nice form"<< std::endl;
			// Not nice 
			G_part.push_back(*G_it);
		      }
		    }


		    do
		      {
			d1 = d2;

			d2 = F_part(d2);
			d2 = L_part(d2);

			// Elements in G' : not a nice l&f form : default to usual behavior
			for( 	Add::Gset_it G_it = G_part.begin();
				G_it != G_part.end();
				++G_it) 
			  {
			    // BFS
			    // chain application of Shom of this level
			    d2 =  ( (L_part &  (*G_it))(d2)) + d2;
			    
			  }
			
			
			for( 	Add::Gset_it G_it = LF_part.begin();
				G_it != LF_part.end();
				++G_it) 
			  {
			    // Use the recursive fire and sat rewriting
			    // Elements in LF_part have been built to be of the form :
			    // (F + L + id + l&f)^* (d) = RecFire[(L+F+id)^* & (l&f)] (d);   
			    trace << "HIT" << std::endl;
			    
			    d2 =  (*G_it)(d2) + d2;
			    
			  }

            if (! can_garbage && fobs::get_fixobserver ()->should_interrupt ())
            {
              return d2;
            }
			if (can_garbage) {
		    	  /* Call the fixpoint Observer */
		    	  if (sns::__fixpointObs != NULL){
			    sns::__fixpointObs->update(d2,d1);
			    /* We must continue */
			    if (sns::__fixpointObs->shouldInterrupt())
			      /* BREAK THE LOOP and return result */
			      return d1;
		    	  }
		    	  //std::cout << d1.nbStates() << std::endl;
			  //			std::cerr << "could trigger !!" << std::endl ;
        
        if (fobs::get_fixobserver ()->should_interrupt ())
        {
          fobs::get_fixobserver ()->update (d2, d1);
          if (fobs::get_fixobserver ()->should_interrupt ())
          {
            return d2;
          }
        }
        
			  if (MemoryManager::should_garbage()) {
			    //			  std::cerr << "triggered !!" << std::endl ;
			    // ensure d1 and d2 are preserved
			    d1.mark();
			    d2.mark();
			    // ensure current operands are preserved
			    F_part.mark();
			    L_part.mark();
			    Shom tt = Shom(this);

			    for( 	Add::Gset_it G_it = partition.G.begin();
					G_it != partition.G.end();
					++G_it) 
			      G_it->mark();
			    MemoryManager::garbage();

			  }
			}

		      }
		    while (d1 != d2);
		    //__cpt += d1.nbStates();
		    //std::cout << d1.nbStates()  << " : " << __cpt << std::endl;
		    return d1;
		  


		  ///END RECFIREANDSAT CASE
		} else {

				
		do
		  {
		    d1 = d2;

		    d2 = F_part(d2);
		    d2 = L_part(d2);

		    for( 	Add::Gset_it G_it = partition.G.begin();
				G_it != partition.G.end();
				++G_it) 
		      {

			// d2 = F_part(d2);
			// 						d2 = L_part(d2);

			// apply local part
			// d2 = L_part(d2);

			if (GShom::getFixpointStrategy() == GShom::DFS) {
			  // saturate firings of each transition (for non deterministic : one to many transitions).
			  // do an internal fixpoint on every g \in G, i.e. 
			  // (\sum_i (g_i + id)\star) \star			  
			  GSDD d3 = d2;
			  do {
			    d2 = d3;
			    d3 =  ( (L_part &  (*G_it))(d2)) + d2;
			  } while (d3 != d2);
			} else {
			  // BFS
			  // chain application of Shom of this level
			  d2 =  ( (L_part &  (*G_it))(d2)) + d2;
			}
		      }
        
        if (! can_garbage && fobs::get_fixobserver ()->should_interrupt ())
        {
          return d2;
        }
		    if (can_garbage) {
		      /* Call the fixpoint Observer */
		      if (sns::__fixpointObs != NULL){
			sns::__fixpointObs->update(d2,d1);
			/* We must continue */
			if (sns::__fixpointObs->shouldInterrupt())
			  /* BREAK THE LOOP and return result */
			  return d1;
		      }
		      //std::cout << d1.nbStates() << std::endl;
		      //			trace << "could trigger !!" << std::endl ;
          
          if (fobs::get_fixobserver ()->should_interrupt ())
          {
            fobs::get_fixobserver ()->update (d2, d1);
            if (fobs::get_fixobserver ()->should_interrupt ())
            {
              return d2;
            }
          }
          
          if (MemoryManager::should_garbage()) {
			//			  trace << "triggered !!" << std::endl ;
			// ensure d1 and d2 are preserved
			d1.mark();
			d2.mark();
			// ensure current operands are preserved
			F_part.mark();
			L_part.mark();
			Shom tt = Shom(this);

			for( 	Add::Gset_it G_it = partition.G.begin();
				G_it != partition.G.end();
				++G_it) 
			  G_it->mark();
			MemoryManager::garbage();

		      }
		    }

		  }
		while (d1 != d2);
		//__cpt += d1.nbStates();
		//std::cout << d1.nbStates()  << " : " << __cpt << std::endl;
		return d1;
	      }
		  
	    }
	}                                                                                               

	  do
	    {
	      d1 = d2;
	      d2 = arg(d2);
        
        if (! can_garbage && fobs::get_fixobserver ()->should_interrupt ())
        {
          return d2;
        }
        
	      if (can_garbage) {
	    	  /* Call the fixpoint Observer */
			  if (sns::__fixpointObs != NULL){
				  sns::__fixpointObs->update(d2,d1);
				  /* We must continue */
				  if (sns::__fixpointObs->shouldInterrupt())
					  /* BREAK THE LOOP and return result */
					  return d1;
			  }
//		trace << "could trigger 2!!" << std::endl ;
          
          if (fobs::get_fixobserver ()->should_interrupt ())
          {
            fobs::get_fixobserver ()->update (d2, d1);
            if (fobs::get_fixobserver ()->should_interrupt ())
            {
              return d2;
            }
          }
          
          if (MemoryManager::should_garbage()) {
//		  trace << "triggered !!" << std::endl ;
		  // ensure d1 and d2 and argument are preserved
		  d1.mark();
		  d2.mark();
		  arg.mark();
		  Shom tt = Shom(this);
		  
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
      os << "(SFix:" << arg << " *)";
    }

  };

	
class MLShomAdapter :public _GShom{
private:
  MLShom h;
public:
  /* Constructor */
  MLShomAdapter(const MLShom &hh):h(hh){}
  /* Compare */
  bool operator==(const _GShom &other) const{
    return h==((MLShomAdapter*)& other )->h;
  }
  size_t hash() const{
    return 19751*h.hash();
  }
  _GShom * clone () const {  return new MLShomAdapter(*this); }
  
  /* Eval */
  GSDD eval(const GSDD &d)const{
    SHomNodeMap m = h (d);
    std::set<GSDD> sum;
    for (SHomNodeMap::const_iterator it = m.begin() ; it != m.end() ; ++it) {
      sum.insert(it->first (it->second));
    }
    return SDED::add(sum);
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


} // end namespace H_Homomorphism




#ifdef PARALLEL_DD

typedef tbb::blocked_range<int> varval_range;


class hom_for
{
private:

  const GShom& gshom_;
  std::vector<GSDD> & val_;
    
  int* to_solve_;

public:

  hom_for( const GShom& ghsom
	   , std::vector<GSDD>& val
	   , int* to_solve
	   )

    : gshom_(ghsom)
    , val_(val)
    , to_solve_(to_solve)
  {
  }
	
  void operator()(const varval_range& range)
    const
  {
    std::vector<GSDD>& val = this->val_;
    int* to_solve = this->to_solve_;

    for( int i = range.begin(); i != range.end(); ++i)
      {
	// GSDD result = gshom_( val[to_solve[i]]) ;
	// sns::cache.insert( gshom_, val[to_solve[i]], result);
	// val[to_solve[i]] = result;
	val[to_solve[i]] = gshom_( val[to_solve[i]]) ;
      }

  }

};

#endif // PARALLEL_DD

GSDD 
_GShom::eval_skip(const GSDD& d) const
{
  if( d == GSDD::null )
    {
      return GSDD::null;
    }
  else if( d == GSDD::one )
    {
      // basic case, mustn't call d.variable()
    }
  else if( d == GSDD::top )
    {
      return GSDD::top;
    }
  else if( this->skip_variable(d.variable()) )
    {
      // build once, use many times on each son
      const GShom gshom(this);
      // Id replies skip true, for correct rewriting rules. But should evaluate now !
      if (gshom == GShom::id)
	return d;
      // for square union
      GSDD_DataSet_map res;
      
#ifdef PARALLEL_DD
      
      // filter pathological single son case
      // fallback to default except if parallel conditions met
      if (d.nbsons() > 1  && (typeid(*this) == typeid(const sns::Fixpoint))) 
	{

          // std::cout << "PARALLEL" << std::endl;

	  // To hold the arcs of the node we are constructing
	  // For each arc <vl,son> of the node d we build an arc
	  // <vl, h(son)>. 
	  // Then we use square union to ensure canonicity of this arc set.
      
	  // The node structure is preserved by application of h, since skip_var is true
	  std::vector<GSDD> son_result;
	  son_result.reserve(d.nbsons());
      
	  // Parallel computation of h(son) is possible.
	  // However using a task to get a cache hit is counter productive, 
	  // so we first get h(son) from cache where possible and update tmp_result
	  // for cache misses we build tosolve, that contains the index of uncomputed h(son) arcs in tmp_result.
	  // We then use a parallel loop to resolve the remaining computations pointed to in tosolve using tasks.
      
	  // to hold index of entries not found in cache. size at most full node size.
	  int to_solve[d.nbsons()];
	  int to_solve_size = 0;
      
	  // one loop to pick up cached results
      
	  // current index in tmp_result
	  int i =0;
	  for(  GSDD::const_iterator it = d.begin();
		it != d.end() ;
		++it,i++) {
	    if (immediat()) {
	      // right concatenating a constant ? ah : Can this happen ?
	      // if this assert is raised, remove it !
              son_result.push_back(eval(it->second));
              assert(false);
	    } else {
	      //  std::pair<bool,GSDD> local_res = sns::cache.contains(gshom,it->second);
	      //  if (local_res.first) {
	      if (false) {
		// cache hit
		//                  son_result.push_back(local_res.second);
	      } else {
		// cache miss : add index i to tosolve list
		to_solve[to_solve_size++] = i;
		// set current value in son_result to son
		son_result.push_back(it->second);
	      }
	    }
	  }
            
	  // the actual parrallel computation
	  //          for i in range given by varval_range : 0 < i < tosolvesize
	  // third parameter in range constructor is grain of parallelism : 1 => 1 task per arc created
	  tbb::parallel_for( varval_range( 0, to_solve_size, 1)
			     // for task body computes  : sonresult[tosolve[i] = gshom(sonresult[tosolve[i]]) 
			     , hom_for(gshom, son_result, to_solve));

      
	  // for( int j = 0; j != to_solve_size; ++j)
	  // {
	  //     GSDD arg = son_result[to_solve[j]];
	  //     GSDD result = gshom( arg) ;
	  //     // sns::cache.insert( gshom, arg, result);
	  //     son_result[to_solve[j]] = result;
	  // }


	  i=0;
	  for( GSDD::const_iterator it = d.begin()
		 ; it != d.end()
		 ; ++it,++i )
	    {
	      // arcs to null are pruned
	      if ( son_result[i] != GSDD::null && !(it->first->empty()))
		{
		  assert( son_result[i].variable() != d.variable() );
		  // use arc value from node d and new son from son_result
		  // note that arc values are copied into res, so d is const always
		  square_union( res, son_result[i] , it->first);
		}
	    }
      
	} else {
	// parallel conditions not enabled
	// std::cout << "SEQUENTIAL" << std::endl;

#endif
	// #else // NOT PARALLEL_DD      

	for( GSDD::const_iterator it = d.begin();
	     it != d.end();
	     ++it)
	  {
	    GSDD son = gshom(it->second);
	    if( son != GSDD::null && !(it->first->empty()) )
	      {
		square_union(res, son, it->first);
	      }
	  }
      
	// #endif // PARALLEL_DD
#ifdef PARALLEL_DD
      } // close else condition : no parallel
#endif      

      GSDD::Valuation valuation;
      valuation.reserve(res.size());  
      for ( GSDD_DataSet_map::const_iterator it = res.begin();
	    it!= res.end();
	    ++it)
	{
	  valuation.push_back(std::make_pair(it->second,it->first));
	}
      
      if( valuation.empty() )
        {
	  return GSDD::null;
        }
      else
	{
	  return GSDD(d.variable(),valuation);
	}
      
    }
  
  return eval(d);
}

GShom
_GShom::compose (const GShom &r) const
{
  //return GHom(this) & r; 
  
  // Note: code duplicated unfortunately
  GShom nullHom = GSDD::null;
  GShom thisH (this);
  if (thisH == nullHom || r == nullHom)
    return nullHom;
  
  if( thisH == GShom::id )
    return r;
  
  if( r == GShom::id )
    return thisH;
  
  return sns::Compose(thisH, r);
}

/*************************************************************************/
/*                         Class StrongShom                               */
/*************************************************************************/

/* Compare */
bool StrongShom::operator==(const _GShom &h) const{
  return typeid(*this)==typeid(h)?*this==*(StrongShom*)&h:false;
}




/* Eval */
GSDD 
StrongShom::eval(const GSDD &d) const
{
  if(d==GSDD::null)
    {
      return GSDD::null;
    }
  else if(d==GSDD::one)
    {
      return phiOne();
    }
  else if(d==GSDD::top)
    {
      return GSDD::top;
    }
  else
    {
      int variable=d.variable();
      d3::set<GSDD>::type s;

      for(GSDD::const_iterator vi=d.begin();vi!=d.end();++vi)
	{
	  s.insert(phi(variable,*vi->first) (vi->second) );
    	}
      return SDED::add(s);
    }
}
  
void StrongShom::print (std::ostream & os) const {
  os << "(StrongSHom)";
}



/*************************************************************************/
/*                         Class GShom                                    */
/*************************************************************************/
using sns::canonical;

// Note: Shom::null is defined in SDD.cpp for static initialization stupid C++ freaking semantics.
GShom::fixpointStrategy GShom::fixpointStrategy_ = BFS;
GShom::saturationStrategy GShom::saturationStrategy_ = ORDINARY;

/* Constructor */
GShom::GShom(const _GShom *h):concret(h){}

GShom::GShom(const _GShom &h):concret(canonical(h)){}

GShom::GShom(const MLShom &h):concret(canonical( sns::MLShomAdapter(h) )) {};

GShom::GShom(const GSDD& d):concret(canonical( sns::Constant(d))){}

GShom::GShom(int var,const DataSet & val, const GShom &h) {
  if ( ! val.empty() ) {
    concret=  canonical ( sns::LeftConcat(GSDD(var,val),h));
  } else {
    concret = _GShom::get_concret(Shom::null) ;
  }
}

//////////////////////////////////////////////////////////////////////////////

typedef Cache<GShom,GSDD,GSDD> ShomCache;

template<>
bool
ShomCache::should_insert (const GShom & h) const
{
  if (typeid(_GShom::get_concret (h)) == typeid(sns::Fixpoint))
  {
    return ! fobs::get_fixobserver ()->was_interrupted ();
  }
  return true;
}
namespace sns {
static ShomCache cache;
}
/* Eval */
GSDD 
GShom::operator()(const GSDD &d) const
{
  if(concret->immediat())
    {
      return eval(d);
    }
  else
    {
      if (d == GSDD::null)
	{
	  return d;
	}
      else
	{
	  return (sns::cache.insert(*this,d)).second;
	}

    }
}

GSDD 
GShom::eval(const GSDD &d) const
{
  return concret->eval_skip(d);
}

/// returns the predescessor homomorphism, using pot to determine variable domains
GShom GShom::invert (const GSDD & pot) const {
  return concret->invert(pot);
}

GShom GShom::compose(const GShom &o) const {
  return concret->compose(o);
}

int GShom::refCounter() const{return concret->refCounter();}

/* Sum */

// GShom GShom::add(const set<GShom>& s){
//    return(new Sum(s));
// }

bool GShom::has_image (const GSDD & d) const {
  

  /// test cache
  
  //  if (miss)
  return concret->has_image(d);
  
}


/* Memory Manager */
unsigned int GShom::statistics(){
  return canonical.size();
}

size_t GShom::cache_size() {
  return sns::cache.size();
}

size_t GShom::cache_peak() {
  return sns::cache.peak();
}

// Todo
void GShom::mark()const{
	concret->set_mark(true);
}

// used to reduce Shom::add creation complexity in recursive cases
typedef ext_hash_map<d3::set<GShom>::type,const _GShom*>::internal_hash_map  addCache_t;
static addCache_t addCache;
void GShom::garbage(){
  addCache.clear();
  sns::cache.clear();
  
  // mark phase
  for(UniqueTable<_GShom>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();++di){
      (*di)->mark_if_refd();
  }
  // sweep phase
  for(UniqueTable<_GShom>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();){
    if(!(*di)->is_marked()){
      UniqueTable<_GShom>::Table::iterator ci=di;
      di++;
      const _GShom *g=*ci;
      canonical.table.erase(ci);
      delete g;
    }
    else{
      (*di)->set_mark(false);
      di++;
    }
  }
}

/*************************************************************************/
/*                    Class Shom                                          */
/*************************************************************************/
/* Constructor */
Shom::Shom(const Shom &h):GShom(h.concret){
  concret->ref();
}

Shom::Shom(const GShom &h):GShom(h.concret){
  concret->ref();
}

Shom::Shom(const GSDD& d):GShom(d){
  concret->ref();
}

Shom::Shom(int var,const DataSet &  val, const GShom &h):GShom(var,val,h){
  concret->ref();
}

Shom::~Shom(){
  assert(concret->refCounter()>0);
  concret->deref();
}

/* Set */

Shom &Shom::operator=(const Shom &h){
  assert(concret->refCounter()>0);
  concret->deref();
  concret=h.concret;
  concret->ref();
  return *this;
}

Shom &Shom::operator=(const GShom &h){
  assert(concret->refCounter()>0);
  concret->deref();
  concret=h.concret;
  concret->ref();
  return *this;
}

/// This predicate is true if the homomorphism global behavior is only to prune some paths.
bool GShom::is_selector() const {
  return concret->is_selector();
}

bool GShom::skip_variable(int var) const {
  return concret->skip_variable(var);
}


const GShom::range_t GShom::full_range = GShom::range_t() ; 
const GHom::range_t GHom::full_range = GHom::range_t() ; 

const GShom::range_t  GShom::get_range() const {
 return concret->get_range();
}


// a test used in commutativity assesment
static bool notInRange (const GShom::range_t & h1r, const GShom & h2) {
  GShom::range_t h2r = h2.get_range();
  // ALL variables range
  if ( h2r.empty() )
    return false;
  
    // Test empty intersection relying on sorted property of sets.
  GShom::range_it h1it = h1r.begin();
  GShom::range_it h2it = h2r.begin();
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



/* Operations */
GShom fixpoint (const GShom &h, bool is_top_level) {
  if( typeid( *_GShom::get_concret(h) ) == typeid(sns::Fixpoint)
      || h == GShom::id  || h.is_selector() || h == GShom(GSDD::null))
    return h;
  

  
  // is it the fixpoint of an union ?
  if (const sns::Add * add = dynamic_cast<const sns::Add*> ( _GShom::get_concret(h) ) )
    {
      // Check if we have (sel & F + id) where sel is a selector and F is a sum
      if (add->parameters.size() == 2) {
	GShom other ;
	bool haveId = false;
	for ( sns::Add::parameters_it it = add->parameters.begin() ; it != add->parameters.end() ; ++it ) {
	  if ( *it == GShom::id )
	    haveId = true;
	  else
	    other = *it;
	}
	if (haveId) {
	  // This looks good, we have the form : fixpoint ( other + Id )
	  // Check if : other = sel & F
	  if (const sns::Compose * comp = dynamic_cast<const sns::Compose*> ( _GShom::get_concret(other) ) ) {
	    // hit : we have a composition
//	    trace << "Hit a composition! ";// comp->print(std::cerr) ; std::cerr << std::endl;
	    bool canApply = false;
	    bool isLeftSel = true;
	    const sns::Add * subadd = NULL;
	    GShom selector;

	    if ( comp->left.is_selector() ) {
	      if (const sns::Add * subadd2 = dynamic_cast<const sns::Add*> ( _GShom::get_concret(comp->right) ) ) {
		subadd = subadd2;
		selector = comp->left;
		isLeftSel = true;
		canApply = true;
	      }
	    } else if (comp->right.is_selector() ) {
	      if (const sns::Add * subadd2 = dynamic_cast<const sns::Add*> ( _GShom::get_concret(comp->left) ) ) {
		subadd = subadd2;
		selector = comp->right;
		isLeftSel = false;
		canApply = true;
	      }
	    }
	    if (canApply) 
	      {
		// This is it !! apply rewriting strategy
//		trace << "Hit matches second criterion sel & Add ! " << std::endl;
		GShom::range_t selr = selector.get_range();
		if (! selr.empty() ) {
		  // selector concerns a subset of variables, probably we can commute with at least some of the terms in subadd
		  d3::set<GShom>::type doC, notC;
		  for (sns::Add::parameters_it it =  subadd->parameters.begin() ; it != subadd->parameters.end() ; ++it ) {
		    if ( notInRange (selr, *it) ) {
		      // insert into commutative operations set
		      doC.insert(*it);
		    } else {
		      // insert into non commutative set
		      notC.insert(*it);
		    }
		  }
		  
		  if (! doC.empty() ) {
		    // Great ! successful application of the rule is possible
		    //		    trace << "Hit Full ! " << doC.size() << "/" << notC.size() << std::endl;
		    d3::set<GShom>::type finalU;
		    finalU.insert(GShom::id);
		    doC.insert(GShom::id);
		    if (isLeftSel ) {
		      finalU.insert( sns::Fixpoint( (selector &  GShom::add(notC))  + GShom::id) );
		      finalU.insert( selector & fixpoint ( GShom::add(doC) ) );
		    } else {
		      finalU.insert( sns::Fixpoint( (GShom::add(notC) & selector)  + GShom::id) );
		      finalU.insert( fixpoint ( GShom::add(doC) ) & selector );
		    }
		    return sns::Fixpoint( GShom::add(finalU) ) ;
		  }
		}
	      }
	  }
	}
      }
    }
  if (is_top_level) {
    if (const sns::LocalApply * lapp = dynamic_cast<const sns::LocalApply *>( _GShom::get_concret(h) ) ) {
      std::cerr << "fixpoint of localApply rewriting as localApply of fixpoint" << std::endl;
      return localApply( fixpoint(lapp->h, true), lapp->target );
    }
    else if (const sns::SLocalApply * lapp = dynamic_cast<const sns::SLocalApply *>( _GShom::get_concret(h) ) ) {
      std::cerr << "fixpoint of SlocalApply rewriting as SlocalApply of fixpoint" << std::endl;
      return localApply( fixpoint(lapp->h, true), lapp->target );
    }
  }
  
  return sns::Fixpoint(h,0, is_top_level);
}

GShom
localApply(const GHom & h, int target)
{
  if( h == GHom::id ) {
    return GShom::id;
  } else if ( h == GHom(DDD::null) ) {
    return Shom::null;
  }
	  
  return sns::LocalApply(h,target);
}


GShom
localApply(const GShom & h, int target)
{
  if( h == GShom::id ||  h == Shom::null )
    {
      return h;
    }
  return sns::SLocalApply(h,target);
}

static  void addParameter (const GShom & hh, 	std::map<int, GHom> & local_homs, std::map<int, GShom> & local_shoms, d3::set<GShom>::type& parameters , bool & have_id) {
  const _GShom * h = _GShom::get_concret(hh);    
  const std::type_info & t = typeid( *h );
  if( t == typeid(sns::Add) )
    {
      const sns::Add::parameters_t & local_param = ((const sns::Add*) h)->parameters;
      for (sns::Add::parameters_it it = local_param.begin() ; it != local_param.end() ; ++it ){
	addParameter( _GShom::get_concret(*it), local_homs,local_shoms ,parameters,have_id );
      }
    }
  else if( t == typeid(sns::LocalApply) )
    {
      const sns::LocalApply* local = (const sns::LocalApply*)(h);
      std::map<int, GHom>::iterator f = local_homs.find( local->target );
      
      if( f != local_homs.end() )
	{
	  f->second = f->second + local->h;
	}
      else
	{
	  local_homs.insert(std::make_pair(local->target,local->h));
	}
      
    }
  else if( t == typeid(sns::SLocalApply) )
    {
      const sns::SLocalApply* local = (const sns::SLocalApply*)(h);
      std::map<int, GShom>::iterator f = local_shoms.find( local->target );
      
      if( f != local_shoms.end() )
	{
	  f->second = f->second + local->h;
	}
      else
	{
	  local_shoms.insert(std::make_pair(local->target,local->h));
	}
      
    }
  else { 
    if( t == typeid(sns::Identity) )
      {
	have_id = true;
      }
    parameters.insert(hh);
  }
}



static void buildUnionParameters(d3::set<GShom>::type& p, d3::set<GShom>::type& parameters, bool & have_id) {
  std::map<int, GHom> local_homs;
  std::map<int, GShom> local_shoms;
  for( d3::set<GShom>::type::const_iterator it = p.begin(); it != p.end(); ++it)
    {
      addParameter( *it , local_homs, local_shoms,parameters,have_id);
    }
  
  for( 	std::map<int, GHom>::iterator it = local_homs.begin();
	it != local_homs.end();
	++it)
    {
      if ( have_id )
	{
	  it->second = it->second + GHom::id;
	}	    
      parameters.insert(localApply(it->second,it->first));
    }
  for( 	std::map<int, GShom>::iterator it = local_shoms.begin();
	it != local_shoms.end();
	++it)
    {
      if( have_id )
	{
	  const std::type_info & t = typeid( *  _GShom::get_concret(it->second) );
	  // avoid pushing id down if it was already done, i.e.
	  // unless it->second is of the form id + h1 + h2 + ...
	  if ( ! ( t == typeid(sns::Add) &&
		   ((sns::Add*)  _GShom::get_concret(it->second))->get_have_id() ) )
	    // push id down
	    it->second = it->second + GShom::id;
	  
	}
      parameters.insert(localApply(it->second,it->first));
    }
  
}


// addcache declaration is just above function garbageCollect
// static addCache_t addCache;
GShom GShom::add(const d3::set<GShom>::type& set)
{  
  if (set.empty() ) 
    return Shom::null;
  
  if( set.size() == 1 )
    return *(set.begin());
  else {
    d3::set<GShom>::type s = set;
    s.erase(Shom::null);
    if( s.size() == 1 )
      return *(s.begin());

    addCache_t::const_iterator acc = addCache.find(s);
    if (acc == addCache.end()) {
      // build the union up
      d3::set<GShom>::type parameters;
      bool have_id = false;
      buildUnionParameters(s,parameters, have_id);
      
      GShom added;
      if( parameters.size() == 1 )
	added = *(parameters.begin());
      else
        added = sns::Add(parameters, have_id);
      
      addCache[s] = added.concret;
      // acc->second = added.concret;
      return added;
    } else {
      return acc->second;
    }
  }
}



static bool commutative (const GShom & h1, const GShom & h2) {
  if ( h1.is_selector() && h2.is_selector() ) 
    return true;

  GShom::range_t h1r = h1.get_range();
  // ALL variables range
  if ( h1r.empty() )
    return false;

  return notInRange (h1r , h2);
}

// add an operand to a commutative composition of hom
static void addCompositionParameter (const GShom & h, sns::And::parameters_t & args) {
	// associativity : a && (b && c) = a && b && c
  if ( const sns::And * hAnd = dynamic_cast<const sns::And*> ( _GShom::get_concret(h) ) ) {
    // recursively add each parameter
     for (sns::And::parameters_it it = hAnd->parameters.begin() ; it != hAnd->parameters.end() ; ++it ) {
       addCompositionParameter (*it, args) ;
     }
  } else {
    // first test for possible nesting of locals
    if ( const sns::LocalApply* lh2 = dynamic_cast<const sns::LocalApply* > ( _GShom::get_concret(h) ) ) {
      // test for local that can be nested
      for (sns::And::parameters_t::iterator it = args.begin() ; it != args.end() ; ++it ) {
	if ( const sns::LocalApply* lh1 = dynamic_cast<const sns::LocalApply* > ( _GShom::get_concret(*it) ) ) {
	  if( lh1->target == lh2->target ) {
	    *it = ( localApply(  lh1->h & lh2->h, lh1->target ) );
	    return;
	  }	
	}
      }
    } else if ( const sns::SLocalApply* lh2 = dynamic_cast<const sns::SLocalApply* > ( _GShom::get_concret(h) ) ) {
      // test for local that can be nested
      for (sns::And::parameters_t::iterator it = args.begin() ; it != args.end() ; ++it ) {
	if ( const sns::SLocalApply* lh1 = dynamic_cast<const sns::SLocalApply* > ( _GShom::get_concret(*it) ) ) {
	  if( lh1->target == lh2->target ) {
	    *it = ( localApply(  lh1->h & lh2->h, lh1->target ) );
	    return;
	  }	
	}
      }
    }
    // Local nesting tests have failed, proceed to deeper commutativity tests

    // partition args into elements that commute with h or not
    sns::And::parameters_t argsC, argsNOTC;
    for (sns::And::parameters_it it = args.begin() ; it != args.end() ; ++it ) {
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
      GShom h1 = *argsNOTC.begin();
      args.push_back ( sns::Compose ( h1,  h ) );
    } else {
      args.push_back ( sns::Compose ( sns::And (argsNOTC), h) );    
    }
  }
}


GShom operator&(const GShom &h1,const GShom &h2){
	
  // Id is neutral to composition :forall h,  h & id = id & h = h 
  if( h1 == GShom::id )
    return h2;
  if( h2 == GShom::id )
    return h1;

  // Null is absorbing to composition : forall h,  h & 0 = 0 & h = 0
  if (h1 == Shom::null || h2 == Shom::null)
    return Shom::null;


//  return sns::Compose(h1,h2);

  GShom h = h1.compose(h2);
  if (typeid(*_GShom::get_concret(h)) != typeid(sns::Compose)) {
    return h;
  }
  // Test commutativity of h1 and h2
  sns::And::parameters_t args;
  addCompositionParameter (h1, args);
  addCompositionParameter (h2, args);
  if ( args.empty() ) {
    std::cerr << " WTF ?? (SHOM.cpp in operator&)" << std::endl;
    return Shom::null;
  } else if ( args.size() == 1 ) {
    return *args.begin();
  } else {
    return sns::And (args);
  }

}

GShom operator+(const GShom &h1,const GShom &h2){
  // if (h1 < h2) 
  //   return GShom(canonical( sns::Add(h1,h2)));
  // else
  //   return GShom(canonical( sns::Add(h2,h1)));

  d3::set<GShom>::type s;
  s.insert(h1);
  s.insert(h2);

  return GShom::add(s);

}

GShom operator*(const GSDD &d,const GShom &h){
  return sns::Mult(h,d);
}

GShom operator*(const GShom &h,const GSDD &d){
  return sns::Mult(h,d);
}

GShom operator^(const GSDD &d,const GShom &h){
  return sns::LeftConcat(d,h);
}

GShom operator^(const GShom &h,const GSDD &d){
  return sns::RightConcat(h,d);
}

GShom operator-(const GShom &h,const GSDD &d){
  return sns::Minus(h,d);
}

GShom operator-(const GShom &h1,const GShom &h2){
  return sns::HomMinus(h1,h2);
}


/// An IF-THEN-ELSE construct.
/// The behavior of the condition **must** be a selection, as indicated by its isSelector() flag.
/// PITFALL : Otherwise an assertion violation will be raised (with an explicit stderr message)
///
/// Semantics : ITE ( cond, iftrue, iffalse) (d) =  (iftrue & cond(d)) + (iffalse & !cond(d)) 
static void printCondError (const GShom & cond) {

  std::cerr << " but the homomorphism passed :" << std::endl;
  std::cerr << cond << std::endl ;
  std::cerr << "Does not have selector flag set to true. If your logic is correct, check that"
	    << " you implement : bool is_selector() const { return true; }\n"
	    << " in all selector inductive homomorphisms." << std::endl ;
}

GShom ITE (const GShom & cond, const GShom & iftrue, const GShom & iffalse) {
  if (! cond.is_selector() ) {
    std::cerr << "Creating a complement condition in an ITE construct : " << std::endl;
    printCondError(cond);
    assert(false);
  }  
  // let optimizations and rewritings do their job.
  return (iftrue & cond) + (iffalse & (!cond));
}

GShom operator! (const GShom & cond) {
  if (! cond.is_selector() ) {
    std::cerr << "Creating a complement condition with operator! :  ! cond" << std::endl;
    printCondError(cond);
    assert(false);
    return Shom::null;
  } else if (cond == GShom::id) {
    return Shom::null;
  } else if (cond == Shom::null ) {
    return GShom::id;
  } else if ( const sns::SLocalApply* lh1 = dynamic_cast<const sns::SLocalApply* > ( _GShom::get_concret(cond) ) )  {
    return localApply ( ! lh1->h , lh1->target ); 
  } else if ( const sns::LocalApply* lh1 = dynamic_cast<const sns::LocalApply* > ( _GShom::get_concret(cond) ) )  {
    return localApply ( ! lh1->h , lh1->target ); 
  }  else if ( const sns::SNotCond* lh1 = dynamic_cast<const sns::SNotCond* > ( _GShom::get_concret(cond) ) )  {
    return  lh1->cond_ ; 
  } else {
    return sns::SNotCond(cond);
  }
}

GShom apply2k (const GSDD & d) {
  if (d == GSDD::null || d == GSDD::top ) {
    return d;
  } else if (d == GSDD::one) {
    return GShom::id;
  }
  return  sns::SApply2k(d);
}

/// Extract the domain of a given variable.
/// Returns the union of the first nodes encountered that bear this variable, with 1 as signle successor.
/// In other words, an SDD of a single var, with an edge value that is the union of all Datasets in the reachable SDD.
GShom extractPotential (int var) {
  return  sns::SDomExtract (var);
}

GShom operator*(const GShom & h,const GShom & cond) {
  if (! cond.is_selector() ) {
    std::cerr << "Creating a * intersection between homomorphisms, but second argument is not a selector. " << std::endl;
    printCondError(cond);
    assert(false);
  }
  if ( h == Shom::null || cond == Shom::null )
    return Shom::null;
    
  // trivial case
  if ( h == cond )
    return h;
  return sns::Inter(h,cond);
}


void GShom::pstats(bool)
{
  std::cout << "*\nGSHom Stats : size unicity table = " <<  canonical.size() << std::endl;

  std::cout << "sizeof(_GShom):" << sizeof(_GShom) << std::endl; 
  std::cout << "sizeof(SIdentity):" << sizeof(sns::Identity) << std::endl; 
 
#ifdef HASH_STAT
  std::cout << std::endl << "GShom Unicity table stats :" << std::endl;
  print_hash_stats(canonical.get_hits(), canonical.get_misses(), canonical.get_bounces());
  
  std::cout << "GShom cache stats : " << std::endl;
  print_hash_stats(sns::cache.get_hits(), sns::cache.get_misses(), sns::cache.get_bounces());
#endif // HASH_STAT
  
  /*
  std::ostream & os = std::cout;
  int i = 0;
  for (UniqueTable<_GShom>::Table::const_iterator it= canonical.table.begin() ;
       it != canonical.table.end();
       ++it ){
    os << i++ << " : " ;
    (*it)->print(os);
    os << std::endl;
  }
*/
}

// pretty print
std::ostream & operator << (std::ostream & os, const GShom & h) {
  h.concret->print(os);
  return os;
}

// Implementation of setter Observer
void fixpointObserver(const IFixpointObserver& obs){
	sns::__fixpointObs = &obs;
}
