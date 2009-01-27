// author : S. Hong,Y.Thierry-Mieg // date : Jan 2009
#include <sstream>
#include <cstdlib>
#include "hom/morpion_hom_vyann.hpp"

// SDD utilities to output stats and dot graphs
#include "util/dotExporter.h"
#include "statistic.hpp"

// option set to true limits verbosity
//static bool beQuiet = false;
// path prefix to output dot files
static string pathdotff = "final";
//static bool dodotexport=false;
// true if user asked for dot export
// static bool dodotexport=false;
// used as first column label of statistics
static std::string modelName = "";

// The number of cells in the game, i.e. 9


void usage() {
  cerr << "Morpion V2; package " << PACKAGE_STRING <<endl;
  cerr << "This tool performs state-space analysis of the tic-tac-toe a.k.a. morpion game.\n"
      << " The reachability set is computed using SDD/DDD, the Hierarchical Set Decision Diagram library, \n"
      << " Please see README file enclosed \n"
      << "in the distribution for more details. \n \nOptions :" << endl;
  cerr<<  "    -d path : specifies the path prefix to construct dot state-space graph" <<endl;
  cerr<<  "    --texhead : print tex header"  << endl;
  cerr<<  "    --texline : print tex line " <<endl ;
  cerr<<  "    --textail : print tex tail"  << endl;
  cerr<<  "    --tex : print full tex description" <<endl;
  cerr<<  "    --quiet : limit output verbosity useful in conjunction with tex output --texline for batch performance runs" <<endl;
  cerr<<  "    --help,-h : display this (very helpful) helping help text"<<endl;
  cerr<<  "Problems ? Comments ? contact " << PACKAGE_BUGREPORT <<endl;
}

void bugreport () {
  cerr << "Timed Petri Net SDD/DDD Analyzer; package " << PACKAGE_STRING <<endl;
  cerr << "If you think your model is valid (i.e. it's a bug from us), please send your model files, specify the version you are running on, and we'll try to fix it." <<endl;
  cerr << "Bugreport contact : " << PACKAGE_BUGREPORT <<endl;
  cerr << "Sorry." << endl;
  exit(1);
}

/**
* Create the default configuration struture of DDD
*/
DDD createInitial () {
  
  GDDD M0 = GDDD::one;

  for (size_t i = 0; i < NBCASE; ++i) {
    M0 = GDDD(i, EMPTY) ^  M0 ;
    std::stringstream cas;
    cas << "cell_" << i;
    DDD::varName(i,cas.str());
  }
  // initially P1TURN
  M0 = GDDD(NBCASE,P1TURN) ^  M0 ;
  DDD::varName(NBCASE,"State");
  return M0;
}


int main (int /*argc*/, char ** /*argv*/) {
  
  // Creation de l'état initial
  DDD initial = createInitial ();
  
  // Creation d'un ensemble d'homomorphisme pour le joueur 1
  Hom nextAA = PlayAnyFreeCell(PA);
  Hom nextBB = PlayAnyFreeCell(PB);
  
  // Initialisation des combinaisons gagnantes
  GHom winnerA = CheckIsWinner (PA);
  GHom winnerB = CheckIsWinner (PB);
 
  // Initialisation des combinaisons non gagnantes
  GHom noWinner = CheckNoWinner();

  std::cout << "Make the fix point : \n\n\n" << std::endl ;
  /* ALGO :
  * 1) First we play BB, only if there is no winner in the current configuration
  * 2) Next we try to play AA (only if there is no winner in the current configuration), there is two possibility in union
  * 2.1) BB can play so it play (no winner and no full game)
  * 2.2) BB can't play because the game is full (it is possible), so nextAA cut this configuration but Full keep it
  * 3) Next we check if there is a winner for player A or B, if yes we note it at the end of the configuration game
  */
  /*
  GHom fullT2 = checkImpossible(tab,0,9) & fixpoint (
                          ( NoteWinner(0) & (winnerA & ( (Full(9) + nextAA)  & nextBB ) ) )
                           +
                          ( NoteWinner(1) & (winnerB & ( (Full(9) + nextAA)  & nextBB ) ) )
                           +
                          ( noWinner & ( (Full(9) + nextAA)  & nextBB ) )
                              
			  + GHom::id ) ;
  */
  /*  
  GHom fullT2 = checkImpossible(tab,0,9) & fixpoint( ( ( (Full(9) + nextAA) & nextBB) ) + GHom::id);
  */
  GHom P1play =  // update game status
		 (  
		  // P2 turn
		  (updateGameStatus(P2TURN) & (!winnerA))
		  +
		  // P1 win
		  (updateGameStatus(P1WIN) & winnerA )
		  )
		 // P1 plays
		 & nextAA 
		 // P1 turn
		 & testGameStatus(P1TURN)
    ;
  GHom P2play =
    // update game status
    (  
     // P1 turn
     (updateGameStatus(P1TURN) & !winnerB)
     +
     // P2 win
     (updateGameStatus(P2WIN) & winnerB )
     )
    // P2 plays
    & nextBB 
    // P2 turn
    & testGameStatus(P2TURN)
    ;
  GHom fullT2 =  /*checkImpossible(tab,0,9) &*/ fixpoint (
		// Move by P1 :
		P1play
		+
		// Move by P2 :
		P2play
		+
		// accumulate
		GHom::id );  
  std::cout << fullT2  << std::endl;
  std::cout << initial << std::endl;
 
  std::cout << P1play (initial) << std::endl;

  std::cout << "here" << std::endl;
  DDD reachable = fullT2 (initial);
  exportDot(GSDD(0,reachable),"reach2");
  Statistic S2 = Statistic(reachable, "reach2" , CSV); // can also use LATEX instead of CSV
  S2.print_table(std::cout);
  
  bool interactive = true;
  if (interactive) {
    reachable = initial;
    for (int i=0 ; i<10 ; ++i) {
      std::cout << "Move " << i << std::endl;
      exportDot(GSDD(0,reachable),"reach");
      std::string s;
      std::cin >> s;    
      reachable = (P1play + P2play) (reachable);
    }
  }

  
  
  return EXIT_SUCCESS;
}
