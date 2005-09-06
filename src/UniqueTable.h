/* -*- C++ -*- */
#ifndef UNIQUETABLE_H
#define UNIQUETABLE_H

// modif
#include <ext/hash_set>
#include <ext/hash_map>
// ajout
using namespace std;
using namespace __gnu_cxx;


/// This class implements a unicity table mechanism, based on an STL hash_set.
/// Requirements on the contained type are thus those of hash_set
template<typename T>
class UniqueTable{

#ifdef INST_STL
  long NbAcces;
  long NbInsertion;
#endif

public:

  UniqueTable(){
#ifdef INST_STL
    NbAcces=0;NbInsertion=0;
#endif
  }

  typedef hash_set<T*> Table;
  Table table; // Unique table of GDDD

/* Canonical */
  T *operator()(T *_g){
#ifdef INST_STL
    NbAcces++;
    int nbjumps=0;
    pair<typename Table::iterator, bool> ref=table.insert(_g, nbjumps); 
    _g->InstrumentNbJumps(nbjumps);
#else
    pair<typename Table::iterator, bool> ref=table.insert(_g); 
    
#endif
  
    
    typename Table::iterator ti=ref.first;
    if (!ref.second){
      delete _g;
    }
#ifdef INST_STL
    else {
      NbInsertion++;
    }
#endif

    return *ti;
  }

  size_t size() const{
    return table.size();
  }

#ifdef INST_STL
  void pstat(bool reinit=true){
    cout << "NbInsertion(" <<NbInsertion << ")*100/NbAccess(" << NbAcces<< ")  = " ;
    cout << ((long)(((long)NbInsertion) * 100)) / ((long)NbAcces) << endl;
    if (reinit ){
      NbAcces=0;
      NbInsertion=0;
    }
  }
#endif
};

#endif
