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
  static void garbage(); 
};

/* Binary operators */
GDDD operator^(const GDDD&,const GDDD&); // concatenation
GDDD operator+(const GDDD&,const GDDD&); // union
GDDD operator*(const GDDD&,const GDDD&); // intersection
GDDD operator-(const GDDD&,const GDDD&); // difference

/******************************************************************************/
namespace __gnu_cxx {
  struct hash<DED> {
    size_t operator()(const DED&) const;
  };
}

namespace std {
  struct equal_to<DED> {
    bool operator()(const DED&,const DED&) const;
  };
}

/******************************************************************************/
class _DED{
public:
  /* Destructor */
  virtual ~_DED(){};
  /* Compare */
  virtual size_t hash() const =0;
  virtual bool operator==(const _DED &) const=0;

  /* Transform */
  virtual GDDD eval() const=0; 
};

#endif






