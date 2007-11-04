#include <typeinfo>
#include <iostream>
#include <vector>
#include <utility>
#include "Hom.h"
#include "DDD.h"
#include "DED.h"
#include "UniqueTable.h"

#include <cassert>

#ifdef PARALLEL_DD
#include <tbb/concurrent_vector.h>
#include <tbb/parallel_reduce.h>
#endif

#ifdef INST_STL
MapJumps  _GHom::HomJumps;
#endif


#ifdef INST_STL
static string TryDemangle(string in)
{
  string res=string("");
  unsigned int idx=0;
  int state=0;
  int len=0;
  bool err=false;
  int depth=0;
  
  while ((idx<in.length())&&(!err)){
    switch(state){
    case 0 : //beginning
      if ((in[idx]>='0')&&(in[idx]<='9')){
	// decode length
	len=10*len+(in[idx]-'0');
      }
      else {
	// first char after length
	res=res+in[idx];
	len--;
	state=1;
      }
      break;


    case 10 : //beginning next (handle 'L')
      if ((in[idx]>='0')&&(in[idx]<='9')){
	// decode length
	len=10*len+(in[idx]-'0');
      }
      else 
	if (in[idx]=='L'){
	  state=11;
	  depth++;
	}
      else
	{
	  // first char after length
	  res=res+in[idx];
	  len--;
	  state=1;
	}
      break;
      
      
    case 1:
      if (len==0){
	// of name 
	if ((idx<in.length())&&(in[idx]!='I')&&(in[idx]!='E')){
	  err=true;
	}
	else 
	  if (in[idx]=='I') {
	    res=res+'<';
	    state=10;
	    len=0;
	    depth++;
	  }
	  else if (in[idx]=='E') {
	    depth--;
	    res=res+'>';
	  }
	  else {
	    err=true;
	  }
      }
      else {
	res=res+in[idx];
	len--;
      }

      break;

    case 11:
      if (in[idx]=='i'){
	// ok
      }
      else 
      if (in[idx]=='E'){
	// end 
	depth--;
	state=1;
      }
      else 
      if ((in[idx]>='0')&&(in[idx]<='9')) {
	res=res+in[idx];
      }
      else {
	err=true;
      }
      break;

    }
    idx++;
  }
  
  if ((depth!=0)||(err)){
    return in;
  }
  else {
    return res;
  }
   
}
#endif


void PrintMapJumps(double ratemin=0){
#ifdef INST_STL
  MapJumps::iterator ii;
  for (ii=_GHom::HomJumps.begin(); ii!=_GHom::HomJumps.end(); ii++){
    double rate= double(ii->second.second)/double (ii->second.first);
    if (rate>ratemin){
      cout << "Hom " << TryDemangle(ii->first) << "\t-->\t\t" << ii->second.second  <<"/" ;
      cout <<  ii->second.first << "= " << rate  << endl; 
    }
  }
#endif
}





/* Unique Table */
namespace __gnu_cxx {
  template<> 
  struct hash<_GHom*>{
    size_t operator()(_GHom * _h) const{
      return _h->hash();
    }
  };
}

namespace std {
  template<>
  struct equal_to<_GHom*>{
    bool operator()(_GHom * _h1,_GHom * _h2){
      return (typeid(*_h1)==typeid(*_h2)?(*_h1)==(*_h2):false);
    }
  };
}

static UniqueTable<_GHom> canonical;

/*************************************************************************/
/*                         Class _GHom                                   */
/*************************************************************************/

/************************** Identity */
class Identity:public _GHom{
public:
  /* Constructor */
  Identity(int ref=0):_GHom(ref,true){}

  /* Compare */
  bool operator==(const _GHom &h) const{return true;}
  size_t hash() const{return 17;}

  /* Eval */
  GDDD eval(const GDDD &d)const{return d;}
};

/************************** Constant */
class Constant:public _GHom{
private:
  GDDD value;
public:
  /* Constructor */
  Constant(const GDDD &d,int ref=0):_GHom(ref,true),value(d){}

  /* Compare */
  bool operator==(const _GHom &h) const{
    return value==((Constant*)&h )->value;
  }
  size_t hash() const{
    return __gnu_cxx::hash<GDDD>()(value);
  }

