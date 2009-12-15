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
#ifndef SHOM_H_
#define SHOM_H_

#include "DataSet.h"
#include "SDD.h"
#include "Hom.h"
#include "hashfunc.hh"

#include <string>
#include <iostream>
#include <map>

#include "util/set.hh"
#include "util/hash_support.hh"

/**********************************************************************/

/// pre-declare the concrete storage class
class _GShom;
/// pre-declare StrongShom for constructor(s) of GShom
class StrongShom;
class MyGShom;

/// This class is the base class for Homomorphisms over SDD.
/// Composition operators between homomorphisms are defined at this level,
/// so this is the common ground between user-defined homomorphisms (i.e. 
/// derived classes of StrongShom) and hard-coded operations such as union,
/// concatenation etc... Like GSDD, GShom implement a smart pointer behavior, 
/// the actual data (instance of hidden private class _GShom) linked to an 
/// instance of GShom has only one occurrence in memory, thanks to 
/// a unicity table.
/// 
/// User homomorphisms should be manipulated and instanciated using Shom, as GShom 
/// provide no reference counting and are thus likely to be collected in a 
/// MemoryManager::garbage() call. Shom provides reference counting.
class GShom 
{
private:
  /// Open access to concret for Shom class
  friend class Shom;
    /// Open access to _GShom based homomophisms
  friend class _GShom;

  /// \name Friendly hard coded composition operators.
  /// Open full access for library implemented hard coded operations.    
  //@{
  friend GShom fixpoint(const GShom &, bool is_top_level);
  friend GShom localApply(const GHom &,int target);
  friend GShom localApply(const GShom &,int target);
  friend GShom add(const d3::set<GShom>::type &);
  friend GShom operator+(const GShom &,const GShom &); 
  friend GShom operator&(const GShom &,const GShom &); 
  friend GShom operator*(const GSDD &,const GShom &); 
  friend GShom operator*(const GShom &,const GSDD &); 
  friend GShom operator^(const GSDD &,const GShom &); 
  friend GShom operator^(const GShom &,const GSDD &); 
  friend GShom operator-(const GShom &,const GSDD &);
  //@}

  /// Pointer to the data instance in the unicity table.
  const _GShom* concret;
public:
  typedef GSDD NodeType;
  /// \name Public Constructors 
  //@{
  /// Default constructor builds identity homomorphism.
  GShom():concret(id.concret){};

  /// Pseudo-private constructor. This should only be called with pointers into the unicity table.
    /// For example return this in a StrongHom phi() body is legal.
  /// \param _h pointer into the unicity table
  GShom(const _GShom *_h);

  /// THIS VERSION IS DELIBERATELY UNIMPLEMENTED
  /// OTHERWISE bad calls like GShom(new myHom()) would promote to const _GShom *_h and bypass unicity.
  /// User code prior to 20/05/08 would use this in the form : return new myHom(xx);
  /// This is now illegal as we take up memory allocation now, so the user should
  /// stack alloc and pass a reference as in GShom(const _GShom &_h). Exceptionally,
  /// for efficiency, return this; in a phi user function is permitted hence public
  ///  visibility of above GShom(const _GShom *_h);
  /// This signature is here to ensure link errors in old user code.
  GShom(_GShom *_h);

    ///  To build GShom from pointers to user homomomorphisms. 
    /// This call ensures unicity of representation.
   GShom(const _GShom &_h);


  /// Construct a constant homomorphism. Applied to any SDD this homomorphism
  /// will return the value it was initialized with.
  GShom(const GSDD& d);   
  /// Left concatenation of a single arc SDD. This is provided as a convenience
  /// and to avoid the inefficiency if we build a node pointing to GSDD::one
  /// and then concatenate something to it. Applied to a SDD d, this homomorphism
  /// will return var--val->h(d).
  /// \param var the variable labeling the node to left concat
  /// \param val the set of values labeling the arc
  /// \param h the homomorphism to apply on the argument d. This defaults to GSHom::id.
  GShom(int var,const DataSet& val, const GShom &h=GShom::id);  // var -- val -> Id
  //@}

  /// Elementary SDD homomorphism identity. Applied to any SDD d, it returns d. 
  static const GShom id;

  /// returns the predescessor homomorphism, using pot to determine variable domains
  GShom invert (const GSDD & pot) const;

  /// \name Comparisons between GShom.
  /// Comparisons between GShom for unicity table. Both equality comparison and
  /// total ordering provided to allow hash and map storage of GShom
  //@{
  bool operator==(const GShom &h) const{return concret==h.concret;};
  bool operator!=(const GShom &h) const{return concret!=h.concret;};
  bool operator<(const GShom &h) const{return concret<h.concret;};
  /// This predicate is true if the homomorphism global behavior is only to prune some paths.
  bool is_selector() const;
  /// This predicate is true if the homomorphism "skips" this variable.
  bool skip_variable(int) const;
  
