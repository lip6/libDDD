#ifndef _D3_CONFIGURATION_HH_
#define _D3_CONFIGURATION_HH_

#include "util/hash_map.hh"
#include "util/set.hh"

#ifdef PARALLEL_DD
#define CONCURR_HASH_MAP
#define PROTECTED_SET
#endif

struct configuration
{

# ifdef CONCURR_HASH_MAP
  typedef concurrent_hash_map_tag hash_map_type;
# else
  typedef gnu_hash_map_tag hash_map_type;
# endif

# ifdef PROTECTED_SET
  typedef protected_std_set_tag set_type;
# endif
  
};

#endif
