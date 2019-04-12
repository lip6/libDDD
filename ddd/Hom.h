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

/* -*- C++ -*- */
#ifndef HOM_H_
#define HOM_H_

#include "ddd/DDD.h"
#include "ddd/util/hash_support.hh"
#include "ddd/util/set.hh"

#include <map>
#include <cassert>
#include <iostream>
/**********************************************************************/

/// pre-declaration of concrete (private) class implemented in .cpp file
class _GHom;
/// pre-declaration of class for user defined operations
class StrongHom;
/// pre-declaration of V. Beaudenon's class for serialization/deserialization
class MyGHom;
/// predeclaration for compatibility with ML hom
class MLHom;

/// This class is the base class representing a homomorphism over DDD.
/// A DDD homomorphism is a linear application that respects the better-defined relation (see ICATPN'2002 paper by Couvreur et al.).
/// It comes with composition &, union +, and intersection * operators to construct complex operations from two homomorphisms.
/// It also comes with the fixpoint() unary operator, that allows to implement saturation (see FORTE'05 paper by Couvreur & Thierry-Mieg)
/// This class does not implement reference counting : 
/// GHom are destroyed on MemoryManager::Garbage
/// unless they are also referenced as Hom.
/// Note that this class is in fact a kind of smart pointer : operations are delegated on "concret"
/// the true implementation class (of private hidden type _GHom) that contains the data and has a single 
/// memory occurrence thanks to the unicity table.
class GHom {
private:
  /// Open access to Hom derived class
  friend class Hom;
  /// Open access to _GHom based homomophisms
  friend class _GHom;
  /// This operator applies its argument to a node until a fixpoint is reached.
  /// Application consists in : while ( h(d) != d ) d = h(d);
  /// Where d is a DDD and h a homomorphism.
  friend GHom fixpoint(const GHom &, bool);
  /// This operator applies its arguments to a node until a fixpoint is reached.
  /// Similar to the fixpoint, but we suppose here that the parameters are commutative
  /// And that any application order converges to the same result
  /// Typically this is the property of a base of monotonic< permutations.
  friend GHom monotonic(const d3::set<GHom>::type & set);
  /// This operator creates an operation that is the union of two operations.
  /// By definition, as homomorphism are linear, (h+g) (d) = h(d) + g(d) ;
  /// Where g,h are homomorphisms and d is a DDD.
  friend GHom operator+(const GHom &,const GHom &);
  /// This operator creates an operation that is the composition of two operations.
  /// (h & g) (d) = h( g(d) ) ;
  /// Where g,h are homomorphisms and d is a DDD.
  friend GHom operator&(const GHom &,const GHom &); 
  ///  This operator creates an operation that is the intersection of an operation and a constant DDD.
  /// (d1 * h) (d2) = d1 * h(d2) ;
  /// Where h is a homomorphism and d1, d2 are DDD.
  friend GHom operator*(const GDDD &,const GHom &); 
  ///  This operator creates an operation that is the intersection of an operation and a constant DDD.
  /// (h * d1) (d2) = h(d2) * d1  ;
  /// Where h is a homomorphism and d1, d2 are DDD.
  friend GHom operator*(const GHom &,const GDDD &); 
  ///  This is the left concatenantion operator, that adds a constant DDD above the operation.
  /// (d1 ^ h) (d2) = d1 ^ h(d2)
  /// Where h is a homomorphism and d1, d2 are DDD.
  friend GHom operator^(const GDDD &,const GHom &);
  ///  This is the right concatenantion operator, that adds a constant DDD below the operation.
  /// (h ^ d1) (d2) = h(d1) ^ d2
  /// Where h is a homomorphism and d1, d2 are DDD.
  friend GHom operator^(const GHom &,const GDDD &); 
  /// This is a set difference constructor, only available for (hom - ddd), not hom - hom as that might not preserve linearity.
  /// (h - d1) (d2) = h(d2) - d1
  /// Where h is a homomorphism and d1, d2 are DDD.
  friend GHom operator-(const GHom &,const GDDD &); 
  
#ifdef HASH_STAT
  // open access to instrumented hashtable
  template <class Value, class Key, class HashFcn,
  class ExtractKey, class SetKey, class EqualKey, class Alloc>
  friend class google::sparse_hashtable;
#endif
  
