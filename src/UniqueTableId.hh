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
#ifndef UNIQUETABLE_ID_H
#define UNIQUETABLE_ID_H

#include <cassert>
#include <vector>
#include "util/configuration.hh"
#include "util/hash_support.hh"
#include "util/hash_set.hh"

// the clone contract

/// Requirements on the contained type are to be cloneable, 
/// hashable and equality comparable. 
/// For memory recollection, it should be markable, 
/// i.e. able 
///  
///  template interface T {
///      T* clone () const ;
///      bool operator==(const T&) const;
///      size_t hash() const;
///  }

/// These are the comparators/hash of d3:: namespace.
///  member hash and operator== are enough thanks to template instanciations. 
// Additional contract requirements for the stored type T :
// it should be cloneable. Implement clone in class C by :
// return new C(*this);



/// This class implements a unique table mechanism, based on a hash.
template<typename T, typename ID>
class UniqueTableId {
  typedef ID id_t;

  // To hash compare id's. id 0 is used as temporary id for comparisons.
  // hash value is given by the contents of unique object.
  // Unique objects T must implement : size_t hash () const;
  struct id_hash {
    size_t operator()(const id_t & id) const{      
      if (!id) return 0;
      return UniqueTableId::instance().resolve(id)->hash();
    }
  };

  // To compare id's. id 0 is used as temporary id for comparisons.
  // compare value is given by the contents of unique object.
  // Unique objects T must implement : bool operator == (const T & other) const;
  struct id_compare {
    bool operator()(const id_t & id1,const id_t & id2) const{
      if (id1==0 || id2 == 0) {
	// deleted key
	return false;
      }
      return * UniqueTableId::instance().resolve(id1) == * UniqueTableId::instance().resolve(id2);
    }
  };


  /// Typedef helps hide implementation type.
  /// The table wil hold actual entries for hashed unique test,
  /// These are the currently valid ids.
  typedef typename d3::hash_set<id_t, id_hash, id_compare>::type  table_t;
  /// The Indexes table stores the id to (unique) T*  map.
  /// It also stores the free list in potential spare spaces.
  typedef typename std::vector<const T*>  indexes_t;
  /// a sparse table holding refcounts for ref'd objects.
  /// Hopefully, we don't have more refs than there are nodes, id_t should be long enough to hold refcounts.
  typedef typename google::sparsetable<id_t> refs_t;
  /// A bitset to store marks on objects used for mark&sweep.
  typedef std::vector<bool> marks_t;

  /// The actual table, operations on the UniqueTable are delegated on this.
  table_t table; // Unique table of unique objects
  /// The actual index table, resolution of object from Id is done with this
  indexes_t index; // Index table of unique objects
  /// The free list is stored hidden in the free space of the index table.
  /// It is sorted by reverse deallocation order, since we only
  /// push or pop to head. Value 0 signifies no successor(it is also the deleted key marker).
  id_t head;
  /// The reference counters for ref'd nodes. It is a sparse table that only stores values for non-zero entries.
  refs_t refs;
  /// The marking entries, a bitset
  marks_t marks;
  // basic stats counter
  size_t peak_size_;

  /// add a node to free list
  void push (const id_t & id) {
    // std::cerr << "b4 push (" <<id << ")"; print_free_list(std::cerr);
    // chain head behind id 
    index[id] =((const T*) ((size_t)head));
    // set head to id
    head = id;
    // std::cerr << "after push (" <<id << ")"; print_free_list(std::cerr);
  }

  /// return the next free id, either collected from free_list or
  /// the newly allocated last position of the index table
  id_t next_id () {
    if (head == 0) {
      id_t ret = index.size();
      index.push_back(NULL);
      marks.push_back(false);
      return ret;
    } else {
      id_t ret = head;
      head = (size_t) index[head];
      marks[ret] = false;
      return ret;
    }
  }

  void print_free_list(std::ostream & os) const {
    id_t pos = head;
    os << "free list  ";
    while (pos != 0) {
      os << pos << " "; 
      pos = (size_t) index[pos];
      if (pos != 0) {
	os << "," ;
      }
    }
    os << std::endl;
  }

  void print_table(std::ostream & os) const {
    os << "table ";
    for(table_it di=begin() ; di!= end(); /* in loop */){
      id_t id = *di;
      ++di;
      os << id << " ";
      if (di != end()) {
	os << ",";
      } else {
	break;
      }
    }
    os << std::endl;
  }

  void print_marked(std::ostream & os) const {
    os << "marked : ";
    for(size_t i = 0 ; i < index.size() ; i++) {
      if (marks[i]) {
	os << i << " ";
      }
    }
    os << std::endl;
  }


public:

  // mark an entry to be kept
  void mark (const id_t & id) {
    if (! marks[id]) {
      marks[id] = true;
      resolve(id)->mark();
    }
  }

