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
  /* Constructors */
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



/* Operations */
GShom fixpoint(const GShom &);
GShom operator+(const GShom &,const GShom &); 
GShom operator&(const GShom &,const GShom &); // composition
GShom operator*(const GSDD &,const GShom &); 
GShom operator*(const GShom &,const GSDD &); 
GShom operator^(const GSDD &,const GShom &); 
GShom operator^(const GShom &,const GSDD &); 
GShom operator-(const GShom &,const GSDD &); 


class Shom:public GShom{
public:
  /* Constructor */
  Shom(const GShom &h=GShom::id);
  Shom(const Shom &h);
  Shom(const GSDD& d);   // constant
  Shom(int var,const DataSet& val, const GShom &h=GShom::id);  // var -- val -> Id
  ~Shom();

  /* Set */
  
  Shom &operator=(const GShom &);
  Shom &operator=(const Shom &);
};

/******************************************************************************/
namespace __gnu_cxx {
  template<>  struct hash<GShom> {
    size_t operator()(const GShom &g) const{
      return (size_t) g.concret;
    }
  };
}

namespace std {
  template<>  struct equal_to<GShom> {
    bool operator()(const GShom &g1,const GShom &g2) const{
      return g1==g2;
    }
  };
}

namespace std {
  template<>  struct less<GShom> {
    bool operator()(const GShom &g1,const GShom &g2) const{
      return g1<g2;
    }
  };
}

/**********************************************************************/
class _GShom{
private:
  friend class GShom;
  friend class Shom;
  mutable int refCounter;
  mutable bool marking;
  mutable bool immediat;
public:
#ifdef INST_STL
  static MapJumps HomJumps;
#endif
  /* Destructor*/
  _GShom(int ref=0,bool im=false):refCounter(ref),marking(false),immediat(im){};
  virtual ~_GShom(){};

  /* Compare */
  virtual bool operator==(const _GShom &h) const=0;
  virtual size_t hash() const=0;

  /* Eval */
  virtual GSDD eval(const GSDD &)const=0;

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

class StrongShom:public _GShom{
public:
  StrongShom(){};
  virtual ~StrongShom(){};
  virtual GSDD phiOne() const{return GSDD::top;}; 
  virtual GShom phi(int var,const DataSet& val) const=0; 
  virtual bool operator==(const StrongShom &h) const=0;
  bool operator==(const _GShom &h) const;

  //Eval
  GSDD eval(const GSDD &)const;  
};
 
#endif /* SHOM_H_ */
