#include "MLSHom.h"

#include "UniqueTable.h"
#include <typeinfo>

namespace d3 { namespace util {
  template<>
  struct equal<_MLShom*>{
    bool operator()(_MLShom * _h1,_MLShom * _h2){
      return (typeid(*_h1)==typeid(*_h2)?(*_h1)==(*_h2):false);
    }
  };
}}

static UniqueTable<_MLShom> canonical;

/************* Class _MLShom *******************/

namespace nsMLShom {

class Identity : public _MLShom {
public:
  /* Constructor */
  Identity(int ref=0):_MLShom(ref){}
  
  virtual bool shouldCache () const { return false ; }
  
  /* Compare */
  bool operator==(const _MLShom&) const{ return true; }
  size_t hash() const { return 8291; }
  
  _MLShom * clone () const { return new Identity(*this) ; }
  
  bool
  skip_variable(int) const 
  {
    return true;
  }
  
  /* Eval */
  SHomNodeMap eval(const GSDD &d) const { 
    SHomNodeMap m; 
    m.add(GShom::id,d); 
    return m; 
  }
};
  
class Add:public _MLShom{
  std::set<MLShom> parameters;
public:
  /* Constructor */
  Add(const std::set<MLShom> & s,int ref=0):_MLShom(ref),parameters(s){}
  
  /* Compare */
  bool operator==(const _MLShom &h) const{ 
    return parameters==((Add*)&h )->parameters;
  }
  _MLShom * clone () const { return new Add(*this) ; }
  size_t hash() const { 
    size_t res=0;
    for(std::set<MLShom>::const_iterator gi=parameters.begin();gi!=parameters.end();++gi)
    {
      res ^= gi->hash();
    }
    return res;
  }
  
  bool
  skip_variable(int) const 
  {
    return false;
  }
  
  /* Eval */
  SHomNodeMap eval(const GSDD &d) const { 
    SHomNodeMap m; 
    for(std::set<MLShom>::const_iterator gi=parameters.begin();gi!=parameters.end();++gi)    
      m.addAll( (*gi)(d)); 
    return m; 
  }
};

class GShomAdapter:public _MLShom{
  GShom h;
public:
  /* Constructor */
  GShomAdapter(const GShom & _h,int ref=0):_MLShom(ref),h(_h){}
  
  /* Compare */
  bool operator==(const _MLShom &other) const{ 
    return h==((GShomAdapter*)&other )->h;
  }
  size_t hash() const { 
    return  17449*h.hash();
  }
  _MLShom * clone () const { return new GShomAdapter(*this) ; }
  
  bool
  skip_variable(int var) const 
  {
    return h.skip_variable(var);
  }
  
  /* Eval */
  SHomNodeMap eval(const GSDD &d) const { 
    SHomNodeMap m; 
    m.add(GShom::id,h(d)); 
    return m; 
  }
};

class ConstantUp:public _MLShom {
  GShom  up;
  MLShom down;
public :
  ConstantUp(const GShom & uup,const MLShom & ddown):up(uup),down(ddown){}
  
  /* Compare */
  bool operator==(const _MLShom &other) const{ 
    return up==((ConstantUp*)&other )->up && down==((ConstantUp*)&other )->down;
  }
  _MLShom * clone () const { return new ConstantUp(*this) ; }
  size_t hash() const { 
    return  10159*(up.hash()^(down.hash()+37));
  }
  
  bool
  skip_variable(int) const 
  {
    return false;
  }
  
  SHomNodeMap eval(const GSDD &d) const { 
    SHomNodeMap m = down(d);
    SHomNodeMap res;
    for (SHomNodeMap::const_iterator it = m.begin() ; it != m.end() ; ++it ){
      res.add(up & it->first, it->second ); 
    }
    return res; 
  }
  
};

class LeftConcat:public _MLShom{
  GSDD left;
  MLShom h;
public:
  /* Constructor */
  LeftConcat(const GSDD & l, const MLShom & _h,int ref=0):_MLShom(ref),left(l),h(_h){}
  
  /* Compare */
  bool operator==(const _MLShom &other) const{ 
    return h==((LeftConcat*)&other )->h && left==((LeftConcat*)&other )->left;
  }
  _MLShom * clone () const { return new LeftConcat(*this) ; }
  size_t hash() const { 
    return  19471*(h.hash()^left.hash());
  }
  
  bool
  skip_variable(int) const 
  {
    return false;
  }
  
  /* Eval */
  SHomNodeMap eval(const GSDD &d) const { 
    SHomNodeMap m = h(d);
    SHomNodeMap res;
    for (SHomNodeMap::const_iterator it = m.begin() ; it != m.end() ; ++it ){
      res.add(it->first, left ^ it->second ); 
    }
    return res; 
  }
};

} // namespace nsMLShom

using namespace nsMLShom;

/************* Class MLShom *******************/

const MLShom MLShom::id(canonical(Identity(1)));

MLShom::~MLShom () {};
MLShom::MLShom (const _MLShom *h):concret(h){};
MLShom::MLShom (const _MLShom &h):concret(canonical(h)){};

MLShom::MLShom (const GShom & up, const MLShom & down):concret(canonical( ConstantUp(up,down))){}
MLShom::MLShom (const GShom &h):concret (canonical( GShomAdapter(h))) {}


MLShom::MLShom (int var, const DataSet &val, const MLShom &h):concret(canonical( LeftConcat(GSDD(var,val),h))){}

SHomNodeMap MLShom::operator() (const GSDD & d) const {
  return concret->eval(d);
}

/************* Class StrongMLShom ***************/


bool StrongMLShom::operator==(const _MLShom &h) const {
  return typeid(*this)==typeid(h)?*this==*(StrongMLShom*)&h:false;
}


SHomNodeMap StrongMLShom::eval(const GSDD &d) const {
  SHomNodeMap res;

  if (d == GSDD::top || d == GSDD::null) {
    res.add(GShom::id, d);
    //    std::cerr << "MLHom array out of bounds !!" << std::endl;
    //    print(std::cerr);
    //    exit(1);
    return res;
  } else if (d== GSDD::one) {    
    return phiOne();
  }
  
  int var = d.variable();
  
  for (GSDD::const_iterator dit = d.begin() ; dit != d.end() ; ++dit) {   
    
    SHomHomMap phires = phi(var,*dit->first);
    for (SHomHomMap::const_iterator homit = phires.begin() ; homit!= phires.end() ; ++homit) {
      SHomNodeMap down = homit->second(dit->second); 
      
      for (SHomNodeMap::const_iterator downit = down.begin() ; downit != down.end() ; ++downit) {
        res.add(homit->first & downit->first, downit->second);
      }
    }
  }
  return res;
  
}

/********* operators *****************/

MLShom operator+(const MLShom &h1,const MLShom &h2){
  d3::set<MLShom>::type s;
  s.insert(h1);
  s.insert(h2);
  return canonical( Add(s));
}

void MLShom::garbage(){
  // mark phase
  for(UniqueTable<_MLShom>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();++di){
    if((*di)->refCounter!=0){
      (*di)->marking=true;
      (*di)->mark();
    }
  }
  // sweep phase
  for(UniqueTable<_MLShom>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();){
    if(!(*di)->marking){
      UniqueTable<_MLShom>::Table::iterator ci=di;
      di++;
      const _MLShom *g=*ci;
      canonical.table.erase(ci);
      delete g;
    }
    else{
      (*di)->marking=false;
      di++;
    }
  }
}
