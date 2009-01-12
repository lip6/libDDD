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

#include "Hom.h"
#include "DDD.h"
#include "DED.h"
#include "SHom.h"
#include "SDD.h"
#include "SDED.h"
#include "UniqueTable.h"
#include "DataSet.h"
#include "MemoryManager.h"
#include "util/configuration.hh"
#include "util/hash_support.hh"
#include "Cache.hh"


#ifdef PARALLEL_DD
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/concurrent_vector.h>
#endif


namespace d3 { namespace util {
  template<>
  struct equal<_GShom*>{
    bool operator()(_GShom * _h1,_GShom * _h2){
      return (typeid(*_h1)==typeid(*_h2)?(*_h1)==(*_h2):false);
    }
  };
}}

static UniqueTable<_GShom> canonical;
/*************************************************************************/
/*                         Class _GShom                                   */
/*************************************************************************/

typedef std::map<GSDD,DataSet*> GSDD_DataSet_map;

namespace S_Homomorphism {

	typedef Cache<GShom,GSDD> ShomCache;

	static ShomCache cache;


/************************** Identity */
class Identity:public _GShom{
public:
  /* Constructor */
  Identity(int ref=0):_GShom(ref,true){}

  /* Compare */
  bool operator==(const _GShom&) const{return true;}
  size_t hash() const{return 17;}
  _GShom * clone () const {  return new Identity(*this); }

  bool
  skip_variable(int) const 
  {
      return true;
  }

  /* Eval */
  GSDD eval(const GSDD &d)const{return d;}

  void print (std::ostream & os) const {
    os << "SId";
  }
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

  void print (std::ostream & os) const {
    os << "(SConstant:" << value << ")";
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
    return left(d)*right;
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
     return GSDD::null;
   else
     return SDED::add(sum);
  }

  void mark() const {
    h.mark();
  }  
  
  size_t hash() const {
    return  h.hash() ^ target * 21727; 
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
       assert( typeid(*it->first) == typeid(const SDD&) );
       SDD v2 = h((const SDD &)*it->first);
       if( ! (v2 == GSDD::null) )
	 {
	   sum.insert(GSDD(d.variable(), v2, it->second));
	 }
     }
   if (sum.empty())
     return GSDD::null;
   else
     return SDED::add(sum);
  }

  void mark() const {
    h.mark();
  }  
  
  size_t hash() const {
    return  h.hash() ^ target * 2177; 
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

  /************************** Add */
/************************** Add */
class Add
    :
    public _GShom
{

// types
public:
	
	typedef
	struct
	{
		GShom F; 
	  d3::set<GShom>::type G;
		const _GShom* L;
		bool has_local;
		
	} partition;

  typedef hash_map<int,partition >::type partition_cache_type;

private:
        
    d3::set<GShom>::type parameters;
	mutable partition_cache_type partition_cache;
	bool have_id;

  void addParameter (const GShom & hh, 	std::map<int, GHom> & local_homs, std::map<int, GShom> & local_shoms) {
    const _GShom * h = get_concret(hh);    
    const std::type_info & t = typeid( *h );
    if( t == typeid(Add) )
      {
	const d3::set<GShom>::type& local_param = ((const Add*) h)->parameters;
	for (d3::set<GShom>::type::const_iterator it = local_param.begin() ; it != local_param.end() ; ++it ){
	  addParameter( get_concret(*it), local_homs,local_shoms  );
	}
      }
    else if( t == typeid(LocalApply) )
      {
	const LocalApply* local = (const LocalApply*)(h);
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
    else if( t == typeid(SLocalApply) )
      {
	const SLocalApply* local = (const SLocalApply*)(h);
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
      if( t == typeid(Identity) )
	{
	  have_id = true;
	}
      parameters.insert(hh);
    }
  }
    


public:
        

    Add(const d3::set<GShom>::type& p, int ref=0)
    	:
        _GShom(ref,true),
        parameters(),
		have_id(false)
    {
		std::map<int, GHom> local_homs;
		std::map<int, GShom> local_shoms;
	
        for( d3::set<GShom>::type::const_iterator it = p.begin(); it != p.end(); ++it)
        {
	  addParameter( *it , local_homs, local_shoms);
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
		if ( ! ( t == typeid(Add) &&
			 ((Add*)  _GShom::get_concret(it->second))->get_have_id() ) )
		  // push id down
		  it->second = it->second + GShom::id;
	      
	      }
	    parameters.insert(localApply(it->second,it->first));
	  }
	
    }



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
	    size_t res = 0;
	    for(d3::set<GShom>::type::const_iterator gi=parameters.begin();gi!=parameters.end();++gi)
	      res^=gi->hash();
	    return res;

    }

  _GShom * clone () const {  return new Add(*this); }

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
			
	    for(	d3::set<GShom>::type::const_iterator gi = parameters.begin();
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
		    part.G.insert(*gi);
		  }
	      }
	    part.F = GShom::add(F);
	    return part.G.empty() && !part.has_local;
	  }
	  // cache hit
	  return caccess->second.G.empty() && !caccess->second.has_local;
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
		
			for(d3::set<GShom>::type::const_iterator gi=parameters.begin();gi!=parameters.end();++gi)
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
	
			const d3::set<GShom>::type& G = part_it->second.G;

			for( 	d3::set<GShom>::type::const_iterator it = G.begin(); 
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
		for( d3::set<GShom>::type::const_iterator gi=parameters.begin();
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
    d3::set<GShom>::type::const_iterator gi=parameters.begin();
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
    return 13*left.hash()+7*right.hash();
  }
  _GShom * clone () const {  return new Compose(*this); }

	bool
    skip_variable(int var) const 
    {
        return get_concret(left)->skip_variable(var)
			and get_concret(right)->skip_variable(var);
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
  LeftConcat(const GSDD &l,const GShom &r,int ref=0):_GShom(ref,true),left(l),right(r){}
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
  RightConcat(const GShom &l,const GSDD &r,int ref=0):_GShom(ref,true),left(l),right(r){}
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

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }

  void print (std::ostream & os) const {
    os << "(SMinus:" << left << " - " << right << ")";
  }

};

