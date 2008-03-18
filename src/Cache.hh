#ifndef __CACHE__H__
#define __CACHE__H__

#ifdef REENTRANT
#include "tbb/atomic.h"
#endif

#include "util/configuration.hh"
#include "util/hash_support.hh"

/************************************************************************/
/*      A cache for operations on nodes **************/
template<typename HomType, typename NodeType>
class Cache {
  typedef typename hash_map<NodeType,NodeType>::type valMap;
  typedef typename hash_map<HomType,valMap>::type cacheType;
  cacheType cache;

#ifdef REENTRANT
  mutable tbb::atomic<long> hits;
  mutable tbb::atomic<long> misses;
    mutable tbb::atomic<long> recompute;
#else
  mutable long hits;
  mutable long misses;
  mutable long recompute;
#endif
public :

  Cache() 
	{
		hits = misses = 0;
	}
  /** Determine if the cache contains the entry for h(d).
    * Returns true and the resulting value if cache entry exists, 
    * or false and NodeType::null otherwise. */ 
  std::pair<bool,NodeType> contains (const HomType & h, const NodeType & d) const;
  
  /** Set the resulting value of h(d) = result. 
    * Returns true if the insert was actually performed or false if the value for h(d) was already in the cache.  */
  bool insert (const HomType & h, const NodeType & d, const NodeType & result);

};

template<typename HomType, typename NodeType>
std::pair<bool,NodeType> Cache<HomType,NodeType>::contains (const HomType & h, const NodeType & d) const {
  typename cacheType::const_accessor access;  


  if (!  cache.find(access,h) ) {
    // first time we hit this homomorphism : no cache
    ++misses;
    return std::make_pair(false,NodeType::null);
  } else {
    typename valMap::const_accessor val_access ;
    
    if (! access->second.find(val_access,d)) {
      // no application of h(d) found
      ++misses;
      return std::make_pair(false,NodeType::null);
    } else {
      // return cached value
      ++hits;
      return std::make_pair(true,val_access->second);
    }
  }

}

template<typename HomType, typename NodeType>
bool Cache<HomType,NodeType>::insert (const HomType & h, const NodeType & d, const NodeType & result) {

  typename cacheType::accessor access;  

  // will create a defult valMap if it does not exist yet
  cache.insert(access,h);
  
  typename valMap::accessor val_access ;
  if (! access->second.insert(val_access,d)) {
	++recompute;
    // no insertion performed : value in cache
    if(result != val_access->second)
	{
		std::cout 
			<< "apply on" << std::endl
			<< d << std::endl << "@@@@@@@@@@@@@@@" << std::endl
			<< result << std::endl << "@@@@@@@@@@@@@@@"
			<< val_access->second << std::endl << "@@@@@@@@@@@@@@@"
			<< val_access->second -result<< std::endl << std::endl
			<< "recompute: "<< recompute
			<< std::endl;
		assert(false);
		
	}
    return false;
  } else {
    // cache insertion
    val_access->second = result;
    return true;
  }

}



#endif

