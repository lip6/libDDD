/* -*- C++ -*- */
#ifndef DDD_H
#define DDD_H
#include <string>
#include <iostream>
#include <vector>
#include <set>

#include <ext/hash_map>

using namespace std;
using namespace __gnu_cxx;

#include "DataSet.h"

/// pre-declaration of concrete (private) class implemented in .cpp file
class _GDDD;

/// This class is the base class representing a Data Decision Diagram
/// It is composed of a set of arcs labeled by integers that point to successor GDDD nodes
/// This class does not implement reference counting : GDDD are destroyed on MemoryManager::Garbage
/// unless they are also referenced as DDD.
/// Note that this class is in fact a kind of smart pointer : operations are delegated on "concret"
/// the true implementation class (of private type _GDDD) that contains the data and has a single 
/// memory occurrence thanks to the unicity table.
class GDDD 
{
private:
  /// To store in unicity tables and cache 
  friend struct hash<GDDD>;
  /// A textual output. 
  /// Don't use it with large number of paths as each element is printed on a different line
  friend ostream& operator<<(ostream &os,const GDDD &g);
  /// open access to concret for reference counting in DDD.
  friend class DDD;
  /// The real implementation class. All true operations are delagated on this pointer.
  /// Construction/destruction take care of ensuring concret is only instantiated once in memory.
  _GDDD *concret;
  /// A private constructor used in internals. 
  /// \param _g The pointer provided should point into the unicity table
  GDDD(_GDDD *_g);
  /// Internal function used in recursion for textual printing of GDDD.
  void print(ostream& os,string s) const;
  /// A function for DDD serialization (beta).
  void saveNode(ostream&, vector<_GDDD*>& )const;
  /// Another function used in serialization.
  unsigned long int nodeIndex(vector<_GDDD*>)const;
public:
  /* Accessors */
  /// To hide how arcs are actually stored. Use GDDD::Valuation to refer to arcs type
  typedef vector<pair<int,GDDD> > Valuation;
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
  
  /* Constructors */
  /// Construct a GDDD with arguments given.
  /// \todo why is this public ???
  /// \warn Valuation should be sorted according to arc values
  /// \param variable the variable labeling the node
  /// \param value the set of arcs of the node
  GDDD(int variable,Valuation value);
  /// Default constructor creates the empty set DDD.
  GDDD():concret(null.concret){};
  /// The most common way for the user of creating DDD.
  /// This constructor builds a node with a single arc of the form var-val->d.
  /// Usually a user will create these single path DDD, possibly by imbrication as in
  /// GDDD(var1, val1, GDDD( var2, val2 )). Then compose them using +, -, *, ^ ...
  /// \param var the variable labeling the node
  /// \param val the value labeling the arc
  /// \param d the successor node or defaults to terminal GDDD::one if none provided
  GDDD(int var,int val,const GDDD &d=one );
  /// To create a DDD with arcs covering a range of values.
  /// This interface creates nodes with a set of arcs bearing the values in the interval 
  /// [val1,var2] that point to the same successor node d.
  /// \param var the variable labeling the node
  /// \param val1 lowest value labeling an arc
  /// \param val2 highest value labeling an arc
  /// \param d the successor node or defaults to terminal GDDD::one if none provided
  GDDD(int var,int val1,int val2,const GDDD &d=one); //var-[val1,var2]->d

  /* Constants */
  /// The accepting terminal. This is the basic leaf for accepted sequences.
  static const GDDD one;
  /// The non-accepting terminal. As DDD are a zero-suppressed variant of decision diagrams,
  /// paths leading to null are suppressed. Only the empty set is represented by null.
  static const GDDD null;
  /// The approximation terminal. This represents *any* finite set of assignment sequences. 
  /// In a "normal" usage pattern, top terminals should not be produced.
  static const GDDD top;

