/* -*- C++ -*- */
#ifndef SDD_H
#define SDD_H

#include <iosfwd>
#include <string>
#include <ext/hash_set>

#include "DataSet.h"
#include "hashfunc.hh"

/// pre-declaration of concrete (private) class implemented in .cpp file
class _GSDD;

/******************************************************************************/
/// This class is the base class representing a hierarchical Set Decision Diagram.
/// It is composed of a set of arcs labeled by sets of values (DataSet in fact) 
/// that point to successor GSDD nodes.
/// This class does not implement reference counting : 
/// GSDD are destroyed on MemoryManager::Garbage
/// unless they are also referenced as SDD.
/// Note that this class is in fact a kind of smart pointer : operations are delegated on "concret"
/// the true implementation class (of private hidden type _GSDD) that contains the data and has a single 
/// memory occurrence thanks to the unicity table.
class GSDD {
private:
  /// Open access to hash function computation procedure.
  friend struct __gnu_cxx::hash<GSDD>;
  /// A textual output. 
  /// Don't use it with large number of paths as each element is printed on a different line
  friend std::ostream& operator<<(std::ostream &os,const GSDD &g);
  /// Open access to concret for reference counting in DDD.
  friend class SDD;
  /// open access to internal implementation class.
  friend class _GSDD;
  /// The real implementation class. All true operations are delagated on this pointer.
  /// Construction/destruction take care of ensuring concret is only instantiated once in memory.
  _GSDD *concret;
  /// Internal function used in recursion for textual printing of GDDD.
  void print(std::ostream& os,std::string s) const;
public:
    /// \name Public Accessors 
  //@{
  /// To hide how arcs are actually stored. Use GSDD::Valuation to refer to arcs type
  typedef std::vector<std::pair<DataSet *,GSDD> > Valuation;
  /// To hide how arcs are stored. Also for more compact expressions : 
  /// use GDDD::const_iterator to iterate over the arcs of a DDD
  typedef Valuation::const_iterator const_iterator;
  /// Returns a node's variable.
  int variable() const;
  /// API for iterating over the arcs of a DDD manually. 
  /// i.e. not using a predefined evaluation mechanism such as StrongHom
  ///
  /// for (GDDD::const_iterator it = a_gddd.begin() ; it != a_gddd.end() ; it++ ) { // do something }
  ///
  /// returns the first arc
  const_iterator begin() const;
  /// API for iterating over the arcs of a DDD manually. 
  /// i.e. not using a predefined evaluation mechanism such as StrongHom
  /// 
  /// for (GDDD::const_iterator it = a_gddd.begin() ; it != a_gddd.end() ; it++ ) { // do something }
  ///
  /// returns a past the end iterator
  const_iterator end() const;

  //@}



  
  /* Constructeurs */
  /// \name Public Constructors 
  //@{
  /// Construct a GSDD with arguments given.
  /// \param variable the variable labeling the node
  /// \param value the outgoing arc of the node
  GSDD(int variable,Valuation value);
  /// Default constructor creates the empty set SDD.
  GSDD():concret(null.concret){};
  /// The most common way for the user of creating SDD.
  /// This constructor builds a node with a single arc of the form var-val->d.
  /// Usually a user will create these single path SDD, possibly by imbrication as in
  /// GSDD(var1, val1, GSDD( var2, val2 )). Then compose them using +, -, *, ^ ...
  /// \param var the variable labeling the node
  /// \param val the value labeling the arc
  /// \param d the successor node or defaults to terminal GSDD::one if none provided  
  GSDD(int var,const DataSet & val,const GSDD &d=one ); //var-val->d
  /// Another common way for the user to create SDD.
  /// This constructor builds a node with a single arc of the form var-val->d.
  /// This adapted version is useful when the arc value is itself the result of Hom applications
  /// as the compiler needs the change from GSDD to SDD type to recognize a DataSet
  /// \param var the variable labeling the node
  /// \param val the value labeling the arc
  /// \param d the successor node or defaults to terminal GSDD::one if none provided  
  GSDD(int var,const GSDD & val,const GSDD &d=one ); //var-val->d
  /// A should be \e private constructor used in internals, DO NOT USE THIS. 
  /// \param _g The pointer provided should point into the unicity table
  /// \todo make this private
  GSDD(_GSDD *_g); 
#ifdef OTF_GARBAGE
  /// In OTF garbage context, reference count maintaining is necessary for GSDD.
  /// Copy constructor with refcount.
  GSDD(const GSDD &);
  /// In OTF garbage context, reference count maintaining is necessary for GSDD.
  /// Assignment constructor with refcount.
  GSDD & operator=(const GSDD &);
#endif
 //@}

