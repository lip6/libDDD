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

/* -*- C++ -*- */
#ifndef DED_H
#define DED_H

#include <set>
#include "ddd/DDD.h"
#include "ddd/Hom.h"
#include "ddd/util/hash_support.hh"

class _DED;
class GDDD;
class GHom;

/******************************************************************************/
class DED{
  //private:
  //  friend struct std::equal_to<DED>;
  //  _DED *concret;
public:
  // GDDD eval();
  // DED(_DED *c):concret(c){};
  // ctor necessary for google sparse hash
  // DED():concret(NULL){};
  // bool operator==(const DED&) const; 
  /// For storage in a hash table
  // size_t hash () const ;


  static GDDD add(const d3::set<GDDD>::type &);
  //static GDDD hom(const GHom &,const GDDD&);

  /* Memory Manager */
  static  unsigned int statistics();
  static void pstats(bool reinit=true);
  static size_t peak();
  static void garbage(); 
};


#ifdef EVDDD
GHom pushEVDDD(int v);
#endif

#endif






