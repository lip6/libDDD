#ifndef _MAP_HH_
#define _MAP_HH_

#include <map>
#include <utility>

#include "util/configuration.hh"

namespace d3 { namespace util
{
    
template
    <
      typename Key
    , typename Data
    , typename Compare = std::less<Key>
    , typename Allocator = typename conf::allocator< std::pair<const Key, Data> >::type
    >
struct map
{
    typedef typename std::map<Key,Data,Compare,Allocator> type;
};
    
}} // namespace d3::util

#endif /* _MAP_HH_ */
