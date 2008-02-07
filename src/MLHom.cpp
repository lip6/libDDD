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

#include "MLHom.h"
#include "UniqueTable.h"

/* Unique Table */
namespace __gnu_cxx {
  template<> 
  struct hash<_MLHom*>{
    size_t operator()(_MLHom * _h) const{
      return _h->hash();
    }
  };
}

namespace std {
  template<>
  struct equal_to<_MLHom*>{
    bool operator()(_MLHom * _h1,_MLHom * _h2){
      return (typeid(*_h1)==typeid(*_h2)?(*_h1)==(*_h2):false);
    }
  };
}

static UniqueTable<_MLHom> canonical;


/*************************************************************************/
/*                         Class _GHom                                   */
/*************************************************************************/

namespace nsMLHom {

/************************** Identity */
class Identity:public _MLHom{
public:
  /* Constructor */
  Identity(int ref=0):_MLHom(ref){}

  virtual bool shouldCache () const { return false ; }

  /* Compare */
  bool operator==(const _MLHom &h) const{ return true; }
  size_t hash() const { return 8291; }

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




} // namespace MLHom

using namespace nsMLHom;

const MLHom MLHom::id(canonical(new Identity(1)));


MLHom::MLHom (const _MLHom *h):concret(h){};
