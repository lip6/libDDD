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
#include "ddd/util/dotExporter.h"
#include "ddd/statistic.hpp"

/* A Modifier avec les standard de Boost */
void usage() {
  cout << "Morpion Game; package " << PACKAGE_STRING <<endl;
  cout << "This tool performs state-space analysis of the tic-tac-toe a.k.a. morpion game.\n"
      << " The reachability set is computed using SDD/DDD, the Hierarchical Set Decision Diagram library, \n"
      << " Please see README file enclosed \n"
      << "in the distribution for more details. \n \nOptions :" << endl;
  cout <<  "Problems ? Comments ? contact " << PACKAGE_BUGREPORT <<endl;
  
  cout << "\nCommand :" << endl;
  cout << "morpionv2 <NB_LINE>" << endl;
  cout << " NB_LINE : The number of Cell Game into one line (Morpion game is a square), by default it 3 for have a game 3x3" << endl;
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
int main (int argc, char ** argv) {

  // Print the command usage
  usage();
  
  // Initialization of the game cell configuration (the number of cell into one line)
  LINE = 3;
  if (argc == 2)
    LINE = atoi(argv[1]);
  NBCELL = LINE * LINE;
  STATE_SYSTEM_CELL = NBCELL;
  COLUMN = LINE;
  
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
  cout << "\nPrint the initial DDD Graph : " << endl;
  cout << initial << endl;
  cout << "\nMake the fix point for Grid Tic Tac Toe ["<< LINE << "," << COLUMN <<"] :\nIt can take a long time ... please wait ..." << std::endl ;
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
  std::cout << "Exporting the DDD Graph into morpion.dot file" << std::endl;
  exportDot(SDD(0,reachable),"morpion");
  Statistic S2 = Statistic(reachable, "morpion" , CSV); // can also use LaTeX instead of CSV
  std::cout << "Statistic Generation of Morpion Space State" << std::endl;
  S2.print_table(std::cout);


	
	

  return EXIT_SUCCESS;
}
