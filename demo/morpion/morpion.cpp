// author : S. Hong,Y.Thierry-Mieg
// date : Jan 2009
#include <sstream>
#include "morpion_hom.hpp"
#include "morpion_const.hpp"
#include "hom_check_imp.h"

// SDD utilities to output stats and dot graphs
#include "util/dotExporter.h"
#include "statistic.hpp"




void usage() {
  cerr << "Morpion; package " << PACKAGE_STRING <<endl;
  cerr<<  "Problems ? Comments ? contact " << PACKAGE_BUGREPORT <<endl;
}

void bugreport () {
  cerr << "Bugreport contact : " << PACKAGE_BUGREPORT <<endl;
  cerr << "Sorry." << endl;
  exit(1);
}




int main (int /*argc*/, char ** /*argv*/) {
  
  // Creation de l'état initial
  DDD initial2 = createInitial2 ();
  
  // Creation d'un ensemble d'homomorphisme pour le joueur 1
  Hom nextAA = createPlayerHit(0);
  Hom nextBB = createPlayerHit(1);
  
  // Insertion des homomorphismes pour couper les chemins que l'on ne veut pas
  array_type tab(boost::extents[LINE][COLUMN]);
  for(int i = 0; i< LINE ; ++i)
  {
    for(int j=0; j<COLUMN ; ++j)
    {
      tab[i][j]=EMPTY;
    }
  }
  
  // Insertion des homomorphismes pour couper les chemins que l'on ne veut pas
  Hom noteWinnerB = createWinnerConfiguration(1);
  Hom noteWinnerA = createWinnerConfiguration(0);
  
  std::cout << "Make the fix point : \n\n\n" << std::endl ;
  /* ALGO :
   * 1) First we play BB, only if there is no winner in the current configuration
   * 2) Next we try to play AA (only if there is no winner in the current configuration), there is two possibility in union
   * 2.1) BB can play so it play (no winner and no full game)
   * 2.2) BB can't play because the game is full (it is possible), so nextAA cut this configuration but Full keep it
   * 3) Next we check if there is a winner for player A or B, if yes we note it at the end of the configuration game
   */
  GHom fullT2 = checkImpossible(tab) & fixpoint ( ( noteWinnerB & ( noteWinnerA & ( (Full() + nextAA)  & nextBB ) ) ) + GHom::id ) ;

  DDD reachable2 = fullT2 (initial2);
  exportDot(GSDD(0,reachable2),"reach2");
  Statistic S2 = Statistic(reachable2, "reach2" , CSV); // can also use LATEX instead of CSV
  S2.print_table(std::cout);

  return 0;
  
 
}