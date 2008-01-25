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
#ifndef DDD_H
#define DDD_H

#include <iosfwd>
#include <string>
#include <vector>
#include <ext/hash_set>

#include "DataSet.h"
#include "hashfunc.hh"

/// pre-declaration of concrete (private) class implemented in .cpp file
class _GDDD;

/// pre-declaration of DDD for use in std::vector<DDD> before DDD declaration (g++ 4)
class DDD;

/// This class is the base class representing a Data Decision Diagram.
/// It is composed of a set of arcs labeled by integers that point to successor GDDD nodes.
/// This class does not implement reference counting : 
/// GDDD are destroyed on MemoryManager::Garbage
/// unless they are also referenced as DDD.
/// Note that this class is in fact a kind of smart pointer : operations are delegated on "concret"
/// the true implementation class (of private hidden type _GDDD) that contains the data and has a single 
/// memory occurrence thanks to the unicity table.
class GDDD 
{
private:
  /// A textual output. 
  /// Don't use it with large number of paths as each element is printed on a different line
  friend std::ostream& operator<<(std::ostream &os,const GDDD &g);
  /// Open access to concret for reference counting in DDD.
  friend class DDD;
  /// The real implementation class. All true operations are delagated on this pointer.
  /// Construction/destruction take care of ensuring concret is only instantiated once in memory.
  _GDDD *concret;
  /// A private constructor used in internals. 
  /// \param _g The pointer provided should point into the unicity table
  GDDD(_GDDD *_g);
  /// Internal function used in recursion for textual printing of GDDD.
  void print(std::ostream& os,std::string s) const;
  /// A function for DDD serialization (beta).
  void saveNode(std::ostream&, std::vector<_GDDD*>& )const;
  /// Another function used in serialization.
  unsigned long int nodeIndex(std::vector<_GDDD*>)const;
public:
  /// \name Public Accessors 
  //@{
  /// To hide how arcs are actually stored. Use GDDD::Valuation to refer to arcs type
  typedef std::vector<std::pair<int,GDDD> > Valuation;
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

  /// \name Public Constructors 
  //@{
  /// Construct a GDDD with arguments given.
  /// \todo why is this public ???
  /// \e WARNING Valuation should be sorted according to arc values
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
  //@}


  /// \name Terminal nodes defined as constants 
  //@{
  /// The accepting terminal. This is the basic leaf for accepted sequences.
  static const GDDD one;
  /// The non-accepting terminal. As DDD are a zero-suppressed variant of decision diagrams,
  /// paths leading to null are suppressed. Only the empty set is represented by null.
  static const GDDD null;
  /// The approximation terminal. This represents *any* finite set of assignment sequences. 
  /// In a "normal" usage pattern, top terminals should not be produced.
  static const GDDD top;
  //@}

  /// \name Comparisons for hash and map storage
  //@{
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
  //@}

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
#ifdef EVDDD
  /// returns the minimum value of the function encoded by a node
  int getMinDistance () const;
  GDDD normalizeDistance (int n) const;
#endif


  /// \name Variable naming.
  //@{
  /// Sets a variable's name. 
  /// \todo This function should be implemented in a name manager somewhere so that it is common to DDD and SDD variables.
  /// \param var the index of the variable to be named
  /// \param name the name to attach to this variable index
  static void varName( int var, const std::string& name );
  /// Gets a variable's name. 
  /// \todo This function should be implemented in a name manager somewhere so that it is common to DDD and SDD variables.
  /// \param var the index of the variable to be named
  /// \return the name attached to this variable index
  static const std::string getvarName( int var );
  //@}

  /// \name Memory Management 
  //@{
  /// Returns unicity table current size. Gives the number of different nodes created and not yet destroyed.
  static  unsigned int statistics();
  /// For garbage collection internals. Marks a GDDD as in use in garbage collection phase. 
  /// 
  void mark() const;
  /// For storage in a hash table
  size_t hash () const { 
    return ddd::knuth32_hash(reinterpret_cast<const size_t>(concret)); 
  }
  /// For garbage collection, do not call this directly, use MemoryManager::garbage() instead.
  /// \todo describe garbage collection algorithm(s) + mark usage homogeneously in one place.
  static void garbage(); 
  /// Prints some statistics to std::cout. Mostly used in debug and development phase.
  /// See also MemoryManager::pstats().
  /// \todo allow output in other place than cout. Clean up output.
  static void pstats(bool reinit=true);
  /// Returns the peak size of the DDD unicity table. This value is maintained up to date upon GarbageCollection.
  static size_t peak();
  //@}
  /// \name Serialization functions.
  //@{
  /// Function for serialization. Save a set of DDD to a stream.
  friend void saveDDD(std::ostream&, std::vector<DDD>);
  /// Function for deserialization. Load a set of DDD from a stream.
  friend void loadDDD(std::istream&, std::vector<DDD>&);
  //@}
};


