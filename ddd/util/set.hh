#ifndef _SET_HH_
#define _SET_HH_

#include <set>

#include "util/configuration.hh"

namespace d3 {

template
    <
      typename Key
    , typename Compare = std::less<Key>
    , typename Allocator = typename conf::allocator<Key>::type
    >
struct set
{
    typedef typename std::set<Key,Compare,Allocator> type;
};


template
    <
      typename Key
    , typename Compare = std::less<Key>
    , typename Allocator = typename conf::allocator<Key>::type
    >
struct multiset
{
    typedef typename std::multiset<Key,Compare,Allocator> type;
};



    
} // namespace d3

#endif /* _SET_HH_ */
