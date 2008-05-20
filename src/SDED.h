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
#ifndef SDED_H
#define SDED_H

#include <set>

#include "DataSet.h"
#include "util/hash_support.hh"

class _SDED;
class GSDD;
class GShom;
        
/******************************************************************************/
class SDED{
private:
  _SDED *concret;

public:
	
  GSDD eval();
  SDED(_SDED *c):concret(c){};
  bool operator==(const SDED&) const; 
  
  static GSDD add(const std::set<GSDD> &);
  static GSDD Shom(const GShom &,const GSDD&);

  /* Memory Manager */
  static  unsigned int statistics();
  static void pstats(bool reinit=true);
  static size_t peak();
#ifdef OTF_GARBAGE
  static void recentGarbage(bool force=false);
#endif // OTF_GARBAGE
  static void garbage(); 
  /// For storage in a hash table
  size_t hash () const ;
};


// Library internal: square_union (cf FORTE'05)
void
square_union (std::map<GSDD,DataSet *> &res,const GSDD & s, DataSet* d);

/******************************************************************************/
class _SDED{

public:
  /* Destructor */
  virtual ~_SDED(){};

#ifdef OTF_GARBAGE
  // to enact dynamic garbage collection mechanism
  // Returns true if all arguments of the operation of type GSDD have property that (isSon()==true)
  // This property triggers long term storage
  virtual bool shouldCache() {return false ;}
#endif

  /* Compare */
  virtual size_t hash() const =0;
  virtual bool operator==(const _SDED &) const=0;
  virtual _SDED * clone () const =0;
  /* Transform */
	virtual GSDD eval() const  = 0; 

};

#ifdef EVDDD
GShom pushEVSDD(int v);
#endif

#endif