/// Textual output of DDD into a stream in (relatively) human readable format.
std::ostream& operator<<(std::ostream &,const GDDD &);
/* Binary operators */
/// Operator for concatenation of DDD. See DDD operations documentation section for details.
/// \todo Write the DDD operations documentation !!
GDDD operator^(const GDDD&,const GDDD&); 
/// Operator for union of DDD. See DDD operations documentation section for details.
/// \todo Write the DDD operations documentation !!
GDDD operator+(const GDDD&,const GDDD&);
/// Operator for intersection of DDD. See DDD operations documentation section for details.
/// \todo Write the DDD operations documentation !!
GDDD operator*(const GDDD&,const GDDD&); 
/// Operator for set difference of DDD. See DDD operations documentation section for details.
/// \todo Write the DDD operations documentation !!
GDDD operator-(const GDDD&,const GDDD&); 


 
/// This class is the public interface for manipulating Data Decision Diagrams.
/// Except when defining new homomorphisms, a user of the library should only 
/// manipulate DDD, not GDDD.
/// Reference counting is enabled for DDD, so they will not be destroyed if they 
/// are still in use upon garbage collection.
class DDD : public GDDD, public DataSet 
{
public:
  /* Constructors */
  /// Copy constructor. Constructs a copy, actual data (concret) is not copied.
  /// RefCounter is updated however.
  DDD(const DDD &);
  /// Copy constructor from base class GDDD, also default DDD constructor to empty set. 
  /// Increments refCounter of g.concret.
  DDD(const GDDD &g=GDDD::null);

  /// The most common way for the user of creating DDD.
  /// This constructor builds a node with a single arc of the form var-val->d.
  /// Usually a user will create these single path DDD, possibly by imbrication as in
  /// DDD(var1, val1, DDD( var2, val2 )). Then compose them using +, -, *, ^ ...
  /// See also GDDD(var,val,d).
  /// \param var the variable labeling the node
  /// \param val the value labeling the arc
  /// \param d the successor node or defaults to terminal GDDD::one if none provided
  DDD(int var,int val,const GDDD &d=one ); 
  /// To create a DDD with arcs covering a range of values.
  /// This interface creates nodes with a set of arcs bearing the values in the interval 
  /// [val1,var2] that point to the same successor node d.
  /// \param var the variable labeling the node
  /// \param val1 lowest value labeling an arc
  /// \param val2 highest value labeling an arc
  /// \param d the successor node or defaults to terminal GDDD::one if none provided  
  DDD(int var,int val1,int val2,const GDDD &d=one); 
  /// Destructor, maintains refCount. Note that destroying a DDD does not actually destroy
  /// any data, it decrements reference count, so that subsequent MemoryManager::garbage call
  /// may truly clear the data.
  ~DDD(); 

  ///\name Assignment operators.
  //@{
  /// Overloaded behavior for assignment operator, maintains reference counting.
  DDD &operator=(const GDDD&);
  /// Overloaded behavior for assignment operator, maintains reference counting.
  DDD &operator=(const DDD&);
  //@}

  /// \name DataSet implementation interface 
  /// This is the implementation of the DataSet class interface used in SDD context.
  /// These functions allow to reference DDD from SDD arcs.
  /// \e IMPORTANT Remember to delete returned values after use. 
  ///
  ///  Note these functions are not resistant to incompatible DataSet types. 
  ///  When these functions have a parameter "b", it should be a reference to a DDD from proper behavior.
  //@{  
  
  /// Return a new copy of a DDD. 
  virtual DataSet *newcopy () const { return new DDD(*this); }
  /// Compute intersection of two DDD. 
  virtual DataSet *set_intersect (const DataSet & b) const ;
  /// Compute union of two DDD. 
  virtual DataSet *set_union (const DataSet & b)  const;
  /// Compute set difference of two DDD. 
  virtual DataSet *set_minus (const DataSet & b) const;
  /// Return true if this is the empty set.
  virtual bool empty() const;
  /// Returns a pointer to  GDDD::null.
  virtual DataSet *empty_set() const;
  /// Compares to DataSet for equality.
  virtual bool set_equal(const DataSet & b) const;
  /// Compares to DataSet for equality.
  virtual long double set_size() const;
  /// Returns a hash key for the DDD.
  virtual size_t set_hash() const;
  /// Textual (human readable) output of a DDD.
  virtual void set_print (std::ostream &os) const { os << *this; }
#ifdef EVDDD
  virtual DataSet *normalizeDistance(int n) const { return new DDD(GDDD::normalizeDistance(n)); }
  virtual int getMinDistance() const { return GDDD::getMinDistance();}
#endif

  //@}
};

/******************************************************************************/
namespace __gnu_cxx {
  /// Computes a hash key for a DDD. 
  /// Value returned is based on unicity of concret in unicity table.
  /// Uses D. Knuth's hash function for pointers.  
  template<>
  struct hash<GDDD> {
    size_t operator()(const GDDD &g) const{
      //return (size_t) g.concret;
      return g.hash(); 
    }
  };
}

namespace std {
  /// Compares two DDD in hash tables. 
  /// Value returned is based on unicity of concret in unicity table.
  template<>
  struct equal_to<GDDD> {
    bool operator()(const GDDD &g1,const GDDD &g2) const{
      return g1==g2;
    }
  };
}

namespace std {
  /// Compares two DDD in hash tables. 
  /// Value returned is based on unicity of concret in unicity table.
  template<>
  struct less<GDDD> {
    bool operator()(const GDDD &g1,const GDDD &g2) const{
      return g1<g2;
    }
  };
}


#ifdef EVDDD
// the variable id of distance nodes
#define DISTANCE -3
#endif


#endif