  /* Eval */
  GDDD eval(const GDDD &d)const{
    return d==GDDD::null?GDDD::null:value;
  }

  /* Memory Manager */
  void mark() const{
    value.mark();
  }
};

/************************** Mult */
class Mult:public _GHom{
private:
  GHom left;
  GDDD right;
public:
  /* Constructor */
  Mult(const GHom &l,const GDDD &r,int ref=0):_GHom(ref),left(l),right(r){}
  /* Compare */
  bool operator==(const _GHom &h) const{
    return left==((Mult*)&h )->left && right==((Mult*)&h )->right;
  }
  size_t hash() const{
    return 83*__gnu_cxx::hash<GHom>()(left)+53*__gnu_cxx::hash<GDDD>()(right);
  }

  /* Eval */
  GDDD eval(const GDDD &d)const{
    return left(d)*right;
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }
};

/************************** Add */
class Add:public _GHom
{

private:

  std::set<GHom> parameters;
  
public:
  
  /* Constructor */
  Add( const std::set<GHom> &param, int ref=0)
  	:
  	_GHom(ref,true),
  	parameters()
  {
    // reprendre les paramètres des unions des fils dans mon set
    for( std::set<GHom>::const_iterator it = param.begin(); it != param.end(); ++it)
    {
      if( typeid( *get_concret(*it) ) == typeid(Add) )
      {
        std::set<GHom>& local_param = ((Add*)get_concret(*it))->parameters;
        parameters.insert( local_param.begin() , local_param.end());
      }
      else
      {
        parameters.insert(*it);
      }
    }
  }
  
  

/* Compare */
  bool operator==(const _GHom &h) const
  {
    return parameters==((Add*)&h )->parameters;
  }
  
  size_t hash() const
  {
    size_t res=0;
    for(std::set<GHom>::const_iterator gi=parameters.begin();gi!=parameters.end();gi++)
      res^=__gnu_cxx::hash<GHom>()(*gi);
    return res;
  }

  /* Eval */
  GDDD eval(const GDDD &d)const{
    std::set<GDDD> s;
    for(std::set<GHom>::const_iterator gi=parameters.begin();gi!=parameters.end();gi++)
      s.insert((*gi)(d));
    return DED::add(s);
  }

  /* Memory Manager */
  void mark() const{
    for(std::set<GHom>::const_iterator gi=parameters.begin();gi!=parameters.end();gi++)
      gi->mark();
  }
};
/************************** Compose */
class Compose:public _GHom{
private:
  GHom left;
  GHom right;
public:
  /* Constructor */
  Compose(const GHom &l,const GHom &r,int ref=0):_GHom(ref,true),left(l),right(r){}
  /* Compare */
  bool operator==(const _GHom &h) const{
    return left==((Compose*)&h )->left && right==((Compose*)&h )->right;
  }
  size_t hash() const{
    return 13*__gnu_cxx::hash<GHom>()(left)+7*__gnu_cxx::hash<GHom>()(right);
  }

  /* Eval */
  GDDD eval(const GDDD &d)const{
    return left(right(d));
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }

};

/************************** LeftConcat */
class LeftConcat:public _GHom{
private:
  GDDD left;
  GHom right;

  friend class StrongHom ;
public:
  /* Constructor */
  LeftConcat(const GDDD &l,const GHom &r,int ref=0):_GHom(ref,true),left(l),right(r){}
  /* Compare */
  bool operator==(const _GHom &h) const{
    return left==((LeftConcat*)&h )->left && right==((LeftConcat*)&h )->right;
  }
  size_t hash() const{
    return 23*__gnu_cxx::hash<GDDD>()(left)+47*__gnu_cxx::hash<GHom>()(right);
  }

  /* Eval */
  GDDD eval(const GDDD &d)const{
    return left^right(d);
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }

};

/************************** LeftConcat */

class LeftArcConcat:public _GHom{
private:
  int var;
  int val;
  GHom right;

  friend class StrongHom ;
public:
  /* Constructor */
  LeftArcConcat(int vr,int vl,const GHom &r,int ref=0):_GHom(ref,true),var(vr),val(vl),right(r){}
  /* Compare */
  bool operator==(const _GHom &h) const{
    return var==((LeftArcConcat*)&h )->var && val==((LeftArcConcat*)&h )->val && right==((LeftArcConcat*)&h )->right;
  }
  size_t hash() const{
    return 23*var + val*1789 +47*__gnu_cxx::hash<GHom>()(right);
  }