  typedef d3::set<int>::type range_t;
  typedef range_t::const_iterator range_it;
  /// Returns the range for this homomorphism, i.e. the dual of skip_variable
  const range_t  get_range () const;  
  /// The full_range : that targets everyone
  static const range_t full_range;
  //@}

  /// \name Evaluation mechanism for homomorphisms.
  //@{
  /// Applying a homomorphism to a node returns a node. 
  /// This is the normal way for users of computing the application of a homomorphism.
  GSDD operator()(const GSDD &d) const; 
  /// The full evaluation, this is the computational procedure, that is called when 
  /// the computation cache yields a miss. Users should not use this function directly, 
  /// but only through GSDD::operator().
  GSDD eval(const GSDD &d) const;
  //@}
  
  /// For debug and development purposes. Gives the reference count of the concrete data. 
  int refCounter() const;

  /// Compute an n-ary sum between homomorphisms. This should be slightly more efficient 
  /// in evaluation than a composition of binary sums constructed using the friend operator+.
  /// \todo : move this to friend status not static member for more homogeneity with other operators.
    static GShom add(const d3::set<GShom>::type & s);

  // pretty print of homomorphisms
  friend std::ostream & operator << (std::ostream & os, const GShom & h);

  /// \name  Memory Management routines. 
  //@{
  /// Return the current size of the unicity table for GShom.
  static  unsigned int statistics();
  /// Return the current size of the cache for GShom.
  static size_t cache_size();
  
  /// Print some usage statistics on Shom. Mostly used for development and debug.
  /// \todo Allow output not in std::cout.
  static void pstats(bool reinit=true);
  /// Mark a concrete data as in use (forbids garbage collection of the data).
  void mark() const;
  /// For storage in a hash table
  size_t hash () const { 
    return ddd::knuth32_hash(reinterpret_cast<const size_t>(concret)); 
  }
  /// Collects and destroys unused homomorphisms. Do not call this directly but through 
  /// MemoryManager::garbage() as order of calls (among GSDD::garbage(), GShom::garbage(), 
  /// SDED::garbage()) is important.
  static void garbage();
  //@}

  // strategies for fixpoint evaluation insaturation context
  // BFS = do each g_i once then go to g_i+1
  // DFS = do each g_i to saturation then go to g_i+1
  enum fixpointStrategy {BFS, DFS};
  
 private : 
  static fixpointStrategy fixpointStrategy_;
 public :
  static fixpointStrategy getFixpointStrategy() { return fixpointStrategy_; }
  static void setFixpointStrategy(fixpointStrategy strat) { fixpointStrategy_ = strat; }


};



/// \name Composition operators between SDD homomorphisms.
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
GShom fixpoint(const GShom &, bool is_top_level=false);
/// Apply a homomorphism on a target variable.
/// This ensures that the operation is local to this variable, and is used to implement auto-saturation.
 GShom localApply(const GHom &,int target);
 GShom localApply(const GShom &,int target);

/// An IF-THEN-ELSE construct.
/// The behavior of the condition **must** be a selection, as indicated by its isSelector() flag.
/// PITFALL : Otherwise an assertion violation will be raised (with an explicit stderr message)
///
/// Semantics : ITE ( cond, iftrue, iffalse) (d) =  (iftrue & cond(d)) + (iffalse & !cond(d)) 
GShom ITE (const GShom & cond, const GShom & iftrue, const GShom & iffalse);

/// A negation/complement constructor for **selector** homomophisms.
/// Let cond be a selector, !cond(d) = d - cond(d)
/// PITFALL : Raises an assert violation if is_selector() returns false !
GShom operator! (const GShom & cond);

