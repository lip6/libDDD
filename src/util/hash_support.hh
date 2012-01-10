/****************************************************************************/
/*								            */
/* This file is part of libDDD, a library for manipulation of DDD and SDD.  */
/*     						                            */
/*     Copyright (C) 2001-2008 Alexandre Hamez, Yann Thierry-Mieg           */
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
#ifndef _HASH_SUPPORT_HH_
#define _HASH_SUPPORT_HH_

#include <utility>
#include <set>
#include <vector>
#include <typeinfo>
#include <string>

#include "hashfunc.hh"

#define GCC_VERSION (__GNUC__ * 10000 \
                + __GNUC_MINOR__ * 100 \
                   + __GNUC_PATCHLEVEL__)


#if GCC_VERSION < 40300
#include <ext/hash_map>
#else
#include <tr1/unordered_map>
#endif


namespace d3 { namespace util {

// Basic definition of a d3 hash function : define a member function : size_t hash() const , in hashable classes 
template<typename T>
struct hash
{
  size_t 
  operator()(const T &e) const
  {
    return e.hash();
  }
};

// Basic definition of a d3 equality test : define a member function : bool operator== (const T & )const , in comparable classes 
template<typename T>
struct equal
{
  bool
  operator()(const T &e1,const T &e2) const
  {
    return e1==e2;
  }
};

// Specialized version for storage of pointers to hashable objects. 
template <typename T>
struct hash<T*>
{
  size_t 
  operator()(const T* e) const
  {
    if (e == NULL) return 0;
    return e->hash();
  }
};

// Specialized version for storage of pointers to comparable objects. Perform deep comparison.
template<typename T>
struct equal<T*>
{
  bool
  operator()(const T* e1,const T* e2) const
  {
    if (e1 == NULL) return e2==NULL;
    if (e2 == NULL) return false;
    return (typeid(*e1)==typeid(*e2)?(*e1)==(*e2):false);
  }
};

// Specialized version for use of int as keys in hash_map
template<>
struct hash<int> {
  size_t operator()(int i) const {
    return ddd::wang32_hash(i);
  };
 };

// Specialized version for std::pair of hashable objects.
template <typename T1,typename T2>
struct hash<std::pair<T1,T2> > {
  size_t operator() (const std::pair<T1,T2> &p)const {
    return hash<T1>()(p.first) ^ hash<T2>()(p.second);
  };
};


// Specialized version for std::set of hashable objects.
template <typename T1>
struct hash<std::set<T1> > {
  size_t operator() (const std::set<T1> &p)const {
    size_t res = 11317;
    typename std::set<T1>::const_iterator it;
    for ( it = p.begin() ; it != p.end() ; ++it )
      res ^= it->hash();
    return res;
  };
};
// Specialized version for std::vector<int>.
template <>
struct hash<const std::vector<int> > {
  size_t operator() (const std::vector<int> &p)const {
    size_t res = 2473;
    std::vector<int>::const_iterator it;
    for ( it = p.begin() ; it != p.end() ; ++it )
      res ^= (ddd::wang32_hash(*it)* res);
    return res;
  };
};
// could this be removed somehow ??
template <>
struct hash<std::vector<int> > {
  size_t operator() (const std::vector<int> &p)const {
    return hash<const std::vector<int> > () (p);
  };
};


template <>
struct hash<std::vector<int>* > {
  size_t operator() (const std::vector<int> *p)const {    
    return hash<const std::vector<int> > () (*p);
  };
};
template <>
struct hash<const std::vector<int>* > {
  size_t operator() (const std::vector<int> *p)const {    
    return hash<const std::vector<int> > () (*p);
  };
};

// Specialized version for std::pair of comparable objects.
template<typename T1,typename T2>
struct equal<std::pair<T1,T2> >
{
  bool
  operator()(const std::pair<T1,T2> & e1,const std::pair<T1,T2>& e2) const
  {
    return equal<T1>()(e1.first,e2.first) && equal<T2>()(e1.second,e2.second);
  }
};


// For use of strings as d3::util hash table keys
  template<>
  struct hash<std::string> {
    size_t operator()(const std::string & string) const{
#if GCC_VERSION < 40300
    return __gnu_cxx::hash<const char*>()(string.c_str());
#else
    return std::tr1::hash<std::string>() (string);
#endif
    }
  };

  template<>
  struct equal<std::string> {
    bool operator()(const std::string & g1, const std::string & g2) const{
      return g1==g2;
    }
  };


}}


#endif
