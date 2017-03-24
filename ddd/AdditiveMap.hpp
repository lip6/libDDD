#ifndef __ADDITIVEMAP_HH__
#define __ADDITIVEMAP_HH__

#include <vector>
#include "ddd/util/hash_support.hh"

template<typename K, typename V, typename EqualKey = d3::util::equal<K> >
class AdditiveMap {
  
  typedef std::vector<std::pair<K,V> > mapType;
  
  mapType map;
  
public:
  typedef typename mapType::value_type value_type;
  typedef typename mapType::const_iterator const_iterator;
  typedef typename mapType::iterator iterator;
  AdditiveMap(){};
  
  // delegate iterator operations to map
  const_iterator end() const { return map.end(); }
  const_iterator begin() const { return map.begin();}
  
  iterator find (const K & key) {
    iterator res = map.begin();
    while (res != map.end()) {
      if (EqualKey () (res->first,key))
        return res;
      ++res;
    }
    return res;
  }
  
  int addAll (const AdditiveMap<K,V> & other) {
    return addAll(other.begin(),other.end());
  }
  
  // adds a set of mappings 
  // returns the number of sums computed
  int addAll (const_iterator begin,const_iterator end) {
    int count = 0;
    for ( ; begin != end ; ++ begin ) {
      const value_type & val = *begin;
      if (add (val.first,val.second) ) 
        ++count;
    }
    return count;
  }
  
  // adds value to the value mapped to key
  // returns true if sum was actually used, false if normal insertion performed
  bool add (const K & key, const V & value) {
    typename mapType::iterator it = find(key);
    if ( it != map.end() ) {
      // found it
      it->second = it->second + value ;
      return true;
    } else {
      map.push_back(std::make_pair(key,value));
      return false;
    }
  }
  // removes value to the value mapped to key
  // returns true if difference - was actually used, false if nothing performed
  bool remove (const K & key, const V & value) {
    typename mapType::iterator it = find(key);
    if ( it != map.end() ) {
      // found it
      it->second = it->second - value ;
      return true;
    } else {
      return false;
    }
  }
};

#endif
