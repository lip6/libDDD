/* -*- C++ -*- */
#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H
#include "DDD.h"
#include "DED.h"
#include "Hom.h"

class MemoryManager{
public:
  /* Mesure*/
  static unsigned int nbDDD(){return GDDD::statistics();};
  static unsigned int nbDED(){return DED::statistics();};
  static unsigned int nbHom(){return GHom::statistics();};
  /* Garbage Collector */
  static void mark(const GDDD &g){g.mark();};
  static void mark(const GHom &h){h.mark();};
  static void garbage(){
    DED::garbage();
    GHom::garbage();
    GDDD::garbage();
  };

  static void pstats(bool reinit=true){
    //cout << " Memory Usage " << MemUsage() << " %" << endl;
    DED::pstats(reinit);
    GHom::pstats(reinit);
    GDDD::pstats(reinit);    
  }
  
};
#endif
