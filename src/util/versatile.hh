#ifndef _VERSATILE_HH_
#define _VERSATILE_HH_

////////////////////////////////////////////////////////////

#ifdef PARALLEL_DD
#include <tbb/task.h>
#endif

////////////////////////////////////////////////////////////

class versatile
{
#ifdef PARALLEL_DD
private:

	tbb::task* parent_task_;

public:
	
	void
	set_parent_task(tbb::task* t)
	{
		parent_task_ = t;
	}
	
	tbb::task*
	get_parent_task()
	{
		return parent_task_;
	}
	
#endif

};

////////////////////////////////////////////////////////////

#endif /* _VERSATILE_HH_ */
