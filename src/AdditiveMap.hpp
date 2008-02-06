#ifndef __ADDITIVEMAP_HH__
#define __ADDITIVEMAP_HH__

#include <map>


template<typename K,typename V>
class AdditiveMap  {
  
  typedef std::map<K,V> mapType;

  mapType map;

public :
  typedef typename mapType::value_type value_type;
  typedef typename mapType::const_iterator const_iterator;
  AdditiveMap(){};

  // delegate iterator operations to map
  const_iterator end() { return map.end(); }
  const_iterator begin() { return map.begin();}

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
  bool add (K key, V value) {
    typename mapType::iterator it = map.find(key);
    if ( it != map.end() ) {
      // found it
      it->second = it->second + value ;
      return true;
    } else {
      map.insert(std::make_pair(key,value));
      return false;
    }
  }
  // removes value to the value mapped to key
  // returns true if difference - was actually used, false if nothing performed
  bool remove (K key,V value) {
    typename mapType::iterator it = map.find(key);
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
