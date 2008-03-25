#ifndef __CACHE__H__
#define __CACHE__H__

#ifdef REENTRANT
#include "tbb/atomic.h"
#endif

#include "pthread.h"
#include "util/configuration.hh"
#include "util/hash_support.hh"
#include <cstdio>


/************************************************************************/
/*      A cache for operations on nodes **************/
template<typename HomType, typename NodeType>
class Cache {
  typedef typename std::pair<HomType,NodeType> key_type;
  typedef typename hash_map<key_type,NodeType>::type cacheType;
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
  std::pair<bool,NodeType> insert (const HomType & h, const NodeType & d);

};

template<typename HomType, typename NodeType>
std::pair<bool,NodeType> Cache<HomType,NodeType>::contains (const HomType & h, const NodeType & d) const {
  typename cacheType::const_accessor access;  


  if (!  cache.find(access,std::make_pair(h,d)) ) {
    // first time we hit this homomorphism : no cache
    ++misses;
    return std::make_pair(false,NodeType::null);
  } else {
      // return cached value
      ++hits;
      return std::make_pair(true,access->second);
  }
  

}

template<typename HomType, typename NodeType>
std::pair<bool,NodeType> Cache<HomType,NodeType>::insert (const HomType & h, const NodeType & d) {

  typename cacheType::accessor access;  

  //  printf("thread : %d   acquire lock <%p,%p>\n",pthread_self(), h.concret, d.concret);
  //  std::cout << "hits/miss/recompute: " << hits <<"/" <<misses << "/" <<recompute << std::endl;

  if (!  cache.insert(access,std::make_pair(h,d))) {
	++hits;
	//	 printf("thread : %d  release lock <%p,%p>\n",pthread_self(), h.concret, d.concret);
	return std::make_pair(false,access->second);
  } else {
    // cache insertion
    NodeType result = h.eval(d);
    access->second = result;
    // printf("thread : %d  release lock <%p,%p>\n",pthread_self(), h.concret, d.concret);
    return std::make_pair(true,result);
  }

}


#endif

