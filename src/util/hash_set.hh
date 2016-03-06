#ifndef _HASH_SET_HH_
#define _HASH_SET_HH_

#define GCC_VERSION (__GNUC__ * 10000 \
                + __GNUC_MINOR__ * 100 \
                   + __GNUC_PATCHLEVEL__)

#ifndef USE_STD_HASH
#  include <google/sparse_hash_set>
#else
#  if GCC_VERSION < 40300
#    include <ext/hash_set>
#  elif __cplusplus>=201103L
#    include <unordered_set>
#  else
#    include <tr1/unordered_set>
#  endif
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
#  if GCC_VERSION < 40300
	typedef typename __gnu_cxx::hash_set<Key,Hash,Compare,Allocator> type;
#  elif __cplusplus>=201103L
	typedef typename std::unordered_set<Key,Hash,Compare,Allocator> type;	
#  else
	typedef typename std::tr1::unordered_set<Key,Hash,Compare,Allocator> type;
#  endif
#endif
};
    
} // namespace d3

#endif /* _SET_HH_ */