  /* Compare */
  /// Comparison between DDD. Note that comparison is based on "concret" address in unicity table.
  /// \param g the node to compare to
  /// \return true if the nodes are equal.
  bool operator==(const GDDD& g) const{return concret==g.concret;};
  /// Comparison between DDD. Note that comparison is based on "concret" address in unicity table.
  /// \param g the node to compare to
  /// \return true if the nodes are not equal.
  bool operator!=(const GDDD& g) const{return concret!=g.concret;};
  /// Total ordering function between DDD. Note that comparison is based on "concret" address in unicity table.
  /// This ordering is necessary for hash and map storage of GDDD.
  /// \param g the node to compare to
  /// \return true if argument g is greater than "this" node.
  bool operator<(const GDDD& g) const{return concret<g.concret;};

  /* Accessors */ 
  /// Returns current reference count of a node.
  /// Reference count corresponds to the number of DDD that use a given concrete node.
  /// No recursive reference counting is used : son nodes may have refCount=0 even if this node has a positive refCounter.
  unsigned int refCounter() const;
  /// Returns the size in number of nodes of a DDD structure.
  unsigned long int size() const;
  /// Returns the number of successors of a given node. This is the size of the arc array of the node.
  size_t nbsons () const;
  /// Returns the number of states or paths represented by a given node.
  long double nbStates() const;
  /// Returns the number of nodes that would be used to represent a DDD if no unicity table was used.
  long double noSharedSize() const;
  /// Sets a variable's name. 
  /// \todo This function should be implemented in a name manager somewhere so that it is common to DDD and SDD variables.
  /// \param var the index of the variable to be named
  /// \param name the name to attach to this variable index
  static void varName( int var, const string& name );
  /// Gets a variable's name. 
  /// \todo This function should be implemented in a name manager somewhere so that it is common to DDD and SDD variables.
  /// \param var the index of the variable to be named
  /// \return the name attached to this variable index
  static const string getvarName( int var );

  /* Memory Management */
  /// Returns unicity table current size. Gives the number of different nodes created and not yet destroyed.
  static  unsigned int statistics();
  /// For garbage collection. Marks a GDDD as in use in garbage collection phase. 
  void mark() const;
  static void garbage(); 
  static void pstats(bool reinit=true);
  static size_t peak();
  /// Function for serialization. Save a set of DDD to a stream.
  friend void saveDDD(ostream&, vector<DDD>);
  /// Function for deserialization. Load a set of DDD from a stream.
  friend void loadDDD(istream&, vector<DDD>&);
};

ostream& operator<<(ostream &,const GDDD &);
/* Binary operators */
GDDD operator^(const GDDD&,const GDDD&); // concatenation
GDDD operator+(const GDDD&,const GDDD&); // union
GDDD operator*(const GDDD&,const GDDD&); // intersection
GDDD operator-(const GDDD&,const GDDD&); // difference


 
/******************************************************************************/
class DDD:public GDDD,public DataSet {
public:
  /* Constructeur */
  DDD(const DDD &);
  DDD(const GDDD &g=GDDD::null);
  DDD(int var,int val,const GDDD &d=one ); //var-val->d
  DDD(int var,int val1,int val2,const GDDD &d=one); //var-[val1,var2]->d
  ~DDD(); 

  /* Set */
  DDD &operator=(const GDDD&);
  DDD &operator=(const DDD&);

  // DataSet interface
  virtual DataSet *newcopy () const { return new DDD(*this); }
  virtual DataSet *set_intersect (const DataSet & b) const  ;
  virtual DataSet *set_union (const DataSet & b)  const ;
  virtual DataSet *set_minus (const DataSet & b) const;
  virtual bool empty() const;
  virtual DataSet *empty_set()const;
  virtual bool set_equal(const DataSet & b) const;
  virtual long double set_size() const;
  virtual size_t set_hash() const;
  virtual void set_print (ostream &os) const { os << *this; }
};

/******************************************************************************/
namespace __gnu_cxx {
  template<>
	struct hash<GDDD> {
		size_t operator()(const GDDD &g) const{
			return (size_t) g.concret;
		}
	};
}

namespace std {
  template<>
	struct equal_to<GDDD> {
		bool operator()(const GDDD &g1,const GDDD &g2) const{
			return g1==g2;
		}
	};
}

namespace std {
  template<>
	struct less<GDDD> {
		bool operator()(const GDDD &g1,const GDDD &g2) const{
			return g1<g2;
		}
	};
}

#endif