  /* Constants */
  /// \name Terminal nodes defined as constants 
  //@{
  /// The accepting terminal. This is the basic leaf for accepted sequences.
  static const GSDD one;
  /// The non-accepting terminal. As DDD are a zero-suppressed variant of decision diagrams,
  /// paths leading to null are suppressed. Only the empty set is represented by null.
  static const GSDD null;
  /// The approximation terminal. This represents *any* finite set of assignment sequences. 
  /// In a "normal" usage pattern, top terminals should not be produced.
  /// In fact the result may be incorrect if Top is encountered at any but the highest level.
  static const GSDD top;
  //@}


#ifdef OTF_GARBAGE
  /// In OTF garbage context, reference count must be maintained for GSDD.
  virtual ~GSDD () ;
#endif
  /* Compare */
 /// \name Comparisons for hash and map storage
  //@{
  /// Comparison between DDD. Note that comparison is based on "concret" address in unicity table.
  /// \param g the node to compare to
  /// \return true if the nodes are equal.
  bool operator==(const GSDD& g) const{return concret==g.concret;};
  /// Comparison between DDD. Note that comparison is based on "concret" address in unicity table.
  /// \param g the node to compare to
  /// \return true if the nodes are not equal.
  bool operator!=(const GSDD& g) const{return concret!=g.concret;};
  /// Total ordering function between DDD. Note that comparison is based on "concret" address in unicity table.
  /// This ordering is necessary for hash and map storage of GDDD.
  /// \param g the node to compare to
  /// \return true if argument g is greater than "this" node.
  bool operator<(const GSDD& g) const{return concret<g.concret;};
  //@}

  /* Visualisation */ 
  /* Accessors */ 
  /// Returns current reference count of a node.
  /// Reference count corresponds to the number of SDD that use a given concrete node.
  /// No recursive reference counting is used : son nodes may have refCount=0 even if this node has a positive refCounter.
  unsigned int refCounter() const;
  /// Returns the size in number of nodes of a SDD structure.
  unsigned long int size() const;
  // Returns the size in number of nodes of a SDD + DDD mixed structure. 
  // \return a pair <number of SDD nodes,number of DDD nodes>
  std::pair<unsigned long int,unsigned long int> node_size() const;
  /// Returns the number of successors of a given node. This is the size of the arc array of the node.
  size_t nbsons () const;
  /// Returns the number of states or paths represented by a given node.
  long double nbStates() const;
#ifdef EVDDD
  /// returns the minimum value of the function encoded by a node
  int getMinDistance () const;
  GSDD normalizeDistance (int n) const;
#endif

  ///  Broken right now, dont use me or fixme first
  /// Returns the number of nodes that would be used to represent a SDD if no unicity table was used.
  //  long double GSDD::noSharedSize() const;


  /* Memory Manager */
  /// \name Memory Management 
  //@{
  /// Returns unicity table current size. Gives the number of different nodes created and not yet destroyed.
  static  unsigned int statistics();
  /// For garbage collection internals. Marks a GSDD as in use in garbage collection phase. 
  /// 
  void mark()const;
  /// For garbage collection, do not call this directly, use MemoryManager::garbage() instead.
  /// \todo describe garbage collection algorithm(s) + mark usage homogeneously in one place.
  static void garbage();
  /// Prints some statistics to std::cout. Mostly used in debug and development phase.
  /// See also MemoryManager::pstats().
  /// \todo allow output in other place than cout. Clean up output.
  static void pstats(bool reinit=true);
  /// Returns the peak size of the DDD unicity table. This value is maintained up to date upon GarbageCollection.
  static size_t peak();

#ifdef OTF_GARBAGE
  /// To check out useless intermediate nodes : on the fly garbage collection.
  /// WARNING : this is not const at all in fact it deletes "concret" and is quite dangerous,
  /// only for expert use not a basic library functionality
  void clearNode() const;
  /// During on the fly garbage collection, nodes marked as son are preserved.
  void markAsSon() const;
  /// During on the fly garbage collection, nodes marked as son are preserved.
  bool isSon() const;
#endif // OTF_GARBAGE
  //@}


};


