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
#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H
#include "DDD.h"
#include "DED.h"
#include "Hom.h"
#include "SDD.h"
#include "SDED.h"
#include "SHom.h"
#include "MLHom.h"
#include "IntDataSet.h"


#include "process.hpp"


class GCHook {
 public:
  virtual ~GCHook() {}
  virtual void preGarbageCollect() =0;
  virtual void postGarbageCollect() = 0;

};

/// This class defines a few utility functions common to DDD.
/// Note that all functions are declared static, so this is more of a namespace than a class.
/// One important function is garbage(), only this MemoryManager::garbage() should be caled :
/// avoid GHom::garbage(), DED::garbage(), GDDD::garbage() (and SDD versions of the same,
/// they should not be called directly.
class MemoryManager{
  typedef std::vector<GCHook*> hooks_t;
  typedef hooks_t::iterator hooks_it;
  // actually defined in DDD.cpp, bottom of file.
  static hooks_t hooks_;
public:
  /* Mesure*/
  /// Returns the size of the unicity table for DDD.
  static unsigned int nbDDD(){return GDDD::statistics();};
  /// Returns the size of the cache unicity table for DDD. 
  static unsigned int nbDED(){return DED::statistics();};
  /// Returns the size of the unicity table for DDD Homomorphisms.
  static unsigned int nbHom(){return GHom::statistics();};
  /// Returns the size of the unicity table for SDD.
  static unsigned int nbSDD(){return GSDD::statistics();};
  /// Returns the size of the cache unicity table for SDD. 
  static unsigned int nbSDED(){return SDED::statistics();};
  /// Returns the size of the unicity table for SDD Homomorphisms.
  static unsigned int nbShom(){return GShom::statistics();};
  /* Garbage Collector */
  /// Convenience function to mark a node as non collectible.
  /// \todo : track usage and check whether this is useful, SDD version undefined.  
  static void mark(const GDDD &g){g.mark();};
  /// Convenience function to mark a Hom as non collectible.
  /// \todo : track usage and check whether this is useful, SDD version undefined.  
  static void mark(const GHom &h){h.mark();};

  /// tester for memory management routine triggering in a top level fixpoint
    static bool should_garbage() {
      // trigger at rougly 5 million objects =1 Gig RAM
      //return nbDED() + nbSDED() + nbShom() + nbSDD() > 3000000;
      size_t mem = process::getResidentMemory();
      if (mem == 0)
	return true;
      // add ten percent growth
      if (mem > last_mem + last_mem / 10 ) {
/* 	std::cerr << "GC triggered at mem=" << mem << std::endl; */
	last_mem = mem;
	return true;
      } else {
/* 	std::cerr << "GC not triggered mem=" << mem << std::endl; */
	return false;
      }
    }

  /// Garbage collection function. 
  /// Call this to reclaim intermediate nodes, unused operations and related cache.
  /// Note that this function is quite costly, and it totally destroys the cache
  static void garbage(){
    for (hooks_it it = hooks_.begin(); it != hooks_.end() ; ++it) {
      (*it)->preGarbageCollect();
    }

    MLHom::garbage();
    // FIXME : if you dont use SDD suppress the following
    SDED::garbage();
    GShom::garbage();
    GSDD::garbage();
    // clear the IntDataSet
    IntDataSet::garbage();
    // END FIXME 
    DED::garbage();
    GHom::garbage();
    GDDD::garbage();

    for (hooks_it it = hooks_.begin(); it != hooks_.end() ; ++it) {
      (*it)->postGarbageCollect();
    }
  };

  /// Prints some statistics about use of unicity tables, also reinitializes peak sizes.
  static void pstats(bool reinit=true){
    //cout << " Memory Usage " << MemUsage() << " %" << endl;
    
    // FIXME : if you dont use SDD suppress the following
    SDED::pstats(reinit);
    GShom::pstats(reinit);
    GSDD::pstats(reinit);    
    // END FIXME 

    DED::pstats(reinit);
    GHom::pstats(reinit);
    GDDD::pstats(reinit);    
  }

  static void setGCThreshold (size_t nbKbyte) {
    last_mem = nbKbyte;
  }

  static size_t getPeakMemory () {
    should_garbage();
    return last_mem;
  }

  static void addHook (GCHook * hook) {
    hooks_.push_back(hook);
  }

 private :
  // actually defined in DDD.cpp, bottom of file.
  static size_t last_mem;


};
#endif
