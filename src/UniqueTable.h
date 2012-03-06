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
#ifndef UNIQUETABLE_H
#define UNIQUETABLE_H

#include <cassert>
#include <vector>
#include "util/hash_set.hh"
#include "util/hash_support.hh"


#ifdef REENTRANT
#include "tbb/queuing_mutex.h"
#include "tbb/mutex.h"
#endif

// the clone contract
namespace unique {

template<typename T>
struct clone
{
  T*
  operator()(const T& e1) const
  {
    return e1.clone();
  }
};

template<typename T>
struct clone<T*>
{
  T*
  operator()(const T* e1) const
  {
    return e1->clone();
  }
};
  
template<>
struct clone<std::vector<int> >
{
  std::vector<int>*
  operator()(const std::vector<int>& e1) const
     {
       return new std::vector<int>(e1);
     }
};
 
}


/// This class implements a unicity table mechanism, based on an STL hash_set.
/// Requirements on the contained type are thus those of hash_set.
template<typename T>
class UniqueTable{

private:
	
#ifdef REENTRANT
  // typedef tbb::queuing_mutex table_mutex_t;
  typedef tbb::mutex table_mutex_t;
  table_mutex_t table_mutex_;
#endif

public:
  /// Constructor, builds a default table.
  UniqueTable()
#ifdef REENTRANT
    :
    table_mutex_()
#endif
  {
#ifndef REENTRANT
#ifndef USE_STD_HASH
    table.set_deleted_key(NULL);
#endif
#endif
  }

  /// Typedef helps hide implementation type (currently gnu gcc's hash_set).
    typedef typename d3::hash_set<const T*>::type  Table;
  /// The actual table, operations on the UniqueTable are delegated on this.
  Table table; // Unique table of GDDD

/* Canonical */
  /// The application operator, returns the address of the value already in 
  /// the table if it exists, or inserts and returns the address of the value inserted.
  /// \param _g the pointer to the value we want to find in the table.
  /// \return the address of an object stored in the UniqueTable such that (*_g == *return_value)
  const T*
    operator()(const T &_g)
  {
#ifdef REENTRANT
    table_mutex_t::scoped_lock lock(table_mutex_);
#endif

    typename Table::const_iterator it = table.find(&_g); 
    if (it != table.end() ) {
      return *it;
    } else {
      T * clone = unique::clone<T>() (_g);
      std::pair<typename Table::iterator, bool> ref=table.insert(clone); 
      assert(ref.second);
      ((void)ref);   
      return clone;
    }
  }

  /// Returns the current number of filled entries in the table.
  size_t
  size() const
  {
    return table.size();
  }
  
#ifdef HASH_STAT
  std::map<std::string, size_t> get_hits() const { return table.get_hits(); }
  std::map<std::string, size_t> get_misses() const { return table.get_misses(); }
  std::map<std::string, size_t> get_bounces() const { return table.get_bounces(); }
#endif // HASH_STAT
};

#endif
