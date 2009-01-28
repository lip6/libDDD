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
#include "hom/check_imp.hpp"
#include "hom/notew.hpp"
#include "hom/check_impossible.hpp"
#include "hom/validate_base.hpp"



// SDD utilities to output stats and dot graphs
#include "util/dotExporter.h"
#include "statistic.hpp"



/* A Modifier avec les standard de Boost */
void usage() {
  cerr << "Morpion V2; package " << PACKAGE_STRING <<endl;
  cerr << "This tool performs state-space analysis of the tic-tac-toe a.k.a. morpion game.\n"
      << " The reachability set is computed using SDD/DDD, the Hierarchical Set Decision Diagram library, \n"
      << " Please see README file enclosed \n"
      << "in the distribution for more details. \n \nOptions :" << endl;
  cerr<<  "Problems ? Comments ? contact " << PACKAGE_BUGREPORT <<endl;
}

void bugreport () {
  cerr << "Bugreport contact : " << PACKAGE_BUGREPORT <<endl;
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

  // Initialisation of the possible hit of players
  Hom nextAA = PlayAnyFreeCell();

  // Initialisation of winner configuration
  //std::tr1::array<Hom, 2> winners = {{CheckIsWinner (PA),CheckIsWinner (PB)}};
  std::map<game_status_type,Hom> winners;
  winners[PA] = CheckIsWinner (PA);
  //winners.insert( std::pair<game_status_type,Hom>(PB,CheckIsWinner (PB)));
  //winners.insert(std::make_pair(PB,CheckIsWinner (PB)));
  winners[PB] = CheckIsWinner (PB);

  // Initialisation of no winner configuration
  Hom noWinner = CheckNoWinner();

  std::clog << "Make the fix point for Grid Tic Tac Toe ["<< LINE << "," << COLUMN <<"] : \n\n\n" << std::endl ;
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

  typedef boost::shared_ptr<const validate_base> ref_validate_base_type;
  std::vector<ref_validate_base_type> tests;
  tests.push_back(ref_validate_base_type(new check_lines()));



  Hom fullT2 = /*checkImpossible(0,STATE_SYSTEM_CELL,tests) & */
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
  S2.print_table(std::cout);


  return EXIT_SUCCESS;
}
