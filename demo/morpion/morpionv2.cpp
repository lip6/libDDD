// author : S. Hong,Y.Thierry-Mieg // date : Jan 2009
#include <sstream>
#include <cstdlib>
//#include <boost/tr1/array.hpp>
#include <map>
#include <vector>
#include <utility>
#include <boost/shared_ptr.hpp>


#include "hom/general_v2.hpp"
#include "hom/const.hpp"
#include "hom/notew.hpp"



// SDD utilities to output stats and dot graphs
#include "util/dotExporter.h"
#include "statistic.hpp"



/* A Modifier avec les standard de Boost */
void usage() {
  cerr << "Morpion Game; package " << PACKAGE_STRING <<endl;
  cerr << "This tool performs state-space analysis of the tic-tac-toe a.k.a. morpion game.\n"
      << " The reachability set is computed using SDD/DDD, the Hierarchical Set Decision Diagram library, \n"
      << " Please see README file enclosed \n"
      << "in the distribution for more details. \n \nOptions :" << endl;
  cerr<<  "Problems ? Comments ? contact " << PACKAGE_BUGREPORT <<endl;
}

void bugreport () {
  cerr << "Bugreport contact : silien.hong@lip6.fr " << PACKAGE_BUGREPORT <<endl;
  cerr << "Sorry." << endl;
  exit(1);
}

/**
 *
 * @param
 * @param
 * @return
 */
int main (int /*argc*/, char ** /*argv*/) {

  // Create the initial state
  DDD initial = createInitial ();

  // Initialisation of the homomorphism for one play of player A and B
  Hom nextAA = PlayAnyFreeCell();

  // Initialisation of the homomorphism for check one configuration of player is winner, for player A or B
  std::map<game_status_type,Hom> winners;
  winners[PA] = CheckIsWinner (PA);
  winners[PB] = CheckIsWinner (PB);

  // Initialisation of no winner configuration
  Hom noWinner = CheckNoWinner();

  std::cout << "Make the fix point for Grid Tic Tac Toe ["<< LINE << "," << COLUMN <<"] : \n\n\n" << std::endl ;
  /* ALGO :
  * For each player :
  *   1) We can play only if there is no winner configuration
  *   2) If there is one winner A or B, we note into the globally state of the system that the current configuration is blocked 
  */
  Hom fullT2 = 
      fixpoint
      ( ( ( ( (NoteWinner(PA) & winners[PA])
            + (NoteWinner(PB) & winners[PB])
            + noWinner
            )
          & nextAA))
      + Hom::id
      );


  DDD reachable = fullT2 (initial);
  exportDot(SDD(0,reachable),"reach2");
  Statistic S2 = Statistic(reachable, "reach2" , CSV); // can also use LaTeX instead of CSV
  std::cout << "Statistic Generation of Morpion Space State" << std::endl;
  S2.print_table(std::cout);


	
	

  return EXIT_SUCCESS;
}
