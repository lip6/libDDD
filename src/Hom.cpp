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

#include "Hom.h"
#include "DDD.h"
#include "DED.h"
#include "UniqueTable.h"

#ifdef PARALLEL_DD
#include <tbb/concurrent_vector.h>
#include <tbb/parallel_reduce.h>
#endif

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
    skip_variable(int) const 
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
                if( typeid(*get_concret(*it)) == typeid(Identity) )
                {
                    have_id = true;
                }
                parameters.insert(*it);
            }
        }
    }

    bool
    get_have_id() const
    {
        return have_id;
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
    
    bool
    skip_variable(int var) const
    {
        return get_concret(left)->skip_variable(var) and get_concret(right)->skip_variable(var);
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
    
    bool
    skip_variable(int var) const
    {
        return get_concret(arg)->skip_variable(var);
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
};

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
    if( d == GDDD::one )
    {
        return phiOne();
    }
    
    int variable=d.variable();
    std::set<GDDD> s;
    for(GDDD::const_iterator vi=d.begin();vi!=d.end();++vi)
    {
        s.insert(phi(variable,vi->first)(vi->second));
    }
    return DED::add(s);
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
    {
        return concret->eval(d);
    }
    else
    {
        return DED::hom(*this,d);
    }
}

GDDD GHom::eval(const GDDD &d) const{
  return concret->eval_skip(d);
}

const GHom GHom::id(canonical(new Identity(1)));

int GHom::refCounter() const{return concret->refCounter;}

/* Sum */

GHom GHom::add(const std::set<GHom>& s){
    if( s.empty() )
        return GDDD::null;
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
	
	if( h1 == GHom::id )
		return h2;

	if( h2 == GHom::id )
		return h1;

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
