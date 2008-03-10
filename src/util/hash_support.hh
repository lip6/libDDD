#ifndef _HASH_SUPPORT_HH_
#define _HASH_SUPPORT_HH_

namespace d3 { namespace util {

template<typename T>
struct hash
{
  size_t 
  operator()(const T &e) const
  {
    return e.hash();
  }
};

template<typename T>
struct equal
{
  bool
  operator()(const T &e1,const T &e2) const
  {
    return e1==e2;
  }
};

template <typename T>
struct hash<T*>
{
  size_t 
  operator()(const T* e) const
  {
    return e->hash();
  }
};

template<typename T>
struct equal<T*>
{
  bool
  operator()(const T* e1,const T* e2) const
  {
    return (*e1)==(*e2);
  }
};

}}


#endif