  /// The real implementation class. All true operations are delagated on this pointer.
  /// Construction/destruction take care of ensuring concret is only instantiated once in memory.  
  const _GHom* concret;
public:
  typedef GDDD NodeType;
  /// A uncontrolled constructor used in internals. 
  /// Made public for calls like return GHom(this) in StrongHom::phi definitions.
  /// \param _h The pointer provided should point into the unicity table
  GHom(const _GHom *_h):concret(_h){};

  /// THIS VERSION IS DELIBERATELY UNIMPLEMENTED
  /// OTHERWISE bad calls like GShom(new myHom()) would promote to const _GShom *_h and bypass unicity.
  /// User code prior to 20/05/08 would use this in the form : return new myHom(xx);
  /// This is now illegal as we take up memory allocation now, so the user should
  /// stack alloc and pass a reference as in GShom(const _GShom &_h). Exceptionally,
  /// for efficiency, return this; in a phi user function is permitted hence public
  ///  visibility of above GShom(const _GShom *_h);
  /// This signature is here to ensure link errors in old user code.
  GHom(_GHom *_h);

  /// build a GHom from user provided homomorphisms such as StrongHom.
  /// This call ensures canonization of h
  GHom(const _GHom &_h);
  /// \name Public Constructors 
  //@{
  /// Default public constructor.
  /// Builds Identity homomorphism : forall d in DDD, id(d) = d
  GHom():concret(id.concret){};

  /// Encapsulate an MLHom, by setting a stop level for the upstream homomorphisms.
  GHom(const MLHom &);
  
  /// Create a constant DDD homomorphism.
  /// These are the basic building bricks of more complex operations.
  /// h(d1) (d2) = d1
  /// Where d1 is a DDD, h(d1) a constant homomorphism and d2 an arbitrary DDD.
  GHom(const GDDD& d);   
  /// Create variable/value pair and left concatenate to a homomorphism.
  /// h(var,val,g) (d) = DDD(var,val) ^ g(d).
  /// In other words : var -- val -> g
  /// \param var the variable index
  /// \param val the value associated to the variable
  /// \param h the homomorphism to apply on successor node. Default is identity, so is equivalent to a left concatenation of a DDD(var,val).
  GHom(int var, int val, const GHom &h=GHom::id);  
  //@}

  /// Elementary homomorphism Identity, defined as a constant.
  /// id(d) = d
  static const GHom id;

  /// \name Comparisons for hash and map storage
  //@{
  /// Comparison between Homomorphisms. Note that comparison is based on "concret" address in unicity table.
  /// \param h the hom to compare to
  /// \return true if the nodes are equal.  
  bool operator==(const GHom &h) const{return concret==h.concret;};
  /// Comparison between Homomorphisms. Note that comparison is based on "concret" address in unicity table.
  /// \param h the hom to compare to
  /// \return true if the nodes are NOT equal.  
  bool operator!=(const GHom &h) const{return concret!=h.concret;};
  /// Total ordering function between Hom.
  /// Note that comparison is based on chronological ordering of creation, and delegated to "concret".
  /// Unlike comparison on addresses in unicity table, this ensures reproductible results.
  /// This ordering is necessary for hash and map storage of GHom.
  /// \param h the node to compare to
  /// \return true if argument h is greater than "this".
  bool operator<(const GHom &h) const;
  /// This predicate is true if the homomorphism global behavior is only to prune some paths.
  bool is_selector() const;
  //@}

