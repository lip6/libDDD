/* -*- C++ -*- */
#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H
#include "DDD.h"
#include "DED.h"
#include "Hom.h"
#include "SDD.h"
#include "SDED.h"
#include "SHom.h"

class MemoryManager{
public:
  /* Mesure*/
  static unsigned int nbDDD(){return GDDD::statistics();};
  static unsigned int nbDED(){return DED::statistics();};
  static unsigned int nbHom(){return GHom::statistics();};
  static unsigned int nbSDD(){return GSDD::statistics();};
  static unsigned int nbSDED(){return SDED::statistics();};
  static unsigned int nbShom(){return GShom::statistics();};
  /* Garbage Collector */
  static void mark(const GDDD &g){g.mark();};
  static void mark(const GHom &h){h.mark();};
  static void garbage(){
    // FIXME : if you dont use SDD suppress the following
    SDED::garbage();
    GShom::garbage();
    GSDD::garbage();
    // END FIXME 
    DED::garbage();
    GHom::garbage();
    GDDD::garbage();
  };

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
  
};
#endif
