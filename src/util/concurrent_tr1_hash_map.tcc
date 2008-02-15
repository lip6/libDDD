#include <tr1/unordered_map>
#include <utility>
#include <tbb/mutex.h>

struct concurrent_tr1_hash_map_tag{};

// Specialization for the TR1 unordered map
template
<
  typename Key,
  typename Data,
  typename HashKey,
  typename EqualKey,
  typename Allocator
>
class hash_map
<
  Key,
  Data,
  HashKey,
  EqualKey,
  Allocator,
  concurrent_tr1_hash_map_tag // <= Specialization
>
{
  // Types
public:


  typedef typename internal_hash_map::iterator iterator;
  typedef typename internal_hash_map::const_iterator const_iterator;
  typedef typename internal_hash_map::size_type size_type;


private:

  typedef tbb::mutex mutex;
  typedef std::tr1::unordered_map< Key,Data,HashKey,EqualKey,Allocator> internal_hash_map;

  // Attributes
private:

  internal_hash_map map_;
  mutex map_mutex_;

  // Methods
public:

  hash_map()
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
    tbb::scoped_lock lock(map_mutex_);
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
    tbb::scoped_lock lock(map_mutex_);
    map_.clear();
  }

  bool
  find( const_accessor& result, const Key& key) const
  {
    const_iterator ci =  map_.find(key);
    result.current_bucket_ = ci;
    result.has_result_ = ( ci == map_.end() );
    return result.has_result_;
  }

  bool
  find( accessor& result, const Key& key)
  {
    iterator i =  map_.find(key);
    result.current_bucket_ = i;
    result.has_result_ = ( i == map_.end() );
    return result.has_result_;
  }

  bool
  insert( const_accessor& result, const Key& key)
  {
    std::pair<const Key, Data> value_to_insert(key,Data());
    std::pair<iterator,bool> p(map_.insert(value_to_insert));
    result.current_bucket_ = p.first;
    result.has_result_ = p.second;
    return result.has_result_;
  }

  bool
  insert( accessor& result, const Key& key)
  {
    std::pair<const Key, Data> value_to_insert(key,Data());
    std::pair<iterator,bool> p(map_.insert(value_to_insert));
    result.current_bucket_ = p.first;
    result.has_result_ = p.second;
    return result.has_result_;
  }

  bool
  erase( const Key& key)
  {
    return map_.erase(key) > 1 ? false : true; 
  }

};