	typedef d3::set<int>::type range_t;
	typedef range_t::const_iterator range_it;
	/// Returns the range for this homomorphism, i.e. the dual of skip_variable
	const range_t  get_range () const;  
	/// The full_range : that targets everyone
	static const range_t full_range;
	
  /// returns the predescessor homomorphism, using pot to determine variable domains
  GHom invert (const GDDD & pot) const;

  /// returns true if and only if h(d) != SDD::null
  GDDD has_image (const GDDD & d) const;

  /// returns a negation of a selector homomorphism h, such that h.negate() (d) = d - h(d)
  GHom negate () const;  

  ///  Evaluation operator. Homomorphisms overload operator(), so they can be directly applied to DDD nodes.
  /// Note that evaluation through this operator uses and updates a cache.
  /// \param d the DDD to apply this h to.
  /// \return h(d), the result of applying this h to d.
  GDDD operator()(const GDDD &d) const; 
  ///  Evaluation function : users should use operator() instead of this.
  ///  This evaluation function does not use the cache, it is called in case of cache miss in the call to operator().
  ///  \param d the argument DDD
  ///  \return h(d) 
  GDDD eval(const GDDD &d) const;
  
  bool skip_variable(int var) const ;

  GHom compose (const GHom &r) const ;

  /// Accessor to visualize the reference count of the concret instance.
  /// See _GHom::refCounter for details.
  int refCounter() const;

  /// A constructor for a union of several homomorphisms.
  /// Note that for canonisation and optimization reasons, union is an n-ary and commutative composition operator.
  /// Use of this constructor may be slightly more efficient than using operator+ multiple times.
  /// H({h1,h2, ..hn}) (d) = sum_i h_i (d).
  /// \param set the set of homomorphisms to put in the union.
  /// \todo std::set not very efficient for storage, replace by a sorted vector ?
    static GHom add(const d3::set<GHom>::type & set);

  /// A constructor for a commutative composition of several homomorphisms.
  /// It's up to user to ensure pairwise commutatitivity off all arguments in the set
  /// \param set the set of homomorphisms to put in the composition.
  /// \todo std::set not very efficient for storage, replace by a sorted vector ?
    static GHom ccompose(const d3::set<GHom>::type & set);


  // pretty print of homomorphisms
  friend std::ostream & operator << (std::ostream & os, const GHom & h);

  /// \name Memory Management 
  //@{
  /// Returns unicity table current size. Gives the number of different _GHom created and not yet destroyed.
  static  unsigned int statistics();
  /// Prints some statistics to std::cout. Mostly used in debug and development phase.
  /// \todo allow output in other place than cout. Clean up output.
  static void pstats(bool reinit=true);
  /// For garbage collection internals. Marks a GHom as in use in garbage collection phase. 
  void mark()const;
  /// For storage in a hash table
  size_t hash () const { 
    return ddd::knuth32_hash(reinterpret_cast<size_t>(concret));
  }
  /// For garbage collection. 
  /// \e WARNING Do not use this function directly !! Use MemoryManager::garbage() to ensure
  /// proper reference counting and cache cleanup.
  /// Garbage collection algorithm is a simple two phase mark and sweep : in phase mark, all nodes with positive
  /// reference counts are marked, as well as their descendants, recursively. In phase sweep, all nodes which are unmarked
  /// are destroyed. This avoids maintaining reference counts during operation : only external references made through
  /// the DDD class are counted, and no recursive reference counting is needed.
  static void garbage(); 
  //@}
};



