/* -*- C++ -*- */
#ifndef SDED_H
#define SDED_H

#include <set>
#include "DataSet.h"


class _SDED;
class GSDD;
class GShom;
        
/******************************************************************************/
class SDED{
private:
  friend struct __gnu_cxx::hash<SDED>;
  friend struct std::equal_to<SDED>;
  _SDED *concret;
public:
  GSDD eval();
  SDED(_SDED *c):concret(c){};
  bool operator==(const SDED&) const; 
  
  static GSDD add(const std::set<GSDD> &);
  static GSDD Shom(const GShom &,const GSDD&);

  /* Memory Manager */
  static  unsigned int statistics();
  static void pstats(bool reinit=true);
  static size_t peak();
#ifdef OTF_GARBAGE
  static void recentGarbage(bool force=false);
#endif // OTF_GARBAGE
  static void garbage(); 
  /// For storage in a hash table
  size_t hash () const ;
};


/******************************************************************************/
namespace __gnu_cxx {
  template<>  struct hash<SDED> {
    size_t operator()(const SDED&d) const { return d.hash(); }
  };
}

namespace std {
  template<>  struct equal_to<SDED> {
    bool operator()(const SDED&a,const SDED&b) const { return a == b ;};
  };
}

/******************************************************************************/
class _SDED{

public:
  /* Destructor */
  virtual ~_SDED(){};

#ifdef OTF_GARBAGE
  // to enact dynamic garbage collection mechanism
  // Returns true if all arguments of the operation of type GSDD have property that (isSon()==true)
  // This property triggers long term storage
  virtual bool shouldCache() {return false ;}
#endif

  /* Compare */
  virtual size_t hash() const =0;
  virtual bool operator==(const _SDED &) const=0;

  /* Transform */
  virtual GSDD eval() const=0; 

};

#ifdef EVDDD
GShom pushEVSDD(int v);
#endif

#endif