  /* Eval */
  GDDD eval(const GDDD &d)const{
    return GDDD(var,val,right(d));
  }

  /* Memory Manager */
  void mark() const{
    right.mark();
  }

};

/************************** RightConcat */
class RightConcat:public _GHom{
private:
  GHom left;
  GDDD right;
public:
  /* Constructor */
  RightConcat(const GHom &l,const GDDD &r,int ref=0):_GHom(ref,true),left(l),right(r){}
  /* Compare */
  bool operator==(const _GHom &h) const{
    return left==((RightConcat*)&h )->left && right==((RightConcat*)&h )->right;
  }
  size_t hash() const{
    return 47*__gnu_cxx::hash<GHom>()(left)+19*__gnu_cxx::hash<GDDD>()(right);
  }

  /* Eval */
  GDDD eval(const GDDD &d)const{
    return left(d)^right;
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }
};

/************************** Minus */
class Minus:public _GHom{
private:
  GHom left;
  GDDD right;
public:
  /* Constructor */
  Minus(const GHom &l,const GDDD &r,int ref=0):_GHom(ref),left(l),right(r){}
  /* Compare */
  bool operator==(const _GHom &h) const{
    return left==((Minus*)&h )->left && right==((Minus*)&h )->right;
  }
  size_t hash() const{
    return 5*__gnu_cxx::hash<GHom>()(left)+61*__gnu_cxx::hash<GDDD>()(right);
  }

  /* Eval */
  GDDD eval(const GDDD &d)const{
    return left(d)-right;
  }

  /* Memory Manager */
  void mark() const{
    left.mark();
    right.mark();
  }
};

/************************** Fixpoint */
class Fixpoint:public _GHom{
private:
  GHom arg;
public:
  /* Constructor */
  Fixpoint(const GHom &a,int ref=0):_GHom(ref),arg(a){}
  /* Compare */
  bool operator==(const _GHom &h) const{
    return arg==((Fixpoint*)&h )->arg ;
  }
  size_t hash() const{
    return 17*__gnu_cxx::hash<GHom>()(arg);
  }

  /* Eval */
  GDDD eval(const GDDD &d)const{
    GDDD d1=d,d2=d;
    do {
      d1=d2;
      d2=arg(d2);
    } while (d1 != d2);

    return d1;
  }

  /* Memory Manager */
  void mark() const{
    arg.mark();
  }
};

/*************************************************************************/
/*                         Class StrongHom                               */
/*************************************************************************/

/* Compare */
bool StrongHom::operator==(const _GHom &h) const{
  return typeid(*this)==typeid(h)?*this==*(StrongHom*)&h:false;
}

// typedef tbb::concurrent_vector< std::pair<int, GDDD> > GDDD_vec;
// 
// struct reducer
// {
// 	std::set<GDDD> result_;
// 	const StrongHom& hom_;
// 	int variable_;
// 	
// 	reducer( const StrongHom& hom,
// 			 int variable)
// 		:
// 		result_(),
// 		hom_(hom),
// 		variable_(variable)
// 	{}
// 	
// 	reducer( reducer& r, tbb::split)
// 		:
// 		result_(r.result_),
// 		hom_(r.hom_),
// 		variable_(r.variable_)
// 	{}
// 	
// 	void
// 	operator()( const GDDD_vec::range_type& vec)
// 	{
// 		assert( std::distance(vec.begin(),vec.end()) == 1 );
// 		std::pair< int, GDDD > element = *vec.begin();
// 		result_.insert( hom_.phi( variable_, element.first)(element.second) );
// 	}
// 	
// 	void
// 	join(const reducer& r)
// 	{
// 		this->result_.insert(r.result_.begin(), r.result_.end());
// 	}
// };


static bool valOrder (const std::pair<int,GDDD > & a,const std::pair<int,GDDD > & b) {
  return a.first < b.first ;
}

