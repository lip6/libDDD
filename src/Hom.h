/* -*- C++ -*- */
#ifndef HOM_H_
#define HOM_H_

#include "DDD.h"
#include "DataSet.h"
#include "hashfunc.hh"

#include <string>
#include <map>
/**********************************************************************/
#ifdef INST_STL
typedef  std::pair<long long int, long long int> PairLL;
typedef std::map<std::string, PairLL > MapJumps;
#endif

/// pre-declaration of concrete (private) class implemented in .cpp file
class _GHom;
/// pre-declaration of class for user defined operations
class StrongHom;
/// pre-declaration of V. Beaudenon's class for serialization/deserialization
class MyGHom;

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
  /// Open access to hash function computation procedure.
  friend struct hash<GHom>;
  /// This operator applies its argument to a node until a fixpoint is reached.
  /// Application consists in : while ( h(d) != d ) d = h(d);
  /// Where d is a DDD and h a homomorphism.
  friend GHom fixpoint(const GHom &);
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
  /// The real implementation class. All true operations are delagated on this pointer.
  /// Construction/destruction take care of ensuring concret is only instantiated once in memory.  
  const _GHom* concret;
  /// A private constructor used in internals. 
  /// \param _h The pointer provided should point into the unicity table
  GHom(const _GHom *_h):concret(_h){};
public:
  /// \name Public Constructors 
  //@{
  /// Default public constructor.
  /// Builds Identity homomorphism : \forall d \in DDD, id(d) = d
  GHom():concret(id.concret){};
  /// Create a (general) homomorphism from a user defined StrongHom.
  /// Common usage is : GHom h = GHom ( new myUserDefinedStrongHom(parameters) )
  GHom(const StrongHom *);
  /// Create a (general) homomorphism from a user defined StrongHom.
  /// Common usage is : GHom h = GHom ( new myUserDefinedStrongHom(parameters) )
  GHom(StrongHom *);
  
  /// Create a (general) homomorphism from a user defined MyGHom.
  /// Purpose of MyGhom is unknown... get rid of it ?
  /// \todo : eliminate references to this class.
  GHom(const MyGHom *);
  /// Another MyGhom artifact
  /// \todo get rid of me !
  GHom(MyGHom *);
  
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
  /// Total ordering function between Hom. Note that comparison is based on "concret" address in unicity table.
  /// This ordering is necessary for hash and map storage of GHom.
  /// \param h the node to compare to
  /// \return true if argument h is greater than "this".
  bool operator<(const GHom &h) const{return concret<h.concret;};
  //@}

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


  /// Accessor to visualize the reference count of the concret instance.
  /// See _GHom::refCounter for details.
  int refCounter() const;

  /// A constructor for a union of several homomorphisms.
  /// Note that for canonisation and optimization reasons, union is an n-ary and commutative composition operator.
  /// Use of this constructor may be slightly more efficient than using operator+ multiple times.
  /// H({h1,h2, ..hn}) (d) = \sum_i h_i (d).
  /// \param the set of homomorphisms to put in the union.
  /// \todo std::set not very efficient for storage, replace by a sorted vector ?
  static GHom add(const set<GHom>&);


  /// \name Memory Management 
  //@{
  /// Returns unicity table current size. Gives the number of different _GHom created and not yet destroyed.
  static  unsigned int statistics();
  /// Prints some statistics to std::cout. Mostly used in debug and development phase.
  /// \todo allow output in other place than cout. Clean up output.
  static void pstats(bool reinit=true);
  /// For garbage collection internals. Marks a GHom as in use in garbage collection phase. 
  void mark()const;
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
GHom fixpoint(const GHom &);
GHom operator+(const GHom &,const GHom &); 
GHom operator&(const GHom &,const GHom &); // composition
GHom operator*(const GDDD &,const GHom &); 
GHom operator*(const GHom &,const GDDD &); 
GHom operator^(const GDDD &,const GHom &); 
GHom operator^(const GHom &,const GDDD &); 
GHom operator-(const GHom &,const GDDD &); 


class Hom:public GHom /*, public DataSet*/ {
 public:
  /* Constructor */
  Hom(const GHom &h=GHom::id);
  Hom(const Hom &h);
  Hom(const GDDD& d);   // constant
  Hom(int var, int val, const GHom &h=GHom::id);  // var -- val -> Id
  ~Hom();
  
  /* Set */
  Hom &operator=(const GHom &);
  Hom &operator=(const Hom &);

  

};

/******************************************************************************/
namespace __gnu_cxx {
  template<>
  struct hash<GHom> {
    size_t operator()(const GHom &g) const{
      //return (size_t) g.concret;
      return ddd::knuth32_hash(reinterpret_cast<const size_t>(g.concret));
    }
  };
}

namespace std {
  template<>
  struct equal_to<GHom> {
    bool operator()(const GHom &g1,const GHom &g2) const{
      return g1==g2;
    }
  };
}

namespace std {
  template<>
  struct less<GHom> {
    bool operator()(const GHom &g1,const GHom &g2) const{
      return g1<g2;
    }
  };
}

/**********************************************************************/
class _GHom{
private:
  friend class GHom;
  friend class Hom;
  mutable int refCounter;
  mutable bool marking;
  mutable bool immediat;
public:
#ifdef INST_STL
  static MapJumps HomJumps;
#endif
  
  /* Destructor*/
  _GHom(int ref=0,bool im=false):refCounter(ref),marking(false),immediat(im){};
  virtual ~_GHom(){};

  /* Compare */
  virtual bool operator==(const _GHom &h) const=0;
  virtual size_t hash() const=0;

  /* Eval */
  virtual GDDD eval(const GDDD &)const=0;

  /* Memory Manager */
  virtual void mark() const{};
#ifdef INST_STL
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

class StrongHom:public _GHom{
public:
  StrongHom(){};
  virtual ~StrongHom(){};
  virtual GDDD phiOne() const{return GDDD::top;}; 
  virtual GHom phi(int var,int val) const=0; 
  virtual bool operator==(const StrongHom &h) const=0;
  bool operator==(const _GHom &h) const;

  //Eval
  GDDD eval(const GDDD &)const;  
};
 

/// Unknown function for this class.
/// \todo : What IS this for ? get rid of it.
class MyGHom:public _GHom{};

#endif /* HOM_H_ */