/// Composition by union of two homomorphisms. 
/// See also GShom::add(). This commutative operation computes a homomorphism 
/// that evaluates as the sum of two homomorphism.
///
/// Semantics : (h1 + h2) (d) = h1(d) + h2(d).
GShom operator+(const GShom &,const GShom &); 
/// Composition by circ (rond) of homomorphisms.
/// 
/// Semantics :  (h1 & h2) (d) = h1( h2(d) ).
GShom operator&(const GShom &,const GShom &);
/// Intersection with a constant SDD.
/// Semantics : (h * d1) (d) = h(d) * d1
GShom operator*(const GSDD &,const GShom &);
/// Intersection with a constant SDD.
/// Semantics : (d1 * h) (d) = d1 * h(d)
GShom operator*(const GShom &,const GSDD &); 
/// Intersection with a selector Hom.
/// Semantics : (d1 * sel) (d) = d1 (d) * sel (d)
/// WARNING : assert failure if 2nd argument is not a selector.
GShom operator*(const GShom &,const GShom &); 
/// Left Concatenation of a constant SDD.
/// Note that this is inherently inefficient, the nodes of d1 are constructed, 
/// but the result a priori will not contain them, unless h(d) == GSDD::one.
///
/// Semantics : (d1 ^ h) (d) = d1 ^ h(d)
GShom operator^(const GSDD &,const GShom &); 
/// Right Concatenation of a constant SDD.
/// This is used to construct new nodes, and has the same efficiency issue as 
/// left concatenation.
///
/// Semantics : (h ^ d1) (d) =  h(d) ^ d1
GShom operator^(const GShom &,const GSDD &);
/// Set difference.
/// Note that this operation is not commutative, nor is it \e linear.
/// This means the difference of two linear homomorphisms is not necessarily linear;
/// (h1 - h2) (d) is not necessarily equal to h1(d) - h2(d). Therefore this operator is 
/// not defined for composition of two homomorphisms, only for a constant and a homomorphism.
///
/// Semantics : (h - d1) (d) =  h(d) - d1
GShom operator-(const GShom &,const GSDD &); 
/// Set difference (generalized).
/// Note that this operation is not commutative, nor is it \e linear.
/// This means the operation a priori cannot "skip", and it could be incorrect to apply it at
///  an arbitrary level of the structure. However, in some cases it is still useful and it can
/// be applied directly at the top node of the structure yielding correct results.
///
/// Semantics : (h1 - h2) (d) =  h1(d) - h2(d)
GShom operator-(const GShom &,const GShom &); 
//@}


/// This is the user interface class to manipulate homomorphisms.
/// The only difference with GShom is that it implements reference counting
/// so that instances of Shom are not collected upon MemoryManager::garbage().
class Shom : public GShom 
{
public:
  /// \name Public Constructors.
  /// Default constructor builds identity homomorphism.
  //@{
  /// Build an Shom from a GShom.
  Shom(const GShom &h=GShom::id);
  /// Copy constructor. Maintains reference count.
  Shom(const Shom &h);
  /// Constructs a constant homomorphism. 
  Shom(const GSDD& d);  
  /// Left concatenation of a single arc SDD. This is provided as a convenience
  /// and to avoid the inefficiency if we build a node pointing to GSDD::one
  /// and then concatenate something to it. Applied to a SDD d, this homomorphism
  /// will return var--val->h(d).
  /// \param var the variable labeling the node to left concat
  /// \param val the set of values labeling the arc
  /// \param h the homomorphism to apply on the argument d. This defaults to GSHom::id.
  Shom(int var,const DataSet& val, const GShom &h=GShom::id); 
  /// Destructor maintains reference counting. 
  /// Note that the destructor does not truly reclaim memory, MemoryManager::garbage() does that.
  ~Shom();
  //@}

  // Elementary emptyset homomorphism
  static const Shom null;

  /// \name Assignment operators.
  //@{
  /// Overloaded behavior for assignment operator, maintains reference counting.
  Shom &operator=(const GShom &);
  /// Overloaded behavior for assignment operator, maintains reference counting.
  Shom &operator=(const Shom &);
  //@}
};

/******************************************************************************/



namespace std {
  /// Compares two GShom in hash tables. 
  /// Value returned is based on unicity of concret in unicity table.
  template<>  struct less<GShom> {
    bool operator()(const GShom &g1,const GShom &g2) const{
      return g1<g2;
    }
  };
}

/**********************************************************************/
/// The concrete data class for Homomorphisms.
/// Users should not use this (private) class directly, but may use it indirectly 
/// by deriving user homomorphisms from the StrongShom class.
class _GShom 
{
private:
  /// open access to container class GShom.
  friend class GShom;
  /// open access to container class Shom.
  friend class Shom;
  /// For garbage collection. 
  /// Counts the number of times a _GShom is referenced from the context of an Shom.
  mutable int refCounter;
  /// For garbage collection. Used in the two phase garbage collection process.
  /// A Shom that is not marked after the first pass over the unicity table, will
  /// be sweeped in the second phase. Outside of garbage collection routine, marking
  /// should always bear the value false.
  mutable bool marking;
  /// For operation cache management. 
  /// If immediat==true,  eval is called without attempting a cache hit. 
  /// Currently only the constant homomorphism has this attribute set to true.
  mutable bool immediat;
  /// The procedure responsible for propagating efficiently across "skipped" variable nodes.
    GSDD eval_skip(const GSDD &) const;
  
public:

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
      virtual const GShom::range_t get_range () const 
    {
      return GShom::full_range;
    }

