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