/// Textual output of SDD into a stream in (relatively) human readable format.
std::ostream& operator<<(std::ostream &,const GSDD &);
/* Binary operators */
/// Operator for union of SDD. See SDD operations documentation section for details.
/// \todo Write the DDD operations documentation !!
GSDD operator^(const GSDD&,const GSDD&); // concatenation
/// Operator for union of DDD. See DDD operations documentation section for details.
/// \todo Write the DDD operations documentation !!
GSDD operator+(const GSDD&,const GSDD&); // union
/// Operator for intersection of DDD. See DDD operations documentation section for details.
/// \todo Write the DDD operations documentation !!
GSDD operator*(const GSDD&,const GSDD&); // intersection
/// Operator for set difference of DDD. See DDD operations documentation section for details.
/// \todo Write the DDD operations documentation !!
GSDD operator-(const GSDD&,const GSDD&); // difference

 
/******************************************************************************/
/// This class is the public interface for manipulating Data Decision Diagrams.
/// Except when defining new homomorphisms, a user of the library should only 
/// manipulate SDD, not GSDD.
/// Reference counting is enabled for SDD, so they will not be destroyed if they 
/// are still in use upon garbage collection.
class SDD:public GSDD,public DataSet {
public:
  /* Constructeur */
  /// Copy constructor. Constructs a copy, actual data (concret) is not copied.
  /// RefCounter is updated however.
  SDD(const SDD &);
  /// Copy constructor from base class GDDD, also default DDD constructor to empty set. 
  /// Increments refCounter of g.concret.
  SDD(const GSDD &g=GSDD::null);
  /// The most common way for the user of creating DDD.
  /// This constructor builds a node with a single arc of the form var-val->d.
  /// Usually a user will create these single path DDD, possibly by imbrication as in
  /// SDD(var1, val1, SDD( var2, val2 )). Then compose them using +, -, *, ^ ...
  /// See also GSDD(var,val,d).
  /// \param var the variable labeling the node
  /// \param val the value labeling the arc
  /// \param d the successor node or defaults to terminal GDDD::one if none provided
  SDD(int var,const DataSet& val,const GSDD &d=one ); //var-val->d
  SDD(int var,const GSDD& val,const GSDD &d=one ); //var-val->d
  /// Destructor, maintains refCount. Note that destroying a DDD does not actually destroy
  /// any data, it decrements reference count, so that subsequent MemoryManager::garbage call
  /// may truly clear the data.
  virtual ~SDD(); 

  /* Set */
  ///\name Assignment operators.
  //@{
  /// Overloaded behavior for assignment operator, maintains reference counting.
  SDD &operator=(const GSDD&);
  /// Overloaded behavior for assignment operator, maintains reference counting.
  SDD &operator=(const SDD&);
 //@}

  /// \name DataSet implementation interface 
  /// This is the implementation of the DataSet class interface used in SDD context.
  /// These functions allow to reference SDD from SDD arcs.
  /// \e IMPORTANT Remember to delete returned values after use. 
  ///
  ///  Note these functions are not resistant to incompatible DataSet types. 
  ///  When these functions have a parameter "b", it should be a reference to a SDD from proper behavior.
  //@{  
  // DataSet interface
  /// Return a new copy of a SDD. 
  virtual DataSet *newcopy () const { return new SDD(*this); }
  /// Compute intersection of two SDD. 
  virtual DataSet *set_intersect (const DataSet & b) const ;
  /// Compute union of two SDD. 
  virtual DataSet *set_union (const DataSet & b)  const;
  /// Compute set difference of two SDD. 
  virtual DataSet *set_minus (const DataSet & b) const;
  /// Return true if this is the empty set.
  virtual bool empty() const;
  /// Returns a pointer to  GSDD::null.
  virtual DataSet *empty_set() const;
  /// Compares to DataSet for equality.
  virtual bool set_equal(const DataSet & b) const;
  /// Compares to DataSet for equality.
  virtual long double set_size() const;
  /// Returns a hash key for the SDD.
  virtual size_t set_hash() const;
  /// Textual (human readable) output of a SDD.
  virtual void set_print (std::ostream &os) const { os << *this; }
#ifdef EVDDD
  virtual DataSet *normalizeDistance(int n) const { return new SDD(GSDD::normalizeDistance(n)); }
  virtual int getMinDistance() const { return GSDD::getMinDistance();}
#endif

  //@}

};

/// Namespace declared to hide these functions. 
/// It is not very nice to access unicity table directly, these functions were exposed to allow graphical dot export of the unicity table contents.
#include "UniqueTable.h"

namespace SDDutil {
#ifdef OTF_GARBAGE
  /// The function used in OTF garbage context to clear up part of the intermediate nodes.
  void recentGarbage();
#endif // OTF_GARBAGE
  /// accessor to UniqueTable instance declared in cpp file, (hem, please don't touch it). 
  /// \todo implement nice generic dot export and eliminate this.
  UniqueTable<_GSDD> * getTable ();
  /// Iterator over the entries of the table, applies foo to each entry in the table.
  /// This was declared for dot export, I do not think it is very useful in general.
  void foreachTable (void (*foo) (const GSDD & g)); 
}


/******************************************************************************/
namespace __gnu_cxx {
  /// Computes a hash key for an SDD. 
  /// Value returned is based on unicity of concret in unicity table.
  /// Uses D. Knuth's hash function for pointers.
  template<>	
  struct hash<GSDD> {
    size_t operator()(const GSDD &g) const{
      //return (size_t) g.concret;
      return ddd::knuth32_hash(reinterpret_cast<const size_t>(g.concret));
    }
  };
}

namespace std {
  /// Compares two SDD in hash tables. 
  /// Value returned is based on unicity of concret in unicity table.
  template<>
  struct equal_to<GSDD> {
    bool operator()(const GSDD &g1,const GSDD &g2) const{
      return g1==g2;
    }
  };
}

namespace std {
  /// Compares two SDD in hash tables. 
  /// Value returned is based on unicity of concret in unicity table.
  template<>
  struct less<GSDD> {
    bool operator()(const GSDD &g1,const GSDD &g2) const{
      return g1<g2;
    }
  };
}




#endif


