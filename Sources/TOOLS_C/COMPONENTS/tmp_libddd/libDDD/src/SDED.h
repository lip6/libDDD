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
  friend struct hash<SDED>;
  friend struct equal_to<SDED>;
  _SDED *concret;
public:
  GSDD eval();
  SDED(_SDED *c):concret(c){};
  bool operator==(const SDED&) const; 
  
  static GSDD add(const set<GSDD> &);
  static GSDD Shom(const GShom &,const GSDD&);

  /* Memory Manager */
  static  unsigned int statistics();
  static void pstats(bool reinit=true);
  static void recentGarbage(bool force=false);
  static void garbage(); 
};


/******************************************************************************/
namespace __gnu_cxx {
  template<>  struct hash<SDED> {
    size_t operator()(const SDED&) const;
  };
}

namespace std {
  template<>  struct equal_to<SDED> {
    bool operator()(const SDED&,const SDED&) const;
  };
}

/******************************************************************************/
class _SDED{
#ifdef INST_STL
static long long NBJumps;
static long long NBAccess;
#endif

public:
  /* Destructor */
  virtual ~_SDED(){};

  // to enact dynamic garbage collection mechanism
  // Returns true if all arguments of the operation of type GSDD have property that (isSon()==true)
  // This property triggers long term storage
  virtual bool shouldCache() {return false ;}

  /* Compare */
  virtual size_t hash() const =0;
  virtual bool operator==(const _SDED &) const=0;

  /* Transform */
  virtual GSDD eval() const=0; 

#ifdef INST_STL
  static void InstrumentNbJumps(int nboops){NBJumps+=(1+nboops);NBAccess++;}
  static void ResetNbJumps(){NBJumps=0; NBAccess=0;}
  static double StatJumps() {if (NBAccess!=0)  return double(NBJumps) / double(NBAccess); return -1;}
#endif

};

#endif

