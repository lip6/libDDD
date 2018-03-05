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

#include <map>
#include "ddd/util/set.hh"

#include "ddd/DataSet.h"
#include "ddd/util/hash_support.hh"

class GSDD;
        
/******************************************************************************/
namespace SDED{
  GSDD add(const d3::set<GSDD>::type &);

  /* Memory Manager */
  unsigned int statistics();
  void pstats(bool reinit=true);
  size_t peak();
  void garbage(); 
};


// Library internal: square_union (cf FORTE'05)
void
square_union (std::map<GSDD,DataSet *> &res,const GSDD & s, DataSet* d);

#ifdef EVDDD
GShom pushEVSDD(int v);
#endif

#endif

