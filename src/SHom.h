/* -*- C++ -*- */
#ifndef SHOM_H_
#define SHOM_H_

#include "DataSet.h"
#include "SDD.h"

#include <string>
#include <map>
/**********************************************************************/
#ifdef INST_STL
typedef  std::pair<long long int, long long int> PairLL;
typedef std::map<std::string, PairLL > MapJumps;
#endif

class _GShom;
class StrongShom;

class GShom {
private:
  friend class Shom;
  friend struct hash<GShom>;
  friend GShom operator+(const GShom &,const GShom &); 
  friend GShom operator&(const GShom &,const GShom &); // composition
  friend GShom operator*(const GSDD &,const GShom &); 
  friend GShom operator*(const GShom &,const GSDD &); 
  friend GShom operator^(const GSDD &,const GShom &); 
  friend GShom operator^(const GShom &,const GSDD &); 
  friend GShom operator-(const GShom &,const GSDD &); 
  const _GShom* concret;
  GShom(const _GShom *_h):concret(_h){};
public:
  /* Constructor */
  GShom():concret(id.concret){};
  GShom(const StrongShom *);
  GShom(StrongShom *);
  GShom(const GSDD& d);   // constant
  GShom(int var,const DataSet& val, const GShom &h=GShom::id);  // var -- val -> Id

  /* Elementary shomomorphisms */
  static const GShom id;

  /* Compare */
  bool operator==(const GShom &h) const{return concret==h.concret;};
  bool operator!=(const GShom &h) const{return concret!=h.concret;};
  bool operator<(const GShom &h) const{return concret<h.concret;};

  /* evaluation */ 
  GSDD operator()(const GSDD &d) const; // evaluation
  GSDD eval(const GSDD &d) const;

  /* visualisation*/
  int refCounter() const;

  /* Operations */
  static GShom add(const set<GShom>&);

  /* Memory Manager */
  static  unsigned int statistics();
  static void pstats(bool reinit=true);
  void mark()const;
  static void garbage(); 
};



/* Operations */

GShom operator+(const GShom &,const GShom &); 
GShom operator&(const GShom &,const GShom &); // composition
GShom operator*(const GSDD &,const GShom &); 
GShom operator*(const GShom &,const GSDD &); 
GShom operator^(const GSDD &,const GShom &); 
GShom operator^(const GShom &,const GSDD &); 
GShom operator-(const GShom &,const GSDD &); 


class Shom:public GShom{
public:
  /* Constructor */
  Shom(const GShom &h=GShom::id);
  Shom(const Shom &h);
  Shom(const GSDD& d);   // constant
  Shom(int var,const DataSet& val, const GShom &h=GShom::id);  // var -- val -> Id
  ~Shom();

  /* Set */
  
  Shom &operator=(const GShom &);
  Shom &operator=(const Shom &);
};

/******************************************************************************/
namespace __gnu_cxx {
  template<>  struct hash<GShom> {
    size_t operator()(const GShom &g) const{
      return (size_t) g.concret;
    }
  };
}

namespace std {
  template<>  struct equal_to<GShom> {
    bool operator()(const GShom &g1,const GShom &g2) const{
      return g1==g2;
    }
  };
}

namespace std {
  template<>  struct less<GShom> {
    bool operator()(const GShom &g1,const GShom &g2) const{
      return g1<g2;
    }
  };
}

/**********************************************************************/
class _GShom{
private:
  friend class GShom;
  friend class Shom;
  mutable int refCounter;
  mutable bool marking;
  mutable bool immediat;
public:
#ifdef INST_STL
  static MapJumps HomJumps;
#endif
  /* Destructor*/
  _GShom(int ref=0,bool im=false):refCounter(ref),marking(false),immediat(im){};
  virtual ~_GShom(){};

  /* Compare */
  virtual bool operator==(const _GShom &h) const=0;
  virtual size_t hash() const=0;

  /* Eval */
  virtual GSDD eval(const GSDD &)const=0;

  /* Memory Manager */
  virtual void mark() const{};
#ifdef INST_STL
  virtual void InstrumentNbJumps(int nbjumps)
  {
    const char *name=typeid(*this).name();
    MapJumps::iterator ii;
    if ((ii=HomJumps.find(string(name)))==HomJumps.end()){
      HomJumps[string(name)]=PairLL(1L, (long long int) (1+nbjumps));
    }
    else {
      ii->second.first++;
      ii->second.second+=(1+nbjumps);
    }
  }
#endif
};

class StrongShom:public _GShom{
public:
  StrongShom(){};
  virtual ~StrongShom(){};
  virtual GSDD phiOne() const{return GSDD::top;}; 
  virtual GShom phi(int var,const DataSet& val) const=0; 
  virtual bool operator==(const StrongShom &h) const=0;
  bool operator==(const _GShom &h) const;

  //Eval
  GSDD eval(const GSDD &)const;  
};
 
#endif /* SHOM_H_ */