/************************** Fixpoint */

class Fixpoint
	: public _GShom
{

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
    return 17*arg.hash();
  }
  _GShom * clone () const {  return new Fixpoint(*this); }

  bool
  skip_variable(int var) const
  {
      return get_concret(arg)->skip_variable(var);
  }


  /* Eval */
  GSDD 
  eval(const GSDD &d) const
  {
    if( d == GSDD::null )
      {
	return GSDD::null;
      }
    else if( d == GSDD::one or d == GSDD::top )
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
				
		do
		  {
		    d1 = d2;

		    d2 = F_part(d2);
		    d2 = L_part(d2);

		    for( 	d3::set<GShom>::type::const_iterator G_it = partition.G.begin();
				G_it != partition.G.end();
				++G_it) 
		      {

			// d2 = F_part(d2);
			// 						d2 = L_part(d2);

			// apply local part
			// d2 = L_part(d2);

			// saturate firings of each transition (for non deterministic : one to many transitions).
			// do an internal fixpoint on every g \in G, i.e. 
			// (\sum_i (g_i + id)\star) \star
			GSDD d3 = d2;
			do {
			  d2 = d3;
			  d3 =  ( (L_part &  (*G_it))(d2)) + d2;
			} while (d3 != d2);

			// was before :
			// chain application of Shom of this level
			// d2 =  ( (L_part &  (*G_it))(d2)) + d2;
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
    os << "(SFix:" << arg << " *)";
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
            // S_Homomorphism::cache.insert( gshom_, val[to_solve[i]], result);
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
      if (d.nbsons() > 1  && (typeid(*this) == typeid(const S_Homomorphism::Fixpoint))) 
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
          if (immediat) {
      // right concatenating a constant ? ah : Can this happen ?
      // if this assert is raised, remove it !
              son_result.push_back(eval(it->second));
              assert(false);
          } else {
	    //  std::pair<bool,GSDD> local_res = S_Homomorphism::cache.contains(gshom,it->second);
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
      //     // S_Homomorphism::cache.insert( gshom, arg, result);
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


// constant Id
const GShom GShom::id(S_Homomorphism::Identity(1));

/* Constructor */
GShom::GShom(const _GShom *h):concret(h){}

GShom::GShom(const _GShom &h):concret(canonical(h)){}

GShom::GShom(const GSDD& d):concret(canonical( S_Homomorphism::Constant(d))){}

GShom::GShom(int var,const DataSet & val, const GShom &h) {
  if ( ! val.empty() ) {
    concret=  canonical ( S_Homomorphism::LeftConcat(GSDD(var,val),h));
  } else {
    concret = canonical( S_Homomorphism::Constant(GSDD::null));
  }
}

//////////////////////////////////////////////////////////////////////////////

/* Eval */
GSDD 
GShom::operator()(const GSDD &d) const
{
	if(concret->immediat)
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
            return (S_Homomorphism::cache.insert(*this,d)).second;
		}

	}
}

GSDD 
GShom::eval(const GSDD &d) const
{
	return concret->eval_skip(d);
}



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
}

// used to reduce Shom::add creation complexity in recursive cases
static hash_map<d3::set<GShom>::type,GShom>::type addCache;
void GShom::garbage(){
  addCache.clear();
  S_Homomorphism::cache.clear();

  // mark phase
  for(UniqueTable<_GShom>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();++di){
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
      const _GShom *g=*ci;
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
  if( typeid( *_GShom::get_concret(h) ) == typeid(S_Homomorphism::Fixpoint)
      || typeid( *_GShom::get_concret(h) ) == typeid(S_Homomorphism::Identity)
      )
      return h;
  return S_Homomorphism::Fixpoint(h);
}

GShom
// localApply(int target,const GHom & h)
localApply(const GHom & h, int target)
{
	if( h == GHom::id )
	{
		return GShom::id;
	}
	return S_Homomorphism::LocalApply(h,target);
}

GShom
// localApply(int target,const GHom & h)
localApply(const GShom & h, int target)
{
	if( h == GShom::id )
	{
	  return GShom::id;
	}
	return S_Homomorphism::SLocalApply(h,target);
}

// addcache declaration is just above function garbageCollect
// static hash_map<d3::set<GShom>::type,GShom>::type addCache;
GShom GShom::add(const d3::set<GShom>::type& set)
{  
  if (set.empty() ) 
    return GSDD::null;
  
  if( set.size() == 1 )
    return *(set.begin());
  else {
    d3::set<GShom>::type s = set;
    s.erase(GShom(GSDD::null));
    if( s.size() == 1 )
      return *(s.begin());
    hash_map<d3::set<GShom>::type,GShom>::type::accessor acc;
    if (addCache.insert(acc,s)) {
      GShom added = canonical(S_Homomorphism::Add(s));
      acc->second = added;
      return added;
    } else {
      return acc->second;
    }
  }
}

GShom operator&(const GShom &h1,const GShom &h2){
	
	if( h1 == GShom::id )
		return h2;

	if( h2 == GShom::id )
		return h1;

	if( typeid( *_GShom::get_concret(h1) ) == typeid(S_Homomorphism::LocalApply) 
		&& typeid( *_GShom::get_concret(h2) ) == typeid(S_Homomorphism::LocalApply) )
	{
	  const S_Homomorphism::LocalApply* lh1 = (const S_Homomorphism::LocalApply*)(_GShom::get_concret(h1));
	  const S_Homomorphism::LocalApply* lh2 = (const S_Homomorphism::LocalApply*)(_GShom::get_concret(h2));

		if( lh1->target == lh2->target )
		{
			return localApply(  lh1->h & lh2->h, lh1->target );
		}
	}

	if( typeid( *_GShom::get_concret(h1) ) == typeid(S_Homomorphism::SLocalApply) 
		&& typeid( *_GShom::get_concret(h2) ) == typeid(S_Homomorphism::SLocalApply) )
	{
	  const S_Homomorphism::SLocalApply* lh1 = (const S_Homomorphism::SLocalApply*)(_GShom::get_concret(h1));
	  const S_Homomorphism::SLocalApply* lh2 = (const S_Homomorphism::SLocalApply*)(_GShom::get_concret(h2));

		if( lh1->target == lh2->target )
		{
			return localApply(  lh1->h & lh2->h, lh1->target );
		}
	}
	
  	return S_Homomorphism::Compose(h1,h2);
}

GShom operator+(const GShom &h1,const GShom &h2){
  // if (h1 < h2) 
  //   return GShom(canonical( S_Homomorphism::Add(h1,h2)));
  // else
 //   return GShom(canonical( S_Homomorphism::Add(h2,h1)));

  d3::set<GShom>::type s;
  s.insert(h1);
  s.insert(h2);

  return GShom::add(s);

}

GShom operator*(const GSDD &d,const GShom &h){
  return S_Homomorphism::Mult(h,d);
}

GShom operator*(const GShom &h,const GSDD &d){
  return S_Homomorphism::Mult(h,d);
}

GShom operator^(const GSDD &d,const GShom &h){
  return S_Homomorphism::LeftConcat(d,h);
}

GShom operator^(const GShom &h,const GSDD &d){
  return S_Homomorphism::RightConcat(h,d);
}

GShom operator-(const GShom &h,const GSDD &d){
  return S_Homomorphism::Minus(h,d);
}


void GShom::pstats(bool)
{
  std::cout << "*\nGSHom Stats : size unicity table = " <<  canonical.size() << std::endl;
  std::ostream & os = std::cout;
  int i = 0;
  for (UniqueTable<_GShom>::Table::const_iterator it= canonical.table.begin() ;
       it != canonical.table.end();
       ++it ){
    os << i++ << " : " ;
    (*it)->print(os);
    os << std::endl;
  }
}

// pretty print
std::ostream & operator << (std::ostream & os, const GShom & h) {
  h.concret->print(os);
  return os;
}
