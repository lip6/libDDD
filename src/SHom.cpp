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
# include <tbb/blocked_range.h>
# ifdef PARALLEL_REDUCE
#  include <tbb/parallel_reduce.h>
# else
#  include <tbb/parallel_for.h>
#  include <tbb/concurrent_vector.h>
# endif
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

namespace S_Homomorphism {

	typedef Cache<GShom,GSDD> ShomCache;

	static ShomCache cache;


/************************** Identity */
class Identity:public _GShom{
public:
  /* Constructor */
  Identity(int ref=0):_GShom(ref,true){}

  /* Compare */
  bool operator==(const _GShom &h) const{return true;}
  size_t hash() const{return 17;}

  bool
  skip_variable(int) const 
  {
      return true;
  }

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
    return value.hash();
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
    return 83*left.hash()+53*right.hash();
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


/*************************************************************************/
/*                         Class LocalApply                              */
/*************************************************************************/

class LocalApply
	:
	public StrongShom
{

public:

  GHom h;
  int target;

	LocalApply()
		:
		h(), target(0)
	{}

  LocalApply (const GHom& hh,int t) :h(hh),target(t) {}

  GSDD phiOne() const {
	  // target not encountered !
    // consider this is not a fault
    return GSDD::one;
  }     

  // optimize away needless exploration of upstream modules that dont contain the place
  bool skip_variable (int var) const {
	  return var != target;
  }
  
  GShom phi(int vr, const DataSet & vl) const {

    assert( typeid(vl) == typeid(const DDD&) );
    
    if( vr != target )
      {
	return GShom(vr,vl,this);
      }
    else
      {
	DDD v2 = h((const DDD &)vl);
	return GShom(vr,v2);
      }

  }

  void mark() const {
    h.mark();
  }  
  
  size_t hash() const {
    return  h.hash() ^ target * 21727; 
  }

  bool operator==(const StrongShom &s) const {
    const LocalApply* ps = (const LocalApply *)&s;
    return target == ps->target && h ==  ps->h;
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
		std::set<GShom> G;
		const LocalApply* L;
		bool has_local;
		
	} partition;

    typedef std::map<int,partition> partition_cache_type;
    

private:
        
    std::set<GShom> parameters;
	mutable partition_cache_type partition_cache;
	bool have_id;

  void addParameter (const _GShom * h, 	std::map<int, GHom> & local_homs) {
    const std::type_info & t = typeid( *h );
    if( t == typeid(Add) )
      {
	const std::set<GShom>& local_param = ((const Add*) h)->parameters;
	for (std::set<GShom>::const_iterator it = local_param.begin() ; it != local_param.end() ; ++it ){
	  addParameter( get_concret(*it), local_homs  );
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
    else { 
      if( t == typeid(Identity) )
	{
	  have_id = true;
	}
      parameters.insert(h);
    }
  }
    


public:
        

    Add(const std::set<GShom>& p, int ref=0)
    	:
        _GShom(ref,true),
        parameters(),
		have_id(false)
    {
		std::map<int, GHom> local_homs;
	
        for( std::set<GShom>::const_iterator it = p.begin(); it != p.end(); ++it)
        {
	  addParameter( get_concret(*it) , local_homs);
        }
	
	for( 	std::map<int, GHom>::iterator it = local_homs.begin();
		it != local_homs.end();
		++it)
	  {
	    if( have_id )
	      {
		it->second = it->second + GHom::id;
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
	    for(std::set<GShom>::const_iterator gi=parameters.begin();gi!=parameters.end();++gi)
	      res^=gi->hash();
	    return res;

    }

	bool
	skip_variable( int var ) const
	{
		partition_cache_type::iterator part_it = partition_cache.find(var);
		if( part_it == partition_cache.end() )
		{
			part_it = partition_cache.insert(std::make_pair(var,partition())).first;
			partition& part = part_it->second;
			part.has_local = false;
			part.L  = NULL;
			std::set<GShom> F;
			
			for(	std::set<GShom>::const_iterator gi = parameters.begin();
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
			  else
			    {
			      // G part
			      part.G.insert(*gi);
			    }
			}
			part.F = GShom::add(F);
		}

		return part_it->second.G.empty() && !part_it->second.has_local;
	}

    partition
    get_partition(int var)
    {
        this->skip_variable(var);
        return partition_cache.find(var)->second;
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
			std::set<GSDD> s;
		
			for(std::set<GShom>::const_iterator gi=parameters.begin();gi!=parameters.end();++gi)
			{
				s.insert((*gi)(d));
			}
			return SDED::add(s);
		}
		else
		{
			std::set<GSDD> s;
			int var = d.variable();

			partition_cache_type::iterator part_it = partition_cache.find(var);
			if( part_it == partition_cache.end() )
			{
				this->skip_variable(var);
				part_it = partition_cache.find(var);
			}

			if( part_it->second.L != NULL )
			{
				s.insert(GShom(part_it->second.L)(d));
			}

			s.insert( part_it->second.F(d) );
	
			std::set<GShom>& G = part_it->second.G;

			for( 	std::set<GShom>::const_iterator it = G.begin(); 
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
		for( std::set<GShom>::const_iterator gi=parameters.begin();
			 gi!=parameters.end();
			 ++gi)
		{
			gi->mark();
		}

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
    return 47*left.hash()+19*right.hash();
  }

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
		if( typeid( *get_concret(arg) ) == typeid(Add) )
		{
			// Check if we have ( Id + F + G )* where F can be forwarded to the next variable
		
			// Rewrite ( Id + F + G )*
			// into ( O(Gn + Id) o (O(Fn + Id)*)* )* 
			Add* add = ((Add*)get_concret(arg));
			if( add->get_have_id() )
			{                           
				Add::partition partition = add->get_partition(variable);

				// operations that can be forwarded to the next variable
				GShom F_part = fixpoint(partition.F);

				GShom L_part = partition.has_local
							? localApply(fixpoint(GHom(partition.L->h)),variable)
							: GShom::id
							;
				
				do
				{
					d1 = d2;

					d2 = F_part(d2);
					d2 = L_part(d2);

					for( 	std::set<GShom>::const_iterator G_it = partition.G.begin();
							G_it != partition.G.end();
							++G_it) 
					{

						// d2 = F_part(d2);
						d2 = L_part(d2);

						// apply local part
						// d2 = L_part(d2);
					  // chain application of Shom of this level
					  d2 = (*G_it)(d2) + d2;
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
};




} // end namespace H_Homomorphism

typedef std::map<GSDD,DataSet*> GSDD_DataSet_map;

#ifdef PARALLEL_DD

// typedef tbb::blocked_range<GSDD::const_iterator> varval_range;
typedef tbb::blocked_range<int> varval_range;

# ifdef PARALLEL_REDUCE

class hom_reducer
{
private:
	
	const GShom& gshom_;
	const GSDD& gsdd_;
	GSDD_DataSet_map res_;
	
public:

	// standard constructor
	hom_reducer( const GShom& gshom
			   , const GSDD& gsdd
			   )
			
			: gshom_(gshom)
			, gsdd_(gsdd)
			, res_()
	{
	}


	// "splitting constructor", called by tbb internally
	hom_reducer(const hom_reducer& reducer,
				tbb::split)

		: gshom_(reducer.gshom_)
		, gsdd_(reducer.gsdd_)
		, res_()
	{
	}

	// what to do when reducing this and hom_apply
	// so here, we make a square_union
	void
	join(const hom_reducer& reducer)
	{
		const GSDD_DataSet_map& to_reduce = reducer.res_;
		for( GSDD_DataSet_map::const_iterator it = to_reduce.begin() 
		   ; it != to_reduce.end()
		   ; ++it )
		{
			square_union(res_, it->first, it->second);
		}
	}
		
	void
	operator()(const varval_range& range)
	{
		GSDD_DataSet_map& res = this->res_;
		
		for( GSDD::const_iterator it = range.begin();
			 it != range.end();
			 ++it)
		{
			GSDD son = gshom_(it->second);
            if( son != GSDD::null && !(it->first->empty()) )
            {
				square_union(res, son, it->first);
            }
		}
	}
	
	const GSDD_DataSet_map&
	get_results() 
	const
	{
		return res_;
	}
	
	
};

# else // NOT PARALLEL_REDUCE

typedef tbb::concurrent_vector<std::pair<DataSet *,GSDD> > concurrent_valuation;

class hom_for
{
private:

	const GShom& gshom_;
	// GSDD::Valuation& gsdd_;
        std::vector<GSDD> & val_;
	// concurrent_valuation& res_;
	int* to_solve_;

public:

	hom_for( const GShom& ghsom
		   // 	       , const GSDD& gsdd
		   // , concurrent_valuation& res 
			// , GSDD::Valuation& gsdd
		        , std::vector<GSDD>& val
			, int* to_solve
	   		)
		
		: gshom_(ghsom)
		// , gsdd_(gsdd)
		, val_(val)
		// , res_(res)
		, to_solve_(to_solve)
	{
	}
	
	void operator()(const varval_range& range)
	const
	{
		// concurrent_valuation& res = this->res_;
		
		
		// for( GSDD::const_iterator it = range.begin();
		// 	 it != range.end();
		// 	 ++it)
		// {
		// 	res.push_back( std::make_pair( it->first
		// 								  , gshom_(it->second) ));
		// }	
	        std::vector<GSDD>& val = this->val_;
		int* to_solve = this->to_solve_;
		
		for( int i = range.begin(); i != range.end(); ++i)
		{
			// gshom_.eval skip cache lookup
			
 			GSDD result = gshom_.eval( val[to_solve[i]]) ;
 			S_Homomorphism::cache.insert( gshom_, val[to_solve[i]], result);
 			val[to_solve[i]] = result;

//			val[to_solve[i]] = gshom_(val[to_solve[i]]) ;
		}

	}

};

# endif // PARALLEL_REDUCE

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
      // for square union
      GSDD_DataSet_map res;
      
#ifdef PARALLEL_DD
      
// # ifdef PARALLEL_REDUCE
      
//       hom_reducer reducer(gshom, d); 
      
//       tbb::parallel_reduce( varval_range(d.begin(),d.end(), 2) 
// 			    , reducer);
      
//       const GSDD_DataSet_map& res = reducer.get_results();      
      
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
      for( GSDD::const_iterator it = d.begin();
	   it != d.end() ;
	   ++it) {
	if (immediat) {
	  // right concatenating a constant ? ah : Can this happen ?
	  // if this assert is raised, remove it !
	  son_result.push_back(eval(it->second));
	  assert(false);
	} else {
	  std::pair<bool,GSDD> res = S_Homomorphism::cache.contains(gshom,d);
	  if (res.first) {
	    // cache hit
	    son_result.push_back(res.second);
	  } else {
	    // cache miss : add index i to tosolve list
	    to_solve[to_solve_size++] = i;
	    // set current value in son_result to son
	    son_result.push_back(it->second);
	  }
	}
      }
      
      std::cout 
	<< "Valuation size " << d.nbsons()
	<< " To solve size " << to_solve_size
	<< std::endl;
      
      // filter pathological single son case
      if (to_solve_size > 1) {
	// the actual parrallel computation
	//          for i in range given by varval_range : 0 < i < tosolvesize
	// third parameter in range constructor is grain of parallelism : 1 => 1 task per arc created
	tbb::parallel_for( varval_range( 0, to_solve_size, 1)
			   // for task body computes  : sonresult[tosolve[i] = gshom(sonresult[tosolve[i]]) 
			   , hom_for(gshom, son_result, to_solve));
      } else if (to_solve_size == 1) {
	GSDD result = gshom.eval( son_result[to_solve[0]]) ;
	S_Homomorphism::cache.insert( gshom, son_result[to_solve[0]], result);
	son_result[to_solve[0]] = result;
      }

      i=0;
      for( GSDD::const_iterator it = d.begin()
	     ; it != d.end()
	     ; ++it,++i )
	{
	  // arcs to null are pruned
	  if ( son_result[i] != GSDD::null && !(it->first->empty()))
	    {
	      // use arc value from node d and new son from son_result
	      // note that arc values are copied into res, so d is const always
	      square_union( res, son_result[i] , it->first);
	    }
	}
      
      
#else // NOT PARALLEL_DD      

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
      
#endif // PARALLEL_DD
      
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
    	std::set<GSDD> s;

    	for(GSDD::const_iterator vi=d.begin();vi!=d.end();++vi)
		{
      		s.insert(phi(variable,*vi->first) (vi->second) );
    	}
    	return SDED::add(s);
  	}
}




/*************************************************************************/
/*                         Class GShom                                    */
/*************************************************************************/

/* Constructor */
GShom::GShom(const _GShom *h):concret(h){}

GShom::GShom(_GShom *h):concret(canonical(h)){}

GShom::GShom(const GSDD& d):concret(canonical(new S_Homomorphism::Constant(d))){}

GShom::GShom(int var,const DataSet & val, const GShom &h) {
  if ( ! val.empty() ) {
    concret=  canonical(new S_Homomorphism::LeftConcat(GSDD(var,val),h));
  } else {
    concret = canonical(new S_Homomorphism::Constant(GSDD::null));
  }
}

//////////////////////////////////////////////////////////////////////////////

/* Eval */
GSDD 
GShom::operator()(const GSDD &d) const
{
	if(concret->immediat)
	{
		return concret->eval(d);
	}
	else
    {
	//      return SDED::Shom(*this,d);
		if (d == GSDD::null) {
			return d;
		} else {
			std::pair<bool,GSDD> res = S_Homomorphism::cache.contains(*this,d);
			if (res.first) {
		// cache hit
				return res.second;
			} else {
				GSDD result = eval(d);
				S_Homomorphism::cache.insert (*this,d,result);
				return result;
			}
		}

	}
}

GSDD 
GShom::eval(const GSDD &d) const
{
	return concret->eval_skip(d);
}

const GShom GShom::id(canonical(new S_Homomorphism::Identity(1)));

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
  return GShom(canonical(new S_Homomorphism::Fixpoint(h)));
}

GShom
// localApply(int target,const GHom & h)
localApply(const GHom & h, int target)
{
	if( h == GHom::id )
	{
		return GShom::id;
	}
	return new S_Homomorphism::LocalApply(h,target);
}

GShom GShom::add(const std::set<GShom>& s)
{
  if (s.empty() ) 
    return GSDD::null;
	if( s.size() == 1 )
		return *(s.begin());
	else
  		return GShom(canonical(new S_Homomorphism::Add(s)));   
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
	
  	return GShom(canonical(new S_Homomorphism::Compose(h1,h2)));
}

GShom operator+(const GShom &h1,const GShom &h2){
  // if (h1 < h2) 
  //   return GShom(canonical(new S_Homomorphism::Add(h1,h2)));
  // else
 //   return GShom(canonical(new S_Homomorphism::Add(h2,h1)));

  std::set<GShom> s;
  s.insert(h1);
  s.insert(h2);

  return GShom(canonical(new S_Homomorphism::Add(s)));

}

GShom operator*(const GSDD &d,const GShom &h){
  return GShom(canonical(new S_Homomorphism::Mult(h,d)));
}

GShom operator*(const GShom &h,const GSDD &d){
  return GShom(canonical(new S_Homomorphism::Mult(h,d)));
}

GShom operator^(const GSDD &d,const GShom &h){
  return GShom(canonical(new S_Homomorphism::LeftConcat(d,h)));
}

GShom operator^(const GShom &h,const GSDD &d){
  return GShom(canonical(new S_Homomorphism::RightConcat(h,d)));
}

GShom operator-(const GShom &h,const GSDD &d){
  return GShom(canonical(new S_Homomorphism::Minus(h,d)));
}


void GShom::pstats(bool reinit)
{
  std::cout << "*\nGSHom Stats : size unicity table = " <<  canonical.size() << std::endl;
}
