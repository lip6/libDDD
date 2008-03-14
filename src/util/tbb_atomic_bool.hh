#ifndef _D3_TBB_ATOMIC_BOOL_HH_
#define _D3_TBB_ATOMIC_BOOL_HH_
#include <tbb/atomic.h>

namespace tbb
{

template<>
struct atomic<bool> {
private:
    bool volatile my_value;
    typedef internal::atomic_word<sizeof(bool)>::word word;
public:
    typedef bool value_type;
    template<memory_semantics M>
    value_type compare_and_swap( value_type value, value_type comparand ) {
        return internal::atomic_traits<sizeof(value_type),M>::compare_and_swap(&my_value,word(value),word(comparand))!=0;
    }

    value_type compare_and_swap( value_type value, value_type comparand ) {
        return compare_and_swap<__TBB_full_fence>(value,comparand);
    }

    template<memory_semantics M>
    value_type fetch_and_store( value_type value ) {
        return internal::atomic_traits<sizeof(value_type),M>::fetch_and_store(&my_value,word(value))!=0;
    }

    value_type fetch_and_store( value_type value ) {
        return fetch_and_store<__TBB_full_fence>(value);
    }

    operator value_type() const {
        return __TBB_load_with_acquire(my_value);
    }

    value_type operator=( value_type rhs ) {
        __TBB_store_with_release(my_value,rhs);
        return rhs;
    }
};
} // namespace tbb

#endif
