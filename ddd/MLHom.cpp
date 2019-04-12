/****************************************************************************/
/*								            */
/* This file is part of libDDD, a library for manipulation of DDD and SDD.  */
/*     						                            */
/*     Copyright (C) 2001-2008 Yann Thierry-Mieg, Jean-Michel Couvreur      */
/*                             and Denis Poitrenaud                         */
/*     						                            */
/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU Lesser General Public License as       */
/*     published by the Free Software Foundation; either version 3 of the   */
/*     License, or (at your option) any later version.                      */
/*     This program is distributed in the hope that it will be useful,      */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/*     GNU LEsserGeneral Public License for more details.                   */
/*     						                            */
/* You should have received a copy of the GNU Lesser General Public License */
/*     along with this program; if not, write to the Free Software          */
/*Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*     						                            */
/****************************************************************************/

#include "ddd/MLHom.h"
#include "ddd/UniqueTable.h"
#include <typeinfo>
#include "ddd/util/set.hh"
#include "ddd/MLCache.hh"

namespace d3 { namespace util {
  template<>
  struct equal<_MLHom*>{
    bool operator()(_MLHom * _h1,_MLHom * _h2){
      return (typeid(*_h1)==typeid(*_h2)?(*_h1)==(*_h2):false);
    }
  };
}}

static UniqueTable<_MLHom> canonical;


/*************************************************************************/
/*                         Class _GHom                                   */
/*************************************************************************/

namespace nsMLHom {

  typedef MLCache< MLHom, GDDD, HomNodeMap > MLHomCache;
  static MLHomCache mlcache;

/************************** Identity */
class Identity:public _MLHom{
public:
  /* Constructor */
  Identity(int ref=0):_MLHom(ref){}

  virtual bool shouldCache () const { return false ; }

  /* Compare */
  bool operator==(const _MLHom&) const{ return true; }
  size_t hash() const { return 8291; }

  _MLHom * clone () const { return new Identity(*this) ; }

  bool
  skip_variable(int) const 
  {
    return true;
  }

  /* Eval */
  HomNodeMap eval(const GDDD &d) const { 
    HomNodeMap m; 
    m.add(GHom::id,d); 
    return m; 
  }
};

class Add:public _MLHom{
  std::set<MLHom> parameters;
public:
  /* Constructor */
  Add(const std::set<MLHom> & s,int ref=0):_MLHom(ref),parameters(s){}

  /* Compare */
  bool operator==(const _MLHom &h) const{ 
    return parameters==((Add*)&h )->parameters;
  }
  _MLHom * clone () const { return new Add(*this) ; }
  size_t hash() const { 
    size_t res=0;
    for(std::set<MLHom>::const_iterator gi=parameters.begin();gi!=parameters.end();++gi)
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
  HomNodeMap eval(const GDDD &d) const { 
    HomNodeMap m; 
    for(std::set<MLHom>::const_iterator gi=parameters.begin();gi!=parameters.end();++gi)    
      m.addAll( (*gi)(d)); 
    return m; 
  }
};


class GHomAdapter:public _MLHom{
  GHom h;
public:
  /* Constructor */
  GHomAdapter(const GHom & _h,int ref=0):_MLHom(ref),h(_h){}

  /* Compare */
  bool operator==(const _MLHom &other) const{ 
    return h==((GHomAdapter*)&other )->h;
  }
  size_t hash() const { 
    return  17449*h.hash();
  }
  _MLHom * clone () const { return new GHomAdapter(*this) ; }

  bool
  skip_variable(int var) const 
  {
    return h.skip_variable(var);
  }

  /* Eval */
  HomNodeMap eval(const GDDD &d) const { 
    HomNodeMap m; 
    m.add(GHom::id,h(d)); 
    return m; 
  }
};

class ConstantUp:public _MLHom {
  GHom  up;
  MLHom down;
public :
  ConstantUp(const GHom & uup,const MLHom & ddown):up(uup),down(ddown){}

  /* Compare */
  bool operator==(const _MLHom &other) const{ 
    return up==((ConstantUp*)&other )->up && down==((ConstantUp*)&other )->down;
  }
  _MLHom * clone () const { return new ConstantUp(*this) ; }
  size_t hash() const { 
    return  10159*(up.hash()^(down.hash()+37));
  }

  bool
  skip_variable(int) const 
  {
    return false;
  }

