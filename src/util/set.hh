#ifndef _SET_HH_
#define _SET_HH_

#include "util/configuration.hh"

namespace d3 { namespace util {

template
<
  typename Key,
  typename Compare,
  typename Allocator,
  typename RealSetTag
>
struct set;

}}

#ifdef PARALLEL_DD
#include "util/protected_std_set.tcc"
#endif

#endif
