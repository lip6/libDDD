/* -*- C++ -*- */
#ifndef DDD_H
#define DDD_H
#include <string>
#include <iostream>
#include <vector>
#include <set>
// modif
#include <ext/hash_map>
// ajout
using namespace std;
using namespace __gnu_cxx;

class _GDDD;

/******************************************************************************/
class GDDD{
private:
  friend struct hash<GDDD>;
  friend ostream& operator<<(ostream &os,const GDDD &g);
  friend class DDD;
  _GDDD *concret;
  GDDD(_GDDD *_g);
  void print(ostream& os,string s) const;
public:
  /* Accessors */
  typedef vector<pair<int,GDDD> > Valuation;
  typedef Valuation::const_iterator const_iterator;
  int variable() const;
  const_iterator begin() const;
  const_iterator end() const;
  
  /* Constructeurs */
  GDDD(int variable,Valuation value);
  GDDD():concret(null.concret){};
  GDDD(int var,int val,const GDDD &d=one ); //var-val->d
  GDDD(int var,int val1,int val2,const GDDD &d=one); //var-[val1,var2]->d

  /* Constants */
  static const GDDD one;
  static const GDDD null;
  static const GDDD top;

  /* Compare */
  bool operator==(const GDDD& g) const{return concret==g.concret;};
  bool operator!=(const GDDD& g) const{return concret!=g.concret;};
  bool operator<(const GDDD& g) const{return concret<g.concret;};

  /* Visualisation */ 
  unsigned int refCounter() const;
  unsigned long int size() const;
  size_t nbsons () const;
  long double nbStates() const;
  long double noSharedSize() const;
  static void varName(int,const string &);
  static const string getvarName(int var);

  /* Memory Manager */
  static  unsigned int statistics();
  void mark()const;
  static void garbage(); 
  static void pstats(bool reinit=true);
};

ostream& operator<<(ostream &,const GDDD &);
 
/******************************************************************************/
class DDD:public GDDD {
public:
  /* Constructeur */
  DDD(const DDD &);
  DDD(const GDDD &g=GDDD::null);
  DDD(int var,int val,const GDDD &d=one ); //var-val->d
  DDD(int var,int val1,int val2,const GDDD &d=one); //var-[val1,var2]->d
  ~DDD(); 

  /* Set */
  DDD &operator=(const GDDD&);
  DDD &operator=(const DDD&);
};

/******************************************************************************/
namespace __gnu_cxx {
	struct hash<GDDD> {
		size_t operator()(const GDDD &g) const{
			return (size_t) g.concret;
		}
	};
}

namespace std {
	struct equal_to<GDDD> {
		bool operator()(const GDDD &g1,const GDDD &g2) const{
			return g1==g2;
		}
	};
}

namespace std {
	struct less<GDDD> {
		bool operator()(const GDDD &g1,const GDDD &g2) const{
			return g1<g2;
		}
	};
}

#endif






