/* -*- C++ -*- */
#ifndef HOM_H_
#define HOM_H_

#include "DDD.h"

class _GHom;
class StrongHom;

class GHom {
private:
  friend class Hom;
  friend struct hash<GHom>;
  friend GHom operator+(const GHom &,const GHom &); 
  friend GHom operator&(const GHom &,const GHom &); // composition
  friend GHom operator*(const GDDD &,const GHom &); 
  friend GHom operator*(const GHom &,const GDDD &); 
  friend GHom operator^(const GDDD &,const GHom &); 
  friend GHom operator^(const GHom &,const GDDD &); 
  friend GHom operator-(const GHom &,const GDDD &); 
  const _GHom* concret;
  GHom(const _GHom *_h):concret(_h){};
public:
  /* Constructor */
  GHom():concret(id.concret){};
  GHom(const StrongHom *);
  GHom(StrongHom *);
  GHom(const GDDD& d);   // constant
  GHom(int var, int val, const GHom &h=GHom::id);  // var -- val -> Id

  /* Elementary homomorphisms */
  static const GHom id;

  /* Compare */
  bool operator==(const GHom &h) const{return concret==h.concret;};
  bool operator!=(const GHom &h) const{return concret!=h.concret;};
  bool operator<(const GHom &h) const{return concret<h.concret;};

  /* evaluation */ 
  GDDD operator()(const GDDD &d) const; // evaluation
  GDDD eval(const GDDD &d) const;

  /* visualisation*/
  int refCounter() const;

  /* Operations */
  static GHom add(const set<GHom>&);

  /* Memory Manager */
  static  unsigned int statistics();
  void mark()const;
  static void garbage(); 
};



/* Operations */

GHom operator+(const GHom &,const GHom &); 
GHom operator&(const GHom &,const GHom &); // composition
GHom operator*(const GDDD &,const GHom &); 
GHom operator*(const GHom &,const GDDD &); 
GHom operator^(const GDDD &,const GHom &); 
GHom operator^(const GHom &,const GDDD &); 
GHom operator-(const GHom &,const GDDD &); 


class Hom:public GHom{
public:
  /* Constructor */
  Hom(const GHom &h=GHom::id);
  Hom(const Hom &h);
  Hom(const GDDD& d);   // constant
  Hom(int var, int val, const GHom &h=GHom::id);  // var -- val -> Id
  ~Hom();

  /* Set */
  
  Hom &operator=(const GHom &);
  Hom &operator=(const Hom &);
};

/******************************************************************************/
namespace __gnu_cxx {
  struct hash<GHom> {
    size_t operator()(const GHom &g) const{
      return (size_t) g.concret;
    }
  };
}

namespace std {
  struct equal_to<GHom> {
    bool operator()(const GHom &g1,const GHom &g2) const{
      return g1==g2;
    }
  };
}

namespace std {
  struct less<GHom> {
    bool operator()(const GHom &g1,const GHom &g2) const{
      return g1<g2;
    }
  };
}

/**********************************************************************/
class _GHom{
private:
  friend class GHom;
  friend class Hom;
  mutable int refCounter;
  mutable bool marking;
  mutable bool immediat;
public:
  /* Destructor*/
  _GHom(int ref=0,bool im=false):refCounter(ref),marking(false),immediat(im){};
  virtual ~_GHom(){};

  /* Compare */
  virtual bool operator==(const _GHom &h) const=0;
  virtual size_t hash() const=0;

  /* Eval */
  virtual GDDD eval(const GDDD &)const=0;

  /* Memory Manager */
  virtual void mark() const{};
};

class StrongHom:public _GHom{
public:
  StrongHom(){};
  virtual ~StrongHom(){};
  virtual GDDD phiOne() const{return GDDD::top;}; 
  virtual GHom phi(int var,int val) const=0; 
  virtual bool operator==(const StrongHom &h) const=0;
  bool operator==(const _GHom &h) const;

  //Eval
  GDDD eval(const GDDD &)const;  
};
 
#endif /* HOM_H_ */







