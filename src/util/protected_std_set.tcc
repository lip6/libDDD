#include <set>
#include <utility>
#include <tbb/mutex.h>

struct protected_std_set_tag{};

namespace d3 { namespace util {

template
<
  typename Key,
  typename Compare,
  typename Allocator
>
class set
<
  Key,
  Compare,
  Allocator,
  protected_std_set_tag // <= Specialization
>
{

// Types
private:
  
  //typedef std::set<Key,Compare,Allocator> internal_set;
  typedef tbb::mutex mutex;
  
public:
  
  typedef std::set<Key,Compare,Allocator> internal_set;
  typedef typename internal_set::iterator iterator;
  typedef typename internal_set::const_iterator const_iterator;  
  typedef Key value_type;
  
// Attributes
private:
    
  internal_set internal_set_;
  mutex set_mutex_;
  
// Methods
public:
    
  iterator
  begin()
  {
    return internal_set_.begin();
  }
  
  iterator
  end()
  {
    return internal_set_.end();
  }
  
  std::pair<iterator, bool>
  insert(const value_type& x)
  {
    mutex::scoped_lock lock(set_mutex_);
    return internal_set_.insert(x);
  }
  
  const internal_set&
  get_set() const
  {
    return internal_set_;
  }
  
};

}}
