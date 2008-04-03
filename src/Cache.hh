#ifndef _CACHE_HH_
#define _CACHE_HH_
  
#include "util/configuration.hh"
#include "util/hash_support.hh"


template
    <
      typename HomType
    , typename NodeType
    >
class Cache
{
private:

    struct result_type
    {
        NodeType node_;
#ifdef REENTRANT
        tbb::mutex mutex_;
        bool computed_;
#endif 
        
        result_type()
            : node_()
#ifdef REENTRANT
            , mutex_()
            , computed_(false)
#endif
        {
        }
        
    };

    typedef
        typename  hash_map< std::pair<HomType, NodeType>, result_type >::type 
        hash_map; 
    hash_map cache_;
    
public:
    

    std::pair<bool,NodeType>
    insert(const HomType& hom, const NodeType& node)
    {
        
#ifdef REENTRANT
        tbb::mutex::scoped_lock lock;
#endif
        result_type* result = NULL;
        bool insertion;
        
        { // lock on current bucket
            typename hash_map::accessor access;
            insertion = cache_.insert(access, std::make_pair(hom,node));
            result = &(access->second);
#ifdef REENTRANT
            if( insertion )
            {
                lock.acquire(result->mutex_);
            }
#endif
        } // end of lock on the current bucket
        
        if( insertion )
        {   // wasn't in cache
            result->node_ = hom.eval(node);
#ifdef REENTRANT
            result->computed_ = true;
            lock.release();
#endif            
        }
#ifdef REENTRANT
        else
        {   // was in cache
            if( not result->computed_ )
            {   // need to wait for the end of computation by first thread
                // arrived into the cache
                lock.acquire(result->mutex_);
            }
        }
#endif
        return std::make_pair(insertion,result->node_);
    }
    
};

#endif /* _CACHE_HH_ */