/* Operations */
/// \name Composition operators between DDD homomorphisms.
//@{
/// Apply a homomorphism until fixpoint is reached. 
/// This new unary operator is introduced to implement local saturation in transition
///  relation evaluation. Proper use of fixpoint allows to effectively tackle the 
/// intermediate size problem of decision diagram based representations. 
/// Note that evaluation simply iterates until a fixpoint is reached, thus to cumulate
/// new states with previously reached it should be combined with GShom::id as in
///
/// fixpoint ( h + GShom::id )
///
GHom fixpoint(const GHom &, bool is_top_level=false);
/// A negation/complement constructor for **selector** homomophisms.
/// Let cond be a selector, !cond(d) = d - cond(d)
/// PITFALL : Raises an assert violation if is_selector() returns false !
GHom operator! (const GHom & cond);
/// An IF-THEN-ELSE construct.
/// The behavior of the condition **must** be a selection, as indicated by its isSelector() flag.
/// PITFALL : Otherwise an assertion violation will be raised (with an explicit stderr message)
///
/// Semantics : ITE ( cond, iftrue, iffalse) (d) =  (iftrue & cond(d)) + (iffalse & !cond(d)) 
GHom ITE (const GHom & cond, const GHom & iftrue, const GHom & iffalse);

/// Composition by union of two homomorphisms. 
/// See also GShom::add(). This commutative operation computes a homomorphism 
/// that evaluates as the sum of two homomorphism.
///
/// Semantics : (h1 + h2) (d) = h1(d) + h2(d).
GHom operator+(const GHom &,const GHom &); 
/// Composition by circ (rond) of homomorphisms.
/// 
/// Semantics :  (h1 & h2) (d) = h1( h2(d) ).
GHom operator&(const GHom &,const GHom &); // composition
/// Intersection with a constant DDD.
/// Semantics : (h * d1) (d) = h(d) * d1
GHom operator*(const GDDD &,const GHom &); 
/// Intersection with a constant SDD.
/// Semantics : (d1 * h) (d) = d1 * h(d)
GHom operator*(const GHom &,const GDDD &); 
/// Intersection with a selector Hom.
/// Semantics : (d1 * sel) (d) = d1 (d) * sel (d)
/// WARNING : assert failure if 2nd argument is not a selector.
GHom operator*(const GHom &,const GHom &);
/// Left Concatenation of a constant SDD.
/// Note that this is inherently inefficient, the nodes of d1 are constructed, 
/// but the result a priori will not contain them, unless h(d) == GSDD::one.
///
/// Semantics : (d1 ^ h) (d) = d1 ^ h(d)
GHom operator^(const GDDD &,const GHom &); 
/// Right Concatenation of a constant SDD.
/// This is used to construct new nodes, and has the same efficiency issue as 
/// left concatenation.
///
/// Semantics : (h ^ d1) (d) =  h(d) ^ d1
GHom operator^(const GHom &,const GDDD &); 
/// Set difference.
/// Note that this operation is not commutative, nor is it \e linear.
/// This means the difference of two linear homomorphisms is not necessarily linear;
/// (h1 - h2) (d) is not necessarily equal to h1(d) - h2(d). Therefore this operator is 
/// not defined for composition of two homomorphisms, only for a constant and a homomorphism.
///
/// Semantics : (h - d1) (d) =  h(d) - d1
GHom operator-(const GHom &,const GDDD &); 

/// Apply a 2 level DDD representing a transition relation to current variable.
GHom apply2k (const GDDD &);

/// Return the domain of the first variable bearing the provided index in the provided DDD
/// Useful for invert computations.
GDDD computeDomain (int var, const GDDD& );

/// return true if the provided operations are commutative
bool commutative (const GHom & h1, const GHom & h2);


//@}



/// This is the user interface class to manipulate homomorphisms.
/// The only difference with Hom is that it implements reference counting
/// so that instances of Hom are not collected upon MemoryManager::garbage().
class Hom:public GHom /*, public DataSet*/ {
 public:
  /* Constructor */
  /// \name Public Constructors.
  /// Default constructor builds identity homomorphism.
  //@{
  /// Build an Hom from a GHom.
  Hom(const GHom &h=GHom::id);
  /// Copy constructor. Maintains reference count.
  Hom(const Hom &h);
  /// Constructs a constant homomorphism. 
  Hom(const GDDD& d);   // constant
  /// Left concatenation of a single arc DDD. This is provided as a convenience
  /// and to avoid the inefficiency if we build a node pointing to GSDD::one
  /// and then concatenate something to it. Applied to a SDD d, this homomorphism
  /// will return var--val->h(d).
  /// \param var the variable labeling the node to left concat
  /// \param val the set of values labeling the arc
  /// \param h the homomorphism to apply on the argument d. This defaults to GSHom::id.
  Hom(int var, int val, const GHom &h=GHom::id);  // var -- val -> Id
  /// Destructor maintains reference counting. 
  /// Note that the destructor does not truly reclaim memory, MemoryManager::garbage() does that.
  ~Hom();
  //@}