  // reference a unique object.
  // refs are used as heads for mark & sweep
  void ref (const id_t & id) {
    // make sure sparse table is large enough
    if (refs.size() < id) {
      // exponential may be a bit too much
      refs.resize((3*id)/2);
    }

    // test is true if ref count  != 0
    if (refs.test(id)) {
      // increment
      refs.set(id,refs.get(id)+1);
    } else {
//      std::cerr << "ref " << id <<  std::endl;
      // set to 1
      refs.set(id,1);
    }
  }
  
  // dereference an object.
  // when refcount is 0, the object is collectible unless it gets marked during mark&sweep.
  void deref (const id_t & id) {
    // assume refcount was > 0 
    assert(refs.test(id));
    id_t refc = refs.get(id);
    if (refc==1) {
//      std::cerr << "deref " << id <<  std::endl;
      // set to 0 = erase refcount
      refs.erase(id);
    } else {
      // decrement
      refs.set(id, refc-1);
    }
  }

  typedef typename table_t::const_iterator table_it; 

  table_it begin() const { return table.begin() ; }
  table_it end() const { return table.end() ; }

  
  static UniqueTableId & instance () {
    static UniqueTableId * const single_ = new UniqueTableId();
    return *single_;
  }

  /// Provide an initial size for both hash and index tables.
  /// Both will grow as needed if this size is exceeded.
  UniqueTableId(size_t s=4096):
    table (s), head(0),peak_size_(0)
  {
    index.reserve(s);
    // position 0 is used for deleted key marker
    index.push_back(NULL);
    table.set_deleted_key(0);
    // position 1 is reserved for hash comparisons of temporary objects.
    index.push_back(NULL);
    refs.resize(s);
    // so that marks and index always have congruent sizes.
    marks.reserve(s);
    marks.push_back(false);
    marks.push_back(false);
  }


  const T * resolve (const id_t & id) const {
    return index[id];
  }

/* Canonical */
  /// The application operator, returns the address of the value already in 
  /// the table if it exists, or inserts and returns the address of the value inserted.
  /// \param _g the pointer to the value we want to find in the table.
  /// \return the address of an object stored in the UniqueTable such that (*_g == *return_value)
  id_t
    operator()(const T &_g)
  {
    const int tmpid = 1;
    // temporary store of object at index 0
    index[tmpid]=&_g;
    // look for object identical to the one at id 0
    typename table_t::const_iterator it = table.find (tmpid);
    // whatever happens, free index 0
    index[tmpid]=NULL;

    if (it != table.end()) {
      // a hit, return the index found in table
      return *it;
    } else {
      // copy object to unique table storage memory space.
      // this step takes ownership for the memory, any deallocations must be done through "remove"
      T * clone = unique::clone<T>() (_g);

      id_t id = next_id();
      index[id] = clone;

      std::pair<typename table_t::iterator, bool> ref=table.insert(id); 
      assert(ref.second);
      ((void)ref); 
//      access->second = id;
      return id;
    }
  }

  /// Returns the current number of filled entries in the table.
  size_t
  size() const
  {
    return table.size();
  }

  size_t peak_size () {
    size_t siz = size();
    if (siz > peak_size_) 
      peak_size_=siz;  
    return peak_size_;
  }
  
  void garbage () {
    peak_size();
    // currently mark and sweep mode.

    //     print_table(std::cerr);
    //     print_free_list(std::cerr);
    //     print_marked(std::cerr);

    // mark phase
    // iterate over refcounted entries only 
    for (typename refs_t::nonempty_iterator it = refs.nonempty_begin() ; it != refs.nonempty_end() ; ++it ) {
      id_t id = refs.get_pos(it);
      mark(id);
    }
    //    std::cerr << "after mark ref'd : " ;  print_marked(std::cerr);

    table_t newtable (table.size()*2);
    newtable.set_deleted_key(0);
    // sweep phase
    for(table_it di=begin() ; di!= end();/*++ done in loop*/ ){
      id_t id = *di;
      // to avoid corruption if deleted
//      table_it ci = di;
      ++di;
      if (id==0) {
	continue;
      }
      // if not marked
      if (! marks[id] ) {
	// kill it
	// mark an entry for deletion, the id should not be used again, 
//	table.erase(ci);
	// free memory allocated by clone
	delete (T*) index[id];
	// id may be recycled to designate something else.
	push(id);
      } else {
	newtable.insert(id);
      }
      marks[id] = false;
    }
    // cleanup
    table = newtable;

//          print_table(std::cerr);
//          print_free_list(std::cerr);

  }


#ifdef HASH_STAT
  std::map<std::string, size_t> get_hits() const { return table.get_hits(); }
  std::map<std::string, size_t> get_misses() const { return table.get_misses(); }
  std::map<std::string, size_t> get_bounces() const { return table.get_bounces(); }
#endif // HASH_STAT
};

#endif
