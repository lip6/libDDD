#ifndef __DATASET_H__
#define __DATASET_H__

#include <iosfwd>

/// This class is an abstraction of a set of data.
/// Set Decision Diagrams SDD arcs are labeled by a DataSet *, canonization of SDD requires
/// a set-based interface (union, intersection, set difference), ability to compute a hash key
/// and test two sets for equality for unicity table purposes, and test for emptiness as SDD are 
/// both zero suppresed (no path lead to GSDD::null), and empty-set suppressed (no arc labeled by
/// empty_set is represented)
///
/// Additional interface is provided to query/examine the structure, in particular set_size to 
/// is required to compute the full set size of an SDD, and print (although this last is not essential)
///  
/// Concrete DataSet classes should derive from DataSet and fulfill the contract FOR THEIR OWN TYPE
/// hard or dynamic casting the argument into one's own type is the recommended behavior 
///
/// \todo recent experiments with V. Beaudenon show maybe some behavior should be put here, 
/// for instance set_intersect is always empty if incompatible types are compared.
class DataSet 
{
 public :
  /// destructor
  virtual ~DataSet() {};
  /// returns a new instance copy of this 
  virtual DataSet *newcopy () const = 0;
  /// returns a new instance with elements = this inter b
  virtual DataSet *set_intersect (const DataSet & b) const  = 0;
  /// returns a new instance with elements = this union b
  virtual DataSet *set_union (const DataSet & b)  const = 0;
  /// returns a new instance with elements = this setminus b
  virtual DataSet *set_minus (const DataSet & b) const = 0;
  /// returns true if this is the empty set
  virtual bool empty() const = 0;
  /// returns a pointer to an instance of the empty set
  virtual DataSet *empty_set() const = 0;
  /// Compares two sets for equality.
  virtual bool set_equal(const DataSet & b) const =0;
  /// \return the size (number of elements) in a set
  virtual long double set_size() const = 0;
  /// returns a hash function, used in the SDD hash function computation
  virtual size_t set_hash() const =0;
  /// returns a formatted string description of the set
  virtual void set_print (std::ostream &os) const =0;
#ifdef EVDDD
  virtual DataSet *normalizeDistance(int n) const =0;
  virtual int getMinDistance() const = 0;
#endif

};


#endif
