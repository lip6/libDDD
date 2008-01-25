/****************************************************************************/
/*								            */
/* This file is part of libDDD, a library for manipulation of DDD and SDD.  */
/*     						                            */
/*     Copyright (C) 2001-2008 Yann Thierry-Mieg, Jean-Michel Couvreur      */
/*                             and Denis Poitrenaud                         */
/*     						                            */
/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU Lesser General Public License as       */
/*     published by the Free Software Foundation; either version 3 of the   */
/*     License, or (at your option) any later version.                      */
/*     This program is distributed in the hope that it will be useful,      */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/*     GNU LEsserGeneral Public License for more details.                   */
/*     						                            */
/* You should have received a copy of the GNU Lesser General Public License */
/*     along with this program; if not, write to the Free Software          */
/*Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*     						                            */
/****************************************************************************/

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
