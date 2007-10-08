#include <ext/hash_map>

#ifdef PARALLEL_DD

#include "tbb/atomic.h"
#include "tbb/queuing_mutex.h"
#include "tbb/mutex.h"
#include "tbb/queuing_rw_mutex.h"

#endif

template
<
	typename Key,
	typename Value
>
class __Cache
{
private:

	typedef __gnu_cxx::hash_map<Key,Value> Internal_Cache;
	Internal_Cache internal_cache_;
	
#ifdef PARALLEL_DD
	typedef tbb::queuing_rw_mutex rw_mutex;
	//typedef tbb::mutex mutex;
	
	rw_mutex cache_rw_mutex;
	//mutex cache_mutex;
#endif

public:
	
	typedef typename Internal_Cache::iterator iterator;
	typedef typename Internal_Cache::const_iterator const_iterator;
	
	__Cache()
		:
		internal_cache_()
#ifdef PARALLEL_DD		
		,
		cache_rw_mutex()
#endif
	{}
	
	virtual
	~__Cache ()
	{};

	inline
	unsigned int
	size()
	{
#ifdef PARALLEL_DD
		rw_mutex::scoped_lock lock(cache_rw_mutex,false/*reader*/);
#endif
		return internal_cache_.size();
	}
	
	inline
	void
	erase(typename Internal_Cache::iterator it)
	{
#ifdef PARALLEL_DD
		rw_mutex::scoped_lock lock(cache_rw_mutex,true/*writer*/);
#endif
		internal_cache_.erase(it);
	}
	
	inline
	iterator
	begin()
	{
#ifdef PARALLEL_DD
		rw_mutex::scoped_lock lock(cache_rw_mutex,false/*reader*/);
#endif
		return internal_cache_.begin();
	}

	inline
	const_iterator
	begin() 
	const
	{
	#ifdef PARALLEL_DD
		rw_mutex::scoped_lock lock(cache_rw_mutex,false/*reader*/);
	#endif
		return internal_cache_.begin();
	}

	inline
	iterator
	end()
	{
#ifdef PARALLEL_DD
		rw_mutex::scoped_lock lock(cache_rw_mutex,false/*reader*/);
#endif
		return internal_cache_.end();
	}

	inline
	const_iterator
	end()
	const
	{
	#ifdef PARALLEL_DD
		rw_mutex::scoped_lock lock(cache_rw_mutex,false/*reader*/);
	#endif
		return internal_cache_.end();
	}

	
	inline
	Value&
	operator[](const Key& k)
	{
#ifdef PARALLEL_DD
		rw_mutex::scoped_lock lock(cache_rw_mutex,true/*writer*/);
#endif
		return internal_cache_[k];
	}
	
	inline
	iterator
	find(const Key& k)
	{
#ifdef PARALLEL_DD
		rw_mutex::scoped_lock lock(cache_rw_mutex,false/*reader*/);
#endif
		return internal_cache_.find(k);
	}

	inline
	void
	clear()
	{
#ifdef PARALLEL_DD
		rw_mutex::scoped_lock lock(cache_rw_mutex,true/*writer*/);
#endif
		internal_cache_.clear();
	}
	
};
