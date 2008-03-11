#ifndef __CACHE__H__
#define __CACHE__H__

#include "util/configuration.hh"
#include "util/hash_support.hh"

/************************************************************************/
/*      A cache for operations on nodes **************/
template<typename HomType, typename NodeType>
class Cache {
  typedef typename hash_map<NodeType,NodeType>::type valMap;
  typedef typename hash_map<HomType,valMap>::type cacheType;
  cacheType cache;
public :
  /** Determine if the cache contains the entry for h(d).
    * Returns true and the resulting value if cache entry exists, 
    * or false and NodeType::null otherwise. */ 
  std::pair<bool,NodeType> contains (const HomType & h, const NodeType & d);
  
  /** Set the resulting value of h(d) = result. 
    * Returns true if the insert was actually performed or false if the value for h(d) was already in the cache.  */
  bool insert (const HomType & h, const NodeType & d, const NodeType & result);

};

template<typename HomType, typename NodeType>
std::pair<bool,NodeType> Cache<HomType,NodeType>::contains (const HomType & h, const NodeType & d) {
  typename cacheType::const_accessor access;  
  cache.find(access,h);

  if (access.empty()) {
    // first time we hit this homomorphism : no cache
    return std::make_pair(false,NodeType::null);
  } else {
    typename valMap::const_accessor val_access ;
    access->second.find(val_access,d);
    if (val_access.empty()) {
      // no application of h(d) found
      return std::make_pair(false,NodeType::null);
    } else {
      // return cached value
      return std::make_pair(true,val_access->second);
    }
  }

}

template<typename HomType, typename NodeType>
bool Cache<HomType,NodeType>::insert (const HomType & h, const NodeType & d, const NodeType & result) {

  typename cacheType::accessor access;  

  if (  cache.insert(access,h) ) {
    // first time we hit this homomorphism : no cache built yet
    access->second = valMap();
  }
  
  typename valMap::accessor val_access ;
  if (access->second.insert(val_access,d)) {
    val_access->second = result;
    return true;
  } else {
    assert(result == val_access->second);
    return false;
  }

}



#endif