  /// \name Assignment operators.
  //@{
  /// Overloaded behavior for assignment operator, maintains reference counting.
  Hom &operator=(const GHom &);
  /// Overloaded behavior for assignment operator, maintains reference counting.
  Hom &operator=(const Hom &);
  //@}
};

/******************************************************************************/

namespace std {
  /// Compares two GHom in hash tables. 
  /// Value returned is based on unicity of concret in unicity table.
  template<>
  struct less<GHom> {
    bool operator()(const GHom &g1,const GHom &g2) const{
      return g1<g2;
    }
  };
}

/**********************************************************************/
/// The concrete data class for Homomorphisms.
/// Users should not use this (private) class directly, but may use it indirectly 
/// by deriving user homomorphisms from the StrongHom class.
class _GHom{
private:
  /// open access to container class GHom.
  friend class GHom;
  /// open access to container class Hom.
  friend class Hom;
  /// For garbage collection. 
  /// Counts the number of times a _GShom is referenced from the context of an Shom.
  mutable int refCounter;
  /// For garbage collection. Used in the two phase garbage collection process.
  /// A Hom that is not marked after the first pass over the unicity table, will
  /// be sweeped in the second phase. Outside of garbage collection routine, marking
  /// should always bear the value false.
  mutable bool marking;
  /// For operation cache management. 
  /// If immediat==true,  eval is called without attempting a cache hit. 
  /// Currently only the constant homomorphism has this attribute set to true.  
  mutable bool immediat;
  /// Counter of objects created (see constructors).
  /// This is used for the ordering between homomorphisms.
  size_t creation_counter;
 
  GDDD eval_skip(const GDDD &) const;
public:
  // made public ONLY for the cache, not part of normal API, use GHom has_image instead (cache etc...).
  GDDD has_image_skip(const GDDD &) const;


    /// The skip_variable predicate indicates which variables are "don't care" with respect to this SHom.
    /// This is defined as a StrongHom with :
    ///  phi(var,val) { if ( skip_variable(var) ) return GShom( var, val, this ); else { real behavior } }
     virtual bool
    skip_variable(int) const
    {
        return false;
    }
    /// The isSelector predicate indicates a homomorphism that only selects paths in the SDD (no modifications, no additions)
    /// Tagging with isSelector() allows to enable optimizations and makes the homomorphism eligible as "condition" in ITE construct.
    virtual bool
    is_selector() const
    {
        return false;
    }

	/// The range returns the dual of skip_variable, default implem considers that all variables are affected by this homomorphism.
	virtual const GHom::range_t get_range () const 
    {
		return GHom::full_range;
    }
	
  /// returns the predescessor homomorphism, using pot to determine variable domains
      virtual GHom invert (const GDDD & ) const {
	// default = raise assert
	if ( is_selector() ) {
	  // A default implementation is provided for selector homomorphisms, overloadable.
	  return this;
	}
	// No default implem if ! is_selector 
	std::cerr << "Cannot invert homomorphism : " ;
	print (std::cerr);
	std::cerr << std::endl ;	
	assert(0); 
	return GHom(GDDD::null);
      }


  /// Constructor. Note this class is abstract, so this is only used in initialization
  /// list of derived classes constructors (hard coded operations and StrongShom).
  _GHom(int ref=0,bool im=false):refCounter(ref),marking(false),immediat(im){
    // creation counter
    static size_t counter = 0;
    creation_counter = counter++;
  }
  /// Virtual Destructor. Default behavior. 
  virtual ~_GHom(){};