static int count_larc = 0;
/* Eval */
GDDD StrongHom::eval(const GDDD &d)const{
  if(d==GDDD::null)
    return GDDD::null;
  else if(d==GDDD::one)
    return phiOne();
  else if(d==GDDD::top)
    return GDDD::top;
  
  else{
    int variable=d.variable();
    //      std::set<GDDD> s;
    //      for(GDDD::const_iterator vi=d.begin();vi!=d.end();vi++){
    //        s.insert(phi(variable,vi->first)(vi->second));
    //      }
    //      return DED::add(s);
    typedef std::set< GDDD > listType;
    typedef std::map<int , listType > canoMap;
    // for pathological cases LeftArcConcat
    canoMap toCanonize;
    // for general case
    std::set<GDDD> others;
    
    // for every arc of d
    for(GDDD::const_iterator vi=d.begin();vi!=d.end();vi++){
      // store phi to see if we can optimize
      const GHom & phiTmp =  phi(variable,vi->first);
      
      // pathological case, a LeftArcConcat. Add to toCanonize list 
      if ( typeid(* get_concret(phiTmp)) == typeid(LeftArcConcat) ) {
	// downcast
	const LeftArcConcat * lc = (const LeftArcConcat *) get_concret(phiTmp);
	// consistency check : we expect the Hom to return the same variable with a new value.
	if (lc->var ==  variable) {
	  
	  GDDD son = lc->right (vi->second);
	  if (son != GDDD::null) {
	    // Canonic insertion in toCanonize
	    canoMap::iterator it = toCanonize.find(lc->val);
	    if (it != toCanonize.end() ) {
	      // value already represented, augment list used to produce son node 
	      it->second.insert(son);
	      continue ;
	    } else {
	      // a new arc value, add new entry to toCanonize
	      listType arcs ;
	      arcs.insert(son);
	      toCanonize.insert(std::make_pair(lc->val, arcs));
	      continue ;
	    }
	  } else {
	    // arcs to GDDD::null are not represented
	    continue ;
	  }
       	}
      }
      // we get here if we did not hit any "continue" instruction
      // if this is not LeftArcConcat or consistency check var'=var failed.
      // fallback into general case.
      others.insert(phiTmp (vi->second));
    }
    // finished exploring arcs of d
    // if we have captured an optimized temporary node
    if (! toCanonize.empty()) {
      GDDD::Valuation canoRes ;
      // note that canoMap is sorted by key, so this iteration produces
      // elements in the Valuation canoRes appropriately sorted by arc value
      // this constraint is necessary, see comments in DDD.h on GDDD::GDDD(int var, Valuation v).
      for (canoMap::const_iterator it = toCanonize.begin() ; it != toCanonize.end() ; ++it ) {
	// compute resulting son by summing the list of son nodes
	// then add the arc to node under construction valuation
	canoRes.push_back(std::make_pair(it->first, DED::add(it->second)));
      }
      // useless already done by the map
      //           sort(canoRes.begin(),canoRes.end(),valOrder);

      // construct the node and add it to general case.
      others.insert(GDDD(variable,canoRes));
      ++count_larc;
    }
    if (others.empty())
      return GDDD::null ;
    else
      return DED::add(others);
  }
}

    // else  
	// {
	// 	
	// 	int variable = d.variable();
	// 
	// 	GDDD_vec successors;
	// 	for( GDDD::Valuation::const_iterator it = d.begin();
	// 		 it != d.end();
	// 		 ++it)
	// 	{
	// 		successors.push_back(*it);
	// 	}
	// 	
	// 	
	// 	reducer red_is_dead(*this,variable);
	// 	tbb::parallel_reduce(successors.range(1), red_is_dead);
	// 	
	// 	return DED::add(red_is_dead.result_);
	// 	
	// }



/*************************************************************************/
/*                         Class GHom                                    */
/*************************************************************************/

/* Constructor */
GHom::GHom(const StrongHom *h):concret(h){}

GHom::GHom(StrongHom *h):concret(canonical(h)){}

GHom::GHom(const GDDD& d):concret(canonical(new Constant(d))){}

GHom::GHom(int var, int val, const GHom &h):concret(canonical(new LeftArcConcat(var,val,h))){}

/* Eval */
GDDD GHom::operator()(const GDDD &d) const{
  if(concret->immediat)
    return concret->eval(d);
  else
    return DED::hom(*this,d);
}

GDDD GHom::eval(const GDDD &d) const{
  return concret->eval(d);
}

const GHom GHom::id(canonical(new Identity(1)));

