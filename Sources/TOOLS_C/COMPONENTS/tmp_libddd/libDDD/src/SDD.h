/* -*- C++ -*- */
#ifndef SDD_H
#define SDD_H
#include <string>
#include <iostream>
#include <vector>
#include <set>
// modif
#include <ext/hash_map>  //c'e
// ajout
using namespace std;
using namespace __gnu_cxx;

#include "DDD.h"
#include "DataSet.h"


class _GSDD;

/******************************************************************************/
class GSDD {
private:
  friend struct hash<GSDD>;
  friend ostream& operator<<(ostream &os,const GSDD &g);
  friend class SDD;
  _GSDD *concret;
  GSDD(_GSDD *_g);
  void print(ostream& os,string s) const;
public:
  /* Accessors */
  typedef vector<pair<DataSet *,GSDD> > Valuation;
  typedef Valuation::const_iterator const_iterator;
  int variable() const;
  const_iterator begin() const;
  const_iterator end() const;
  
  /* Constructeurs */
  GSDD(int variable,Valuation value);
  GSDD():concret(null.concret){};
  GSDD(int var,const DataSet & val,const GSDD &d=one ); //var-val->d

  /* Constants */
  static const GSDD one;
  static const GSDD null;
  static const GSDD top;

  /* Compare */
  bool operator==(const GSDD& g) const{return concret==g.concret;};
  bool operator!=(const GSDD& g) const{return concret!=g.concret;};
  bool operator<(const GSDD& g) const{return concret<g.concret;};

  /* Visualisation */ 
  unsigned int refCounter() const;
  unsigned long int size() const;
  // return a pair <SDD nodes,DDD nodes>
  pair<unsigned long int,unsigned long int> node_size() const;
  size_t nbsons () const;
  long double GSDD::nbStates() const;

//  Broken right now, dont use me or fixme first
//  long double GSDD::noSharedSize() const;


  /* Memory Manager */
  static  unsigned int statistics();
  void mark()const;
  static void garbage(); 
  static void pstats(bool reinit=true);
  static size_t peak();
};

ostream& operator<<(ostream &,const GSDD &);
 
/******************************************************************************/
class SDD:public GSDD,public DataSet {
public:
  /* Constructeur */
  SDD(const SDD &);
  SDD(const GSDD &g=GSDD::null);
  SDD(int var,const DataSet& val,const GSDD &d=one ); //var-val->d

  ~SDD(); 

  /* Set */
  SDD &operator=(const GSDD&);
  SDD &operator=(const SDD&);

  // DataSet interface
  virtual DataSet *newcopy () const { return new SDD(*this); }
  virtual DataSet *set_intersect (const DataSet & b) const  ;
  virtual DataSet *set_union (const DataSet & b)  const ;
  virtual DataSet *set_minus (const DataSet & b) const;
  virtual bool empty() const ;
  virtual DataSet *empty_set()const;
  virtual bool set_equal(const DataSet & b) const;
  virtual long double set_size() const;
  virtual size_t set_hash() const ;
  void set_print (ostream &os) const { os << *this; }
};

/******************************************************************************/
namespace __gnu_cxx {
  template<>	struct hash<GSDD> {
		size_t operator()(const GSDD &g) const{
			return (size_t) g.concret;
		}
	};
}

namespace std {
  template<>	struct equal_to<GSDD> {
		bool operator()(const GSDD &g1,const GSDD &g2) const{
			return g1==g2;
		}
	};
}

namespace std {
  template<>	struct less<GSDD> {
		bool operator()(const GSDD &g1,const GSDD &g2) const{
			return g1<g2;
		}
	};
}

#endif


