#ifndef _VECTOR_HH_
#define _VECTOR_HH_

#include <vector>

#include "ddd/util/configuration.hh"

namespace d3 { namespace util
{
    
template
    <
      typename T
    , typename Allocator = typename conf::allocator<T>::type
    >
struct vector
{
    typedef typename std::vector<T,Allocator> type;
};
    
}} // namespace d3::util


#endif /* _VECTOR_HH_ */
