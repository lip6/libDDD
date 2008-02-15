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
#include <ext/hash_set>

#ifdef PARALLEL_DD
#include "tbb/queuing_mutex.h"
#include "tbb/mutex.h"
#endif

/// This class implements a unicity table mechanism, based on an STL hash_set.
/// Requirements on the contained type are thus those of hash_set.
template<typename T>
class UniqueTable{

private:
	
#ifdef PARALLEL_DD
  // typedef tbb::queuing_mutex table_mutex_t;
  typedef tbb::mutex table_mutex_t;
  table_mutex_t table_mutex_;
#endif

public:
  /// Constructor, builds a default table.
  UniqueTable()
#ifdef PARALLEL_DD
    :
    table_mutex_()
#endif
  {
  }

  /// Typedef helps hide implementation type (currently gnu gcc's hash_set).
  typedef __gnu_cxx::hash_set<T*> Table;
  /// The actual table, operations on the UniqueTable are delegated on this.
  Table table; // Unique table of GDDD

/* Canonical */
  /// The application operator, returns the address of the value already in 
  /// the table if it exists, or inserts and returns the address of the value inserted.
  /// \param _g the pointer to the value we want to find in the table.
  /// \return the address of an object stored in the UniqueTable such that (*_g == *return_value)
  const T*
  operator()(T *_g)
  {
#ifdef PARALLEL_DD
    table_mutex_t::scoped_lock lock(table_mutex_);
#endif

    std::pair<typename Table::iterator, bool> ref=table.insert(_g); 

    typename Table::iterator ti=ref.first;
    
    if ( !ref.second)
      {
		assert( _g != *ti );
        delete _g;
      }
    return *ti;
  }

  /// Returns the current number of filled entries in the table.
  size_t
  size() const
  {
    return table.size();
  }
};

#endif