  HomNodeMap eval(const GDDD &d) const { 
    HomNodeMap m = down(d);
    HomNodeMap res;
    for (HomNodeMap::const_iterator it = m.begin() ; it != m.end() ; ++it ){
      res.add(up.compose(it->first), it->second ); 
    }
    return res; 
  }

};

class LeftConcat:public _MLHom{
  GDDD left;
  MLHom h;
public:
  /* Constructor */
  LeftConcat(const GDDD & l, const MLHom & _h,int ref=0):_MLHom(ref),left(l),h(_h){}

  /* Compare */
  bool operator==(const _MLHom &other) const{ 
    return h==((LeftConcat*)&other )->h && left==((LeftConcat*)&other )->left;
  }
  _MLHom * clone () const { return new LeftConcat(*this) ; }
  size_t hash() const { 
    return  19471*(h.hash()^left.hash());
  }

  bool
  skip_variable(int) const 
  {
    return false;
  }

  /* Eval */
  HomNodeMap eval(const GDDD &d) const { 
    HomNodeMap m = h(d);
    HomNodeMap res;
    for (HomNodeMap::const_iterator it = m.begin() ; it != m.end() ; ++it ){
      res.add(it->first, left ^ it->second ); 
    }
    return res; 
  }
};



} // namespace MLHom

using namespace nsMLHom;


/************* Class MLHom *******************/

const MLHom MLHom::id(canonical(Identity(1)));

MLHom::~MLHom () {};
MLHom::MLHom (const _MLHom *h):concret(h){};
MLHom::MLHom (const _MLHom &h):concret(canonical(h)){};

MLHom::MLHom (const GHom & up, const MLHom & down):concret(canonical( ConstantUp(up,down))){}
MLHom::MLHom (const GHom &h):concret (canonical( GHomAdapter(h))) {}


MLHom::MLHom (int var, int val, const MLHom &h):concret(canonical( LeftConcat(GDDD(var,val),h))){}

HomNodeMap MLHom::eval(const GDDD &d) const  {
  return concret->eval(d);
}

HomNodeMap MLHom::operator() (const GDDD & d) const {
//   if (d == DDD::null)
//     return HomNodeMap::null;
  return  nsMLHom::mlcache.insert(*this,d).second;
}

/************* Class StrongMLHom ***************/


bool StrongMLHom::operator==(const _MLHom &h) const {
    return typeid(*this)==typeid(h)?*this==*(StrongMLHom*)&h:false;
}


HomNodeMap StrongMLHom::eval(const GDDD &d) const {
  HomNodeMap res;

  if (d == GDDD::top || d == GDDD::null) {
    res.add(GHom::id, d);
//    std::cerr << "MLHom array out of bounds !!" << std::endl;
//    print(std::cerr);
//    exit(1);
    return res;
  } else if (d== GDDD::one) {    
    return phiOne();
  }

  int var = d.variable();

  for (GDDD::const_iterator dit = d.begin() ; dit != d.end() ; ++dit) {   

    HomHomMap phires = phi(var,dit->first);
    for (HomHomMap::const_iterator homit = phires.begin() ; homit!= phires.end() ; ++homit) {
      HomNodeMap down = homit->second(dit->second); 
      
      for (HomNodeMap::const_iterator downit = down.begin() ; downit != down.end() ; ++downit) {
	res.add(homit->first.compose(downit->first), downit->second);
      }
    }
  }
  return res;

}

/********* operators *****************/

MLHom operator+(const MLHom &h1,const MLHom &h2){
  d3::set<MLHom>::type s;
  s.insert(h1);
  s.insert(h2);
  return canonical( Add(s));
}

void MLHom::garbage(){
  // clear operation cache
  mlcache.clear();
  // mark phase
  for(UniqueTable<_MLHom>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();++di){
    if((*di)->refCounter!=0){
      (*di)->marking=true;
      (*di)->mark();
    }
  }
  // sweep phase
  for(UniqueTable<_MLHom>::Table::iterator di=canonical.table.begin();di!=canonical.table.end();){
    if(!(*di)->marking){
      UniqueTable<_MLHom>::Table::iterator ci=di;
      ++di;
      const _MLHom *g=*ci;
      canonical.table.erase(ci);
      delete g;
    }
    else{
      (*di)->marking=false;
      ++di;
    }
  }
}
