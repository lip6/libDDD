#ifndef _HASH_MAP_HH_
#define _HASH_MAP_HH_

//namespace d3 { namespace util {

template
<
  typename Key,
  typename Data,
  typename HashKey,// = ddd::Hash<Key>,
  typename EqualKey,// = ddd::EqualKey<Key>,
  typename Allocator,// = ddd::Allocator<Key>,
  typename RealHashMapTag // default value?
>
struct hash_map;

//}} namespace d3::util

#ifdef PARALLEL_DD
#include "util/concurrent_hash_map.tcc"
#else
#include "util/ext_hash_map.tcc"
#endif

#endif