int GHom::refCounter() const{return concret->refCounter;}

/* Sum */

GHom GHom::add(const std::set<GHom>& s){
  return(new Add(s));
}


/* Memory Manager */
unsigned int GHom::statistics(){
  return canonical.size();
}

// Todo
void GHom::mark()const{
  if(!concret->marking){
    concret->marking=true;
    concret->mark();
  }
};

void GHom::garbage(){
  // mark phase
  for(UniqueTable<_GHom>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();di++){
    if((*di)->refCounter!=0){
      (*di)->marking=true;
      (*di)->mark();
    }
  }
  // sweep phase
  for(UniqueTable<_GHom>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();){
    if(!((*di)->marking)){
      UniqueTable<_GHom>::Table::iterator ci=di;
      di++;
      _GHom *g=(*ci);
      canonical.table.erase(ci);
      delete g;
    }
    else{
      (*di)->marking=false;
      di++;
    }
  }
}

/*************************************************************************/
/*                    Class Hom                                          */
/*************************************************************************/
/* Constructor */
Hom::Hom(const Hom &h):GHom(h.concret){
  concret->refCounter++;
}

Hom::Hom(const GHom &h):GHom(h.concret){
  concret->refCounter++;
}

Hom::Hom(const GDDD& d):GHom(d){
  concret->refCounter++;
}

Hom::Hom(int var, int val, const GHom &h):GHom(var,val,h){
  concret->refCounter++;
}

Hom::~Hom(){
  assert(concret->refCounter>0);
  concret->refCounter--;
}

/* Set */

Hom &Hom::operator=(const Hom &h){
  assert(concret->refCounter>0);
  concret->refCounter--;
  concret=h.concret;
  concret->refCounter++;
  return *this;
}

Hom &Hom::operator=(const GHom &h){
  assert(concret->refCounter>0);
  concret->refCounter--;
  concret=h.concret;
  concret->refCounter++;
  return *this;
}

/* Operations */
GHom fixpoint (const GHom &h) {
  if (h != GHom::id)
    return GHom(canonical(new Fixpoint(h)));
  else
    return GHom::id;
}

GHom operator&(const GHom &h1,const GHom &h2){
  return GHom(canonical(new Compose(h1,h2)));
}

GHom operator+(const GHom &h1,const GHom &h2){
  std::set<GHom> s;
  s.insert(h1);
  s.insert(h2);
//  return(new Add(s));
  return GHom(canonical(new Add(s)));
}

GHom operator*(const GDDD &d,const GHom &h){
  return GHom(canonical(new Mult(h,d)));
}

GHom operator*(const GHom &h,const GDDD &d){
  return GHom(canonical(new Mult(h,d)));
}

GHom operator^(const GDDD &d,const GHom &h){
  // optimize pathologic case, just one arc left concatenated 
  if (d.nbsons() == 1) {
    GDDD::const_iterator it = d.begin();
    if (it->second == GDDD::one)
      return GHom(canonical(new LeftArcConcat(d.variable(),it->first,h)));
  }
  return GHom(canonical(new LeftConcat(d,h)));
}

GHom operator^(const GHom &h,const GDDD &d){
  return GHom(canonical(new RightConcat(h,d)));
}

GHom operator-(const GHom &h,const GDDD &d){
  return GHom(canonical(new Minus(h,d)));
}

/*************************************************************************/
/*                         Class MyGHom                                    */  
/*************************************************************************/

/* Constructor */
GHom::GHom(const MyGHom *h):concret(h){}

GHom::GHom(MyGHom *h):concret(canonical(h)){}



void GHom::pstats(bool reinit)
{
  std::cout << "*\nGHom Stats : size unicity table = " <<  canonical.size() << std::endl;
  std::cout << "LeftArcConcats caught : " << count_larc <<std::endl;
#ifdef INST_STL
  canonical.pstat(reinit);
  
  std::cout << "\n ----------------  MAPPING Jumps on GHOM --------------" << std::endl;
  PrintMapJumps();
  if (reinit){
    std::cout << "\n -----  END MAPPING Jumps on GHOM, reseting table -----" << std::endl;
    _GHom::HomJumps.clear();
  }
  else
    {
      cout << "\n -----  END MAPPING Jumps on GHOM   --------" << endl; 
    }
#endif
}
