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

#ifndef __INT_DATASET_H__
#define __INT_DATASET_H__

#include <vector>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <iostream>

#include "DataSet.h"
#include "UniqueTable.h"

using std::vector;


// a hash function for vector<int>
namespace d3 { namespace util {
    /// Computes a hash key for a vector<int>. 
    template<>	
      struct hash<vector<int> *> {
	size_t operator()(const vector<int> *data) const{
	  size_t res=1;
	  for (vector<int>::const_iterator it= data->begin() ; it != data->end() ; it++) 
	    res += res*(*it)+2711;
	  return res;
	}
      };
  }} // namespaces


/// This class is a very basic implementation of DataSet interface 
/// based on std::vector<int> and a unicity table
class IntDataSet : public DataSet {
  static UniqueTable<vector<int> > canonical;
  
  static const vector<int> * empty_;
  
  const vector<int>* data;
  // private constructors
  IntDataSet (const vector<int>* ddata): data(ddata) {};
  
public :
  /// typedef IntDataSet::const_iterator
  typedef std::vector<int>::const_iterator const_iterator;

  /// read-only iterator interface
  const_iterator begin () const { return data->begin() ; }
  const_iterator end () const { return data->end() ; }


  /// public constructor from non unique vector<int>
  IntDataSet (const vector<int> & ddata) {
    vector<int> * tmp = new vector<int> (ddata);
    sort( tmp->begin() , tmp->end() );
    data = canonical( tmp );
  }
  /// public deafult constructor = empty set
  IntDataSet () {
    data = empty_ ;
  }
  
  /// destructor
  virtual ~IntDataSet() {};
  /// returns a new instance copy of this 
  DataSet *newcopy () const  {
    return new IntDataSet(data);
  }
  /// returns a new instance with elements = this inter b
  DataSet *set_intersect (const DataSet & b) const  {
    vector<int> res ;
    const vector<int>* bvec = ((const IntDataSet &) b).data;
    std::set_intersection(data->begin(), data->end(),bvec->begin(), bvec->end(),std::back_insert_iterator<vector<int> > (res));
    // trim
    vector<int>* trimres = new vector<int> (res);
    assert (trimres->size() == trimres->capacity());
    return new IntDataSet(canonical(trimres));	
  }
  /// returns a new instance with elements = this union b
  DataSet *set_union (const DataSet & b)  const {
    vector<int> res ;
    const vector<int>* bvec = ((const IntDataSet &) b).data;
    std::set_union(data->begin(), data->end(),bvec->begin(), bvec->end(),std::back_insert_iterator<vector<int> > (res));
    // trim
    vector<int>* trimres = new vector<int> (res);
    assert (trimres->size() == trimres->capacity());
    return new IntDataSet(canonical(trimres));	
  }
  /// returns a new instance with elements = this setminus b
  DataSet *set_minus (const DataSet & b) const {
    vector<int> res ;
    const vector<int>* bvec = ((const IntDataSet &) b).data;
    std::set_difference(data->begin(), data->end(),bvec->begin(), bvec->end(),std::back_insert_iterator<vector<int> > (res));
    // trim
    vector<int>* trimres = new vector<int> (res);
    assert (trimres->size() == trimres->capacity());
    return new IntDataSet(canonical(trimres));
  }
  
  /// returns true if this is the empty set
  bool empty() const {
    return data == empty_;
  }
  /// returns a pointer to an instance of the empty set
  DataSet *empty_set() const {
    return new IntDataSet();
  }
  /// Compares two sets for equality.
  bool set_equal(const DataSet & b) const {
    return data == ((const IntDataSet &) b).data;
  }
  /// \return the size (number of elements) in a set
  long double set_size() const {
    return data->size();
  }
  /// returns a hash function, used in the SDD hash function computation
  virtual size_t set_hash() const {
    return d3::util::hash<vector<int>* > () (data);
  }
  /// returns a formatted string description of the set
  virtual void set_print (std::ostream &os) const {
    os << "[" ;
    if (! data->empty() ) {
      std::copy( data->begin(), --data->end(),
		 std::ostream_iterator<int>(os, ",") );
      os << *(--data->end());
    }
    os << "]" ;
  }
#ifdef EVDDD
  DataSet *normalizeDistance(int n) const {
    return new IntDataSet(data);
  }
  int getMinDistance() const {
    return 0;
  }
#endif

};




#endif // __INT_DATASET_H__
