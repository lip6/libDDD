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

#ifndef __MLHOM__H__
#define __MLHOM__H__

#include "ddd/Hom.h"
#include "ddd/AdditiveMap.hpp"

class MLHom;

typedef AdditiveMap<GHom,GDDD> HomNodeMap ;

typedef  AdditiveMap<GHom,MLHom> HomHomMap; 


class _MLHom;
class StrongMLHom;

class MLHom {

   /// By definition, as homomorphism are linear, (h+g) (d) = h(d) + g(d) ;
  /// Where g,h are homomorphisms and d is a DDD.
  friend MLHom operator+(const MLHom &,const MLHom &);
  /// The real implementation class. All true operations are delagated on this pointer.
  /// Construction/destruction take care of ensuring concret is only instantiated once in memory.  
  const _MLHom* concret;

#ifdef HASH_STAT
  // open access to instrumented hashtable
  template <class Value, class Key, class HashFcn,
  class ExtractKey, class SetKey, class EqualKey, class Alloc>
  friend class google::sparse_hashtable;
#endif
  
public :

  /// Elementary homomorphism Identity, defined as a constant.
  /// id(d) = <id, d>
  static const MLHom id;

  /// Default public constructor.
  /// Builds Identity homomorphism : forall d in DDD, id(d) = d
  MLHom():concret(id.concret){};
  
  MLHom(const GHom &h);
  MLHom (const GHom & up, const MLHom & down);
  MLHom (const _MLHom &);
  MLHom (_MLHom *);
  MLHom (const _MLHom *);
  
  
  /// Create variable/value pair and left concatenate to a homomorphism.
  /// h(var,val,g) (d) = DDD(var,val) ^ g(d).
  /// In other words : var -- val -> g
  /// \param var the variable index
  /// \param val the value associated to the variable
  /// \param h the homomorphism to apply on successor node. Default is identity, so is equivalent to a left concatenation of a DDD(var,val).
  MLHom(int var, int val, const MLHom &h=MLHom::id);  

  virtual ~MLHom();

  
  bool operator<(const MLHom &h) const {return concret<h.concret;};
  bool operator==(const MLHom &h) const {return concret==h.concret;};
  /// Hash key computation. It is essential for good hash table operation that the spread
  /// of the keys be as good as possible. Also, fast hash key computation is a good design goal.
  /// Note that bad hash functions will yield more collisions, thus equality comparisons which
  /// may be quite costly.
  size_t hash() const { return ddd::knuth32_hash(reinterpret_cast<size_t>(concret)); };
  
  /// The computation function responsible for evaluation over a node.
  /// Users should not directly use this. Normal behavior is to use operator()
  /// that encapsulates this call with operation caching.
    HomNodeMap eval(const GDDD &d) const ;
  /// cache calls to eval
  HomNodeMap operator() (const GDDD &) const;

  
  /// Collects and destroys unused homomorphisms. Do not call this directly but through 
  /// MemoryManager::garbage() as order of calls (among GSDD::garbage(), GShom::garbage(), 
  /// SDED::garbage()) is important.
  static void garbage();

};

/// Composition by union of two homomorphisms. 
/// This commutative operation computes a homomorphism 
/// that evaluates as the sum of two homomorphism.
///
/// Semantics : (h1 + h2) (d) = h1(d) + h2(d).
MLHom operator+(const MLHom &,const MLHom &); 

class _MLHom {
  /// For garbage collection. 
  /// Counts the number of times a _MLHom is referenced from the context of an MLHom.
  mutable int refCounter;
  /// For garbage collection. Used in the two phase garbage collection process.
  /// A Shom that is not marked after the first pass over the unicity table, will
  /// be sweeped in the second phase. Outside of garbage collection routine, marking
  /// should always bear the value false.
  mutable bool marking;

  /// open access to container class MLHom.
  friend class MLHom;

  /// For garbage collection. Used in first phase of garbage collection.
  virtual void mark() const{};

  
public:
  _MLHom (int ref=0) : refCounter(ref),marking(false) {}
  /** test if caching should be done : default means should cache */
  virtual bool shouldCache () const { return true ; }

  /// Virtual Destructor. 
  virtual ~_MLHom(){};
  virtual HomNodeMap eval(const GDDD &) const = 0;

  /** unique table trivia */
  virtual size_t hash() const = 0;
  virtual bool operator==(const _MLHom &h) const=0;
  // for use by unique table : return new MyConcreteClassName(*this);
  virtual _MLHom * clone () const =0 ;
  
};

class StrongMLHom : public _MLHom {
public :

  bool operator==(const _MLHom &h) const;

  virtual bool operator==(const StrongMLHom &) const=0;

  HomNodeMap eval(const GDDD &) const ;

  /// User defined behavior is input through this function
  virtual HomHomMap phi (int var,int val) const=0;   
  virtual HomNodeMap phiOne () const=0;   

};

#endif
