#ifndef _D3_CONFIGURATION_HH_
#define _D3_CONFIGURATION_HH_

#ifdef PARALLEL_DD
#ifndef REENTRANT
#define REENTRANT
#endif
#endif

#include "util/hash_support.hh"
#include "util/ext_hash_map.hh"
#include "util/tbb_hash_map.hh"

template
<
  typename Key,
  typename Data,
  typename HashKey = d3::util::hash<Key>,
  typename EqualKey = d3::util::equal<Key>
> struct hash_map
{
# ifdef REENTRANT
  typedef tbb_hash_map<Key,Data,HashKey,EqualKey> type;
# else
  typedef ext_hash_map<Key,Data,HashKey,EqualKey> type;
#endif
};

namespace conf
{

template
<
	typename T
>
struct allocator
{
	typedef typename std::allocator<T> type;
};

} // conf

#endif
