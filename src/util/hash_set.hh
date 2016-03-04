#ifndef _HASH_SET_HH_
#define _HASH_SET_HH_

#ifndef USE_STD_HASH
#include <google/sparse_hash_set>
#else
#include <unordered_set>
#endif

#include "util/configuration.hh"

namespace d3 {

template
    <
      typename Key
    , typename Hash = d3::util::hash<Key>
    , typename Compare = d3::util::equal<Key> 
    , typename Allocator = typename conf::allocator<Key>::type
    >
struct hash_set
{
#ifndef USE_STD_HASH
  typedef typename google::sparse_hash_set<Key,Hash,Compare,Allocator> type;
#else
  typedef typename std::unordered_set<Key,Hash,Compare,Allocator> type;
#endif
};
    
} // namespace d3

#endif /* _SET_HH_ */