  /// Comparator. Used in case of hash collision. 
  /// Should be appropriately defined in derived classes, in particular in user defined
  /// homomorphisms.
  virtual bool operator==(const _GHom &h) const=0;
  /// Ordering between _GHom. It is the chronological ordering of creation
  bool operator< (const _GHom &h) const;
  /// Hash key computation. It is essential for good hash table operation that the spread
  /// of the keys be as good as possible. Also, fast hash key computation is a good design goal.
  /// Note that bad hash functions will yield more collisions, thus equality comparisons which
  /// may be quite costly.
  virtual size_t hash() const=0;

  // for use by unique table : return new MyConcreteClassName(*this);
  virtual _GHom * clone () const =0 ;

  /// The computation function responsible for evaluation over a node.
  /// Users should not directly use this. Normal behavior is to use GShom::operator()
  /// that encapsulates this call with operation caching.
  virtual GDDD eval(const GDDD &)const=0;

  /// For garbage collection. Used in first phase of garbage collection.
  virtual void mark() const{};

  virtual GDDD has_image(const GDDD &) const;
  /// returns a negation of a selector homomorphism h, such that h.negate() (d) = d - h(d)
  virtual GHom negate () const;  


  virtual GHom compose (const GHom &r) const ;

  virtual void print (std::ostream & os) const = 0;
  
  // Enable access to the concrete GHom for _GHom homorphisms
  static const _GHom*
  get_concret(const GHom& ghom)
  {
    return ghom.concret;
  }
  
  
};

/// The abstract base class for user defined operations. 
/// This is the class users should derive their operations from.
/// It defines the interface of a Strong Homomorphism :
/// * evaluation over terminal GDDD::one, which should return a constant DDD
/// * evaluation over an arbitrary var--val--> pair, that returns a 
///   \e homomorphism to apply to the successor node
/// Users also have to provide a comparison function, and a hash function (from _GHom).
/// See the demo folder for examples of user defined homomorphisms.
class StrongHom
	:
    public _GHom
{
public:
  /// Default constructor. Empty behavior.
  /// \todo Is this declaration useful ?
  StrongHom(){};
  /// Default destructor. Empty behavior.
  /// \todo Is this declaration useful ?
  virtual ~StrongHom(){};
  /// Evaluation over terminal GDDD::one. Returns a constant DDD.
  /// A homomorphism that does not overload phiOne does not expect to meet
  /// the terminal during it's evaluation, therefore default behavior returns GDDD::top
  virtual GDDD phiOne() const{return GDDD::top;}; 
  /// Evaluation over an arbitrary arc of a SDD. 
  /// \param var the index of the variable labeling the node.
  /// \param val the value labeling the arc.
  /// \return a homomorphism to apply on the successor node  
  virtual GHom phi(int var,int val) const=0; 
  /// Comparator is pure virtual. Define a behavior in user homomorphisms.
  virtual bool operator==(const StrongHom &h) const=0;
  
  /// Comparator for unicity table. Users should not use this. The behavior is 
  /// to check for type mismatch (and return false if that is the case) or call 
  /// specialized comparators of derived subclasses otherwise.
  bool operator==(const _GHom &h) const;

  /// pretty print
  virtual void print (std::ostream & os) const ;


  /// The evaluation mechanism of strong homomorphisms. 
  /// Evaluation is defined as : 
  /// 
  /// Let an SDD d= (var, Union_i (val_i, d_i) )
  ///
  /// h (d) = Sum_i ( phi(var, val_i) (d_i) ) 
  GDDD eval(const GDDD &)const; 

  virtual GDDD has_image (const GDDD &) const;

};
 

/// Unknown function for this class.
/// \todo : What IS this for ? get rid of it.
class MyGHom:public _GHom{};

#endif /* HOM_H_ */
