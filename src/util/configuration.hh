#ifndef _D3_CONFIGURATION_HH_
#define _D3_CONFIGURATION_HH_

#include "util/hash_support.hh"
#include "util/ext_hash_map.hh"
#include "util/tbb_hash_map.hh"

#ifdef PARALLEL_DD
#define CONCURR_HASH_MAP
#define PROTECTED_SET
#endif


template
<
  typename Key,
  typename Data,
  typename HashKey = d3::util::hash<Key>,
  typename EqualKey = d3::util::equal<Key>
> struct hash_map
{
# ifdef CONCURR_HASH_MAP
  typedef tbb_hash_map<Key,Data,HashKey,EqualKey> type;
# else
  typedef ext_hash_map<Key,Data,HashKey,EqualKey> type;
#endif
};


#endif
