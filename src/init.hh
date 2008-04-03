#ifndef _D3_MANAGER_HH_
#define _D3_MANAGER_HH_

#ifdef PARALLEL_DD
#include "tbb/task_scheduler_init.h"
#endif

namespace d3 {
	
class init
{
private:
	
#ifdef PARALLEL_DD
    tbb::task_scheduler_init tbb_init_;
#endif

public:

	init()
#ifdef PARALLEL_DD
        :
        tbb_init_()
#endif
	{
	}
	
	~init()
	{
	}
};
	
} // namespace d3

#endif /* _D3_MANAGER_HH_ */
