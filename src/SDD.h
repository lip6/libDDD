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
#include "hashfunc.hh"

class _GSDD;

/******************************************************************************/
class GSDD {
private:
  friend struct hash<GSDD>;
  friend ostream& operator<<(ostream &os,const GSDD &g);
  friend class SDD;
  friend class _GSDD;
  _GSDD *concret;
  
  void print(ostream& os,string s) const;
public:
  GSDD(_GSDD *_g);
#ifdef OTF_GARBAGE
  virtual ~GSDD () ;
#endif
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
#ifdef OTF_GARBAGE
  GSDD(const GSDD &);
  GSDD & operator=(const GSDD &);
#endif
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

#ifdef OTF_GARBAGE
  // to check out useless intermediate nodes : on the fly garbage collection
  // WARNING : this is not const at all in fact it deletes "concret" and is quite dangerous,
  // only for expert use not a basic library functionality
  void clearNode() const;
  void markAsSon() const;
  bool isSon() const;
#endif // OTF_GARBAGE

  static void pstats(bool reinit=true);
  static size_t peak();
};


ostream& operator<<(ostream &,const GSDD &);
/* Binary operators */
GSDD operator^(const GSDD&,const GSDD&); // concatenation
GSDD operator+(const GSDD&,const GSDD&); // union
GSDD operator*(const GSDD&,const GSDD&); // intersection
GSDD operator-(const GSDD&,const GSDD&); // difference

 
/******************************************************************************/
class SDD:public GSDD,public DataSet {
public:
  /* Constructeur */
  SDD(const SDD &);
  SDD(const GSDD &g=GSDD::null);
  SDD(int var,const DataSet& val,const GSDD &d=one ); //var-val->d

  virtual ~SDD(); 

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

// not very nice to access unicity table directly
#include "UniqueTable.h"
namespace SDDutil {
#ifdef OTF_GARBAGE
  void recentGarbage();
#endif // OTF_GARBAGE
  UniqueTable<_GSDD> * getTable ();
  void foreachTable (void (*foo) (const GSDD & g)); 
}


/******************************************************************************/
namespace __gnu_cxx {
  template<>	struct hash<GSDD> {
		size_t operator()(const GSDD &g) const{
		  //return (size_t) g.concret;
		  return ddd::knuth32_hash(reinterpret_cast<const size_t>(g.concret));
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


