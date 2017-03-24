#ifndef _CACHE_HH_
#define _CACHE_HH_
  
#include "util/configuration.hh"

template
    <
      typename FuncType
    , typename ParamType
    , typename ResType
    , typename EvalFunc=int
  >
class Cache
{
private:
  mutable size_t peak_;
  
  typedef typename  hash_map< std::pair<FuncType, ParamType>, ResType >::type 
                    hash_map; 
  hash_map cache_;
    
public:
  Cache () : peak_ (0) {};
  Cache (size_t s) : peak_ (0), cache_ (s) {};
    
  /** clear the cache, discarding all values. */
  void clear (bool keepstats = false) {
    peak();
    cache_.clear();
  }

  size_t peak () const {
    size_t s = size();
    if ( peak_ < s )
      peak_ = s;
    return peak_;
  }


  size_t size () const {
    return cache_.size();
  }

  ResType eval (const  FuncType & func, const ParamType  & param) const {
    return func.eval(param);
  }

  bool
  should_insert (const FuncType & ) const
  {
    return true;
  }
  
    std::pair<bool,ResType>
    insert(const FuncType& hom, const ParamType& node)
    {
      bool found;
      
      { // lock on current bucket
	typename hash_map::const_accessor access;
	found = cache_.find ( access, std::make_pair(hom,node));
	if (found)
	  return std::make_pair(false, access->second);
      } // end of lock on the current bucket
      
      // wasn't in cache
      ResType result = eval(hom, node);
      if (should_insert (hom))
      {
      // lock on current bucket
      typename hash_map::accessor access;
      bool insertion = cache_.insert ( access, std::make_pair(hom,node));
      if (insertion) {
	// should happen except in MT case
	access->second = result;
      }
      return std::make_pair(insertion,result);
      }
      else
        return std::make_pair (false, result);
    }
  
#ifdef HASH_STAT
  std::map<std::string, size_t> get_hits() const { return cache_.get_hits(); }
  std::map<std::string, size_t> get_misses() const { return cache_.get_misses(); }
  std::map<std::string, size_t> get_bounces() const { return cache_.get_bounces(); }
#endif // HASH_STAT    
};

#endif /* _CACHE_HH_ */
