/* -*- C++ -*- */
#ifndef SHOM_H_
#define SHOM_H_

#include "DataSet.h"
#include "SDD.h"

#include <string>
#include <map>
/**********************************************************************/

#ifdef INST_STL
/// To store hit/miss info in INST_STL context
typedef  std::pair<long long int, long long int> PairLL;
/// To store hit/miss info in INST_STL context
typedef std::map<std::string, PairLL > MapJumps;
#endif

/// pre-declare the concrete storage class
class _GShom;
/// pre-declare StrongShom for constructor(s) of GShom
class StrongShom;

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
  /// Open access to concret for hash key computation
  friend struct hash<GShom>;

  /// \name Friendly hard coded composition operators.
  /// Open full access for library implemented hard coded operations.    
  //@{
  friend GShom fixpoint(const GShom &);
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
  /// Private constructor. This is only called with pointers into the unicity table.
  /// \param _h pointer into the unicity table
  GShom(const _GShom *_h):concret(_h){};
public:
  /// \name Public Constructors 
  //@{
  /// Default constructor builds identity homomorphism.
  GShom():concret(id.concret){};
  /// Construct a homomorphism from a (user defined) Strong homomorphism. 
  /// Pointer provided should be into unicity table. 
  /// \todo Where is this used ? is it useful ?
  GShom(const StrongShom *);
  /// Construct a homomorphism from a (user defined) Strong homomorphism. 
  /// No constraints on the pointer which will be canonized for unicity.
  GShom(StrongShom *);
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

  /// \name Comparisons between GShom.
  /// Comparisons between GShom for unicity table. Both equality comparison and
  /// total ordering provided to allow hash and map storage of GShom
  //@{
  bool operator==(const GShom &h) const{return concret==h.concret;};
  bool operator!=(const GShom &h) const{return concret!=h.concret;};
  bool operator<(const GShom &h) const{return concret<h.concret;};
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
  static GShom add(const set<GShom>&);

  /// \name  Memory Management routines. 
  //@{
  /// Return the current size of the unicity table for GShom.
  static  unsigned int statistics();
  /// Print some usage statistics on Shom. Mostly used for development and debug.
  /// \todo Allow output not in std::cout.
  static void pstats(bool reinit=true);
  /// Mark a concrete data as in use (forbids garbage collection of the data).
  void mark() const;
  /// Collects and destroys unused homomorphisms. Do not call this directly but through 
  /// MemoryManager::garbage() as order of calls (among GSDD::garbage(), GShom::garbage(), 
  /// SDED::garbage()) is important.
  static void garbage();
  //@}
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
GShom fixpoint(const GShom &);
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

  /// \name Assignment operators.
  //@{
  /// Overloaded behavior for assignment operator, maintains reference counting.
  Shom &operator=(const GShom &);
  /// Overloaded behavior for assignment operator, maintains reference counting.
  Shom &operator=(const Shom &);
  //@}
};

/******************************************************************************/


namespace __gnu_cxx {
  /// Computes a hash key for an Shom. 
  /// Value returned is based on unicity of concret in unicity table.
  template<>  struct hash<GShom> {
    size_t operator()(const GShom &g) const{
      return (size_t) g.concret;
    }
  };
}

namespace std {
  /// Compares two GShom in hash tables. 
  /// Value returned is based on unicity of concret in unicity table.
  template<>  struct equal_to<GShom> {
    bool operator()(const GShom &g1,const GShom &g2) const{
      return g1==g2;
    }
  };
}

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
public:
#ifdef INST_STL
  /// For hash table optimization.
  static MapJumps HomJumps;
#endif
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
  /// may be quite costly. Use INST_STL version of the library to check your hash functions
  /// for good behavior.
  virtual size_t hash() const=0;

  /// The computation function responsible for evaluation over a node.
  /// Users should not directly use this. Normal behavior is to use GShom::operator()
  /// that encapsulates this call with operation caching.
  virtual GSDD eval(const GSDD &) const=0;

  /// For garbage collection. Used in first phase of garbage collection.
  virtual void mark() const{};

#ifdef INST_STL
  /// For hash table optimization.
  virtual void InstrumentNbJumps(int nbjumps)
  {
    const char *name=typeid(*this).name();
    MapJumps::iterator ii;
    if ((ii=HomJumps.find(string(name)))==HomJumps.end()){
      HomJumps[string(name)]=PairLL(1L, (long long int) (1+nbjumps));
    }
    else {
      ii->second.first++;
      ii->second.second+=(1+nbjumps);
    }
  }
#endif
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

  /// The evaluation mechanism of strong homomorphisms. 
  /// Evaluation is defined as : 
  /// 
  /// Let an SDD d= (var, Union_i (val_i, d_i) )
  ///
  /// h (d) = Sum_i ( phi(var, val_i) (d_i) ) 
  GSDD eval(const GSDD &)const;  
};
 
#endif /* SHOM_H_ */
