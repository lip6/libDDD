#ifndef _EXT_HASH_MAP_HH_
#define _EXT_HASH_MAP_HH_

#define GCC_VERSION (__GNUC__ * 10000 \
                + __GNUC_MINOR__ * 100 \
                   + __GNUC_PATCHLEVEL__)

#if GCC_VERSION < 40300
#include <ext/hash_map>
#else
#include <tr1/unordered_map>
#endif

#include "util/hash_support.hh"
#include <utility>

template
<
  typename Key,
  typename Data,
  typename HashKey = d3::util::hash<Key>,
  typename EqualKey = d3::util::equal<Key>
>
class ext_hash_map
{
  // Types
public:

#if GCC_VERSION < 40300
  typedef __gnu_cxx::hash_map<Key,Data,HashKey,EqualKey> internal_hash_map;
#else
  typedef std::tr1::unordered_map<Key,Data,HashKey,EqualKey> internal_hash_map;
#endif

  typedef typename internal_hash_map::iterator iterator;
  typedef typename internal_hash_map::const_iterator const_iterator;
  typedef typename internal_hash_map::size_type size_type;

  ////////////////////////////////////////////////////////////////

  class const_accessor
  {
    // Types
  public:

    typedef const typename std::pair<const Key, Data> value_type;

    // Attributes
  private:

    friend class ext_hash_map;
    friend class accessor;
    bool has_result_;
    const_iterator current_bucket_;
    
    // Methods
  public:
    
    const_accessor()
      :
      has_result_(false),
      current_bucket_()
    {
    }

    bool
    empty() const
    {
      return ! has_result_;
    }

    const value_type&
    operator*() const
    {
      return *current_bucket_;
    }

    const value_type* 
    operator->() const
    {
      return &operator*();
    }

    void
    release()
    {
      // nothing to do, because there are no mutexes to release
    }

  private:
    
    // cannot copy or assign a const_accessor
    const_accessor(const const_accessor&);
    const_accessor& operator=(const const_accessor&);

  };

  ////////////////////////////////////////////////////////////////
  class accessor
    :
    public const_accessor
  {
    // Types
  public:
    typedef typename std::pair<const Key, Data> value_type;
    
    // Attributes
  private:
    iterator current_bucket_;
    
    friend class ext_hash_map;
    // Methods
  public:
    
    value_type&
    operator*() const
    {
      return *(this->current_bucket_);
    }
    
    value_type*
    operator->() const
    {
      return &operator*();
    }

  };
  
  ////////////////////////////////////////////////////////////////
  
  // Attributes
private:

  friend class const_accessor;
  internal_hash_map map_;

  // Methods
public:

  ext_hash_map()
    :
    map_()
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
    map_.clear();
  }

  bool
  find( accessor& result, const Key& key)
  {
    iterator i =  map_.find(key);
    result.current_bucket_ = i;
    result.has_result_ = ( i != map_.end() );
    return result.has_result_;
  }

  bool
  find( const_accessor& result, const Key& key) const
  {
    const_iterator i =  map_.find(key);
    result.current_bucket_ = i;
    result.has_result_ = ( i != map_.end() );
    return result.has_result_;
  }

  bool
  insert( accessor& result, const Key& key)
  {
    std::pair<const Key, Data> value_to_insert(key,Data());
    std::pair<iterator,bool> p(map_.insert(value_to_insert));
    result.current_bucket_ = p.first;
    result.has_result_ = true;
    return p.second;
  }

  bool
  erase( const Key& key)
  {
    return map_.erase(key) > 1 ? false : true; 
  }

};

#endif