  /// Constructor. Note this class is abstract, so this is only used in initialization
  /// list of derived classes constructors (hard coded operations and StrongShom).
  _GShom(int ref=0,bool im=false):refCounter(ref),marking(false),immediat(im){};
  /// Destructor. Default behavior. 
  /// \todo Remove this declaration ? compiler generated version sufficient.
  virtual ~_GShom(){};

  /// Comparator. Used in case of hash collision. 
  /// Should be appropriately defined in derived classes, in particular in user defined
  /// homomorphisms.
  virtual bool operator==(const _GShom &h) const=0;
  /// Hash key computation. It is essential for good hash table operation that the spread
  /// of the keys be as good as possible. Also, fast hash key computation is a good design goal.
  /// Note that bad hash functions will yield more collisions, thus equality comparisons which
  /// may be quite costly.
  virtual size_t hash() const=0;

  // for use by unique table : return new MyConcreteClassName(*this);
  virtual _GShom * clone () const =0 ;


  /// The computation function responsible for evaluation over a node.
  /// Users should not directly use this. Normal behavior is to use GShom::operator()
  /// that encapsulates this call with operation caching.
  virtual GSDD eval(const GSDD &) const=0;

  /// For garbage collection. Used in first phase of garbage collection.
  virtual void mark() const{};

  virtual void print (std::ostream & os) const = 0;
public:
    
    // Enable access to the concrete GSHom for _GSHom homorphisms
  /// TODO : this is a dirty trick to allow us to do terms rewriting in Add, Fixpoint etc...
  /// A more elegant architecture would be nice.
    static
	const _GShom*
    get_concret(const GShom& gshom)
    {
        return gshom.concret;
    }

    // produce the predescessor homomorphism, using pot to compute variable domains
    virtual GShom invert (const GSDD & ) const { 
      // default = raise assert
      if ( is_selector() ) {
	// A default implmentation is provided for selector homomorphisms, overloadable.
	// sel^-1 (s) = pot - sel(pot) + s = ((pot-sel(pot)) + id)
	// return  ( (pot - GShom(this)(pot))+ GShom::id ); 
	// NEW VERSION : the invert of a selection is itself...
	return this;
      }
      // No default implem if ! is_selector 
      std::cerr << "Cannot invert homomorphism : " ;
      print (std::cerr);
      std::cerr << std::endl ;
      assert(0); 
      return Shom::null;
    }

};

/// The abstract base class for user defined operations. 
/// This is the class users should derive their operations from.
/// It defines the interface of a Strong Homomorphism :
/// * evaluation over terminal GSDD::one, which should return a constant SDD
/// * evaluation over an arbitrary var--val--> pair, that returns a 
///   \e homomorphism to apply to the successor node
/// Users also have to provide a comparison function, and a hash function (from _GShom).
/// See the demo folder for examples of user defined homomorphisms.
class StrongShom : public _GShom 
{
public:
  /// Default constructor. Empty behavior.
  /// \todo Is this declaration useful ?
  StrongShom(){};
  /// Default destructor. Empty behavior.
  /// \todo Is this declaration useful ?
  virtual ~StrongShom(){};
  /// Evaluation over terminal GSDD::one. Returns a constant SDD.
  /// A homomorphism that does not overload phiOne does not expect to meet
  /// the terminal during it's evaluation, therefore default behavior returns GSDD::top
  virtual GSDD phiOne() const{return GSDD::top;}; 
  /// Evaluation over an arbitrary arc of a SDD. 
  /// \param var the index of the variable labeling the node.
  /// \param val the set of values labeling the arc.
  /// \return a homomorphism to apply on the successor node
  virtual GShom phi(int var,const DataSet& val) const=0;
  /// Comparator is pure virtual. Define a behavior in user homomorphisms.
  virtual bool operator==(const StrongShom &h) const=0;
  /// Comparator for unicity table. Users should not use this. The behavior is 
  /// to check for type mismatch (and return false if that is the case) or call 
  /// specialized comparators of derived subclasses otherwise.
  bool operator==(const _GShom &h) const;

  /// pretty print
  virtual void print (std::ostream & os) const ;

  /// The evaluation mechanism of strong homomorphisms. 
  /// Evaluation is defined as : 
  /// 
  /// Let an SDD d= (var, Union_i (val_i, d_i) )
  ///
  /// h (d) = Sum_i ( phi(var, val_i) (d_i) ) 
  GSDD eval(const GSDD &)const;  
};

class MyGShom : public _GShom{};
 
#endif /* SHOM_H_ */
