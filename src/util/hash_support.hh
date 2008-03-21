#ifndef _HASH_SUPPORT_HH_
#define _HASH_SUPPORT_HH_

#include <utility>

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

template <typename T1,typename T2>
struct hash<std::pair<T1,T2> > {
  size_t operator() (const std::pair<T1,T2> &p)const {
    return hash<T1>()(p.first) ^ hash<T2>()(p.second);
  };
};


template<typename T1,typename T2>
struct equal<std::pair<T1,T2> >
{
  bool
  operator()(const std::pair<T1,T2> & e1,const std::pair<T1,T2>& e2) const
  {
    return equal<T1>()(e1.first,e2.first) && equal<T2>()(e1.second,e2.second);
  }
};


}}


#endif
