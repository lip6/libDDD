/* -*- C++ -*- */
#ifndef UNIQUETABLE_H
#define UNIQUETABLE_H
// modif
#include <ext/hash_map>
// ajout
using namespace std;
using namespace __gnu_cxx;

template<typename T>
class UniqueTable{
public:
  typedef hash_map<T*,T*> Table;
  Table table; // Unique table of GDDD

/* Canonical */
  T *operator()(T *_g){
    // ajout (typename en début de ligne)
    typename Table::const_iterator ti=table.find(_g); // search the result in the table
    if(ti==table.end()){
      table[_g]=_g;  
      return _g;
    }
    else{
      delete _g;
      return ti->second;
    }      
  }

  int size() const{
    return table.size();
  }
};

#endif
