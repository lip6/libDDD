/* -*- C++ -*- */
#ifndef DED_H
#define DED_H

#include <set>
#include "DDD.h"
#include "Hom.h"

class _DED;
class GDDD;
class GHom;

/******************************************************************************/
class DED{
private:
  friend struct hash<DED>;
  friend struct equal_to<DED>;
  _DED *concret;
public:
  GDDD eval();
  DED(_DED *c):concret(c){};
  bool operator==(const DED&) const; 
  static GDDD add(const set<GDDD> &);
  static GDDD hom(const GHom &,const GDDD&);

  /* Memory Manager */
  static  unsigned int statistics();
  static void pstats(bool reinit=true);
  static void garbage(); 
};


/******************************************************************************/
namespace __gnu_cxx {
  template<>
  struct hash<DED> {
    size_t operator()(const DED&) const;
  };
}

namespace std {
  template<>
  struct equal_to<DED> {
    bool operator()(const DED&,const DED&) const;
  };
}

/******************************************************************************/
class _DED{

#ifdef INST_STL
static long long NBJumps;
static long long NBAccess;
#endif

public:
  /* Destructor */
  virtual ~_DED(){};

  virtual bool shouldCache() { return true;}

  /* Compare */
  virtual size_t hash() const =0;
  virtual bool operator==(const _DED &) const=0;

  /* Transform */
  virtual GDDD eval() const=0; 

#ifdef INST_STL
  static void InstrumentNbJumps(int nboops){NBJumps+=(1+nboops);NBAccess++;}
  static void ResetNbJumps(){NBJumps=0; NBAccess=0;}
  static double StatJumps() {if (NBAccess!=0)  return double(NBJumps) / double(NBAccess); return -1;}
#endif

};

#endif






