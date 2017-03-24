#ifndef _CONCURRENT_HASH_MAP_HH_
#define _CONCURRENT_HASH_MAP_HH_

#ifdef REENTRANT

#include <tbb/concurrent_hash_map.h>
#include <tbb/mutex.h>

#include "util/hash_support.hh"

template
<
  typename Key,
  typename Data,
  typename HashKey = d3::util::hash<Key>,
  typename EqualKey = d3::util::equal<Key>
>
struct tbb_hash_map
{
  struct hash_compare
  {    
    bool
    equal( const Key& k1, const Key& k2)
    {
      return EqualKey()(k1,k2);
    }

    size_t
    hash( const Key& k)
    {
      return HashKey()(k);
    }

  };

  // Types
  typedef tbb::mutex mutex;
  typedef tbb::concurrent_hash_map<Key,Data,hash_compare> internal_hash_map;
  typedef typename internal_hash_map::iterator iterator;
  typedef typename internal_hash_map::const_iterator const_iterator;
  typedef typename internal_hash_map::size_type size_type;
  typedef typename internal_hash_map::accessor accessor;
  typedef typename internal_hash_map::const_accessor const_accessor;

  // Attributes
  internal_hash_map map_;
  mutex map_mutex_;

  // Methods
  tbb_hash_map()
    :
    map_(),
    map_mutex_()
  {
  }

  iterator
  begin()
  {
    return map_.begin();
  }

  const_iterator
  begin() const
  {
    return map_.begin();
  }
  
  iterator
  end()
  {
    return map_.end();
  }

  const_iterator
  end() const
  {
    return map_.end();
  }

  size_type
  size() const
  {
    return map_.size();
  }
  
  bool
  empty() const
  {
    return map_.empty();
  }

  void
  clear()
  {
    // non reentrant method, need to lock the hash_map
    mutex::scoped_lock lock(map_mutex_);
    map_.clear();
  }

  bool
  find( const_accessor& result, const Key& key) const
  {
    return map_.find(result,key);
  }

  bool
  find( accessor& result, const Key& key)
  {
    return map_.find(result,key);
  }

  bool
  insert( const_accessor& result, const Key& key)
  {
    return map_.insert(result,key);
  }

  bool
  insert( accessor& result, const Key& key)
  {
    return map_.insert(result,key);
  }

  bool
  erase( const Key& key)
  {
    return map_.erase(key);
  }

};

#endif // REENTRANT

#endif /* _CONCURRENT_HASH_MAP_HH_ */
