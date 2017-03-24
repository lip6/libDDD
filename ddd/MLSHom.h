#ifndef __MLSHOM__H__
#define __MLSHOM__H__

#include "ddd/AdditiveMap.hpp"
#include "ddd/SHom.h"

class MLShom;

typedef AdditiveMap<GShom, GSDD> SHomNodeMap;
typedef AdditiveMap<GShom, MLShom> SHomHomMap;

class _MLShom;

class MLShom {
  
  /// By definition, as homomorphism are linear, (h+g) (d) = h(d) + g(d) ;
  /// Where g,h are homomorphisms and d is a SDDD.
  friend MLShom operator+(const MLShom &,const MLShom &);
  /// The real implementation class. All true operations are delagated on this pointer.
  /// Construction/destruction take care of ensuring concret is only instantiated once in memory.  
  const _MLShom* concret;
  
  public :
  
  /// Elementary homomorphism Identity, defined as a constant.
  /// id(d) = <id, d>
  static const MLShom id;
  
  /// Default public constructor.
  /// Builds Identity homomorphism : forall d in DDD, id(d) = d
  MLShom():concret(id.concret){};
  
  MLShom(const GShom &h);
  MLShom (const GShom & up, const MLShom & down);
  MLShom (const _MLShom &);
  MLShom (_MLShom *);
  MLShom (const _MLShom *);
  
  
  /// Create variable/value pair and left concatenate to a homomorphism.
  /// h(var,val,g) (d) = DDD(var,val) ^ g(d).
  /// In other words : var -- val -> g
  /// \param var the variable index
  /// \param val the value associated to the variable
  /// \param h the homomorphism to apply on successor node. Default is identity, so is equivalent to a left concatenation of a DDD(var,val).
  MLShom(int var, const DataSet & val, const MLShom &h=MLShom::id);  
  
  virtual ~MLShom();
  
  
  bool operator<(const MLShom &h) const {return concret<h.concret;};
  bool operator==(const MLShom &h) const {return concret==h.concret;};
  /// Hash key computation. It is essential for good hash table operation that the spread
  /// of the keys be as good as possible. Also, fast hash key computation is a good design goal.
  /// Note that bad hash functions will yield more collisions, thus equality comparisons which
  /// may be quite costly.
  size_t hash() const { return ddd::knuth32_hash(reinterpret_cast<const size_t>(concret)); };
  
  /// The computation function responsible for evaluation over a node.
  /// Users should not directly use this. Normal behavior is to use operator()
  /// that encapsulates this call with operation caching.
  SHomNodeMap eval(const GSDD &) const ;
  /// cache calls to eval
  SHomNodeMap operator() (const GSDD &) const;
  
  
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
MLShom operator+(const MLShom &,const MLShom &); 

class _MLShom {
  /// For garbage collection. 
  /// Counts the number of times a _MLHom is referenced from the context of an MLHom.
  mutable int refCounter;
  /// For garbage collection. Used in the two phase garbage collection process.
  /// A Shom that is not marked after the first pass over the unicity table, will
  /// be sweeped in the second phase. Outside of garbage collection routine, marking
  /// should always bear the value false.
  mutable bool marking;
  
  /// open access to container class MLHom.
  friend class MLShom;
  
  /// For garbage collection. Used in first phase of garbage collection.
  virtual void mark() const{};
  
  
public:
  _MLShom (int ref=0) : refCounter(ref),marking(false) {}
  /** test if caching should be done : default means should cache */
  virtual bool shouldCache () const { return true ; }
  
  /// Virtual Destructor. 
  virtual ~_MLShom(){};
  virtual SHomNodeMap eval(const GSDD &) const = 0;
  
  /** unique table trivia */
  virtual size_t hash() const = 0;
  virtual bool operator==(const _MLShom &h) const=0;
  // for use by unique table : return new MyConcreteClassName(*this);
  virtual _MLShom * clone () const =0 ;
  
};

class StrongMLShom : public _MLShom {
  public :
  
  bool operator==(const _MLShom &h) const;
  
  virtual bool operator==(const StrongMLShom &) const=0;
  
  SHomNodeMap eval(const GSDD &) const ;
  
  /// User defined behavior is input through this function
  virtual SHomHomMap phi (int var,const DataSet & val) const=0;   
  virtual SHomNodeMap phiOne () const=0;   
  
};

#endif
