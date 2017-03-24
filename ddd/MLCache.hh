#ifndef _MLCACHE_HH_
#define _MLCACHE_HH_
  
#include "ddd/util/configuration.hh"


template
    <
      typename MLHomType
    , typename NodeType
  ,   typename HomNodeMapType
    >
class MLCache
{
private:
  mutable size_t peak_;
  
  typedef typename  hash_map< std::pair<MLHomType, NodeType>, HomNodeMapType >::type 
                    hash_map; 
  hash_map cache_;
    
public:
  MLCache () : peak_ (0) {};
    
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

    std::pair<bool,HomNodeMapType>
    insert(const MLHomType& hom, const NodeType& node)
    {
      bool found;
      
      { // lock on current bucket
	typename hash_map::const_accessor access;
	found = cache_.find ( access, std::make_pair(hom,node));
	if (found)
	  return std::make_pair(false, access->second);
      } // end of lock on the current bucket
      
      // wasn't in cache
      HomNodeMapType result = hom.eval(node);
      // lock on current bucket
      typename hash_map::accessor access;
      bool insertion = cache_.insert ( access, std::make_pair(hom,node));
      if (insertion) {
	// should happen except in MT case
	access->second = result;
      }
      return std::make_pair(insertion,result);
    }
  
#ifdef HASH_STAT
  std::map<std::string, size_t> get_hits() const { return cache_.get_hits(); }
  std::map<std::string, size_t> get_misses() const { return cache_.get_misses(); }
  std::map<std::string, size_t> get_bounces() const { return cache_.get_bounces(); }
#endif // HASH_STAT    
};

#endif /* _MLCACHE_HH_ */
