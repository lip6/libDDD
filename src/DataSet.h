#ifndef __DATASET_H__
#define __DATASET_H__

#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <cassert>
using namespace std;


// Concrete DataSet classes should derive from DataSet and fulfill the contract FOR THEIR OWN TYPE
// hard or dynamic casting the argument into one's own type is the recommended behavior 
class DataSet {
 public :
  // destructor
  virtual ~DataSet() {};
  // returns a new instance copy of this 
  virtual DataSet *newcopy () const = 0;
  // returns a new instance with elements = this inter b
  virtual DataSet *set_intersect (const DataSet & b) const  = 0;
  // returns a new instance with elements = this union b
  virtual DataSet *set_union (const DataSet & b)  const = 0;
  // returns a new instance with elements = this setminus b
  virtual DataSet *set_minus (const DataSet & b) const = 0;
  // returns true if this is the empty set
  virtual bool empty() const = 0;
  // returns a pointer to an instance of the empty set
  virtual DataSet *empty_set() const = 0;
  // compares two sets for equality
  virtual bool set_equal(const DataSet & b) const =0;
  // returns the size (number of elements) in a set
  virtual size_t set_size() const = 0;
  // returns a hash function, used in the SDD hash function computation
  virtual size_t set_hash() const =0;
  // returns a formatted string description of the set
  virtual void set_print (ostream &os) const =0;
};


#endif
