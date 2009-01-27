// author : S. Hong,Y.Thierry-Mieg
// date : Jan 2009
#include "hom/general_v2.hpp"

#include <iostream>
#include <boost/functional/hash.hpp>
#include <vector>

#include "hom/notew.hpp"
#include "hom/nowinner.hpp"
#include "hom/play.hpp"
#include "hom/winner.hpp"



// The player designated plays in a free cll, if any are available.
Hom
  PlayAnyFreeCell ()
{
  std::set<GHom> nextAAset;

  for ( size_t i = 0; i < NBCASE; ++i )
  {
    nextAAset.insert ( Play ( i ) );
  }

  return GHom::add ( nextAAset );
}

// check if a player has won
// Select any path where the designated player has won : i.e has three aligned cross or circle.
Hom
    CheckIsWinner ( game_status_type player )
{
  std::set<GHom> winAAset;

  // lines
  winAAset.insert ( CheckCellWinner ( player, 0 ) & CheckCellWinner ( player, 1 ) & CheckCellWinner ( player, 2 ) ) ;
  winAAset.insert ( CheckCellWinner ( player, 3 ) & CheckCellWinner ( player, 4 ) & CheckCellWinner ( player, 5 ) ) ;
  winAAset.insert ( CheckCellWinner ( player, 6 ) & CheckCellWinner ( player, 7 ) & CheckCellWinner ( player, 8 ) ) ;

  // cols
  winAAset.insert ( CheckCellWinner ( player, 0 ) & CheckCellWinner ( player, 3 ) & CheckCellWinner ( player, 6 ) ) ;
  winAAset.insert ( CheckCellWinner ( player, 1 ) & CheckCellWinner ( player, 4 ) & CheckCellWinner ( player, 7 ) ) ;
  winAAset.insert ( CheckCellWinner ( player, 2 ) & CheckCellWinner ( player, 5 ) & CheckCellWinner ( player, 8 ) ) ;

  // diagonals
  winAAset.insert ( CheckCellWinner ( player, 0 ) & CheckCellWinner ( player, 4 ) & CheckCellWinner ( player, 8 ) ) ;
  winAAset.insert ( CheckCellWinner ( player, 2 ) & CheckCellWinner ( player, 4 ) & CheckCellWinner ( player, 6 ) ) ;

  return GHom::add ( winAAset );
}


// Strategy marker
const int NOWINNER_STRAT = 0;

Hom CheckNoWinner ()
{
  if ( NOWINNER_STRAT == 0 )
  {
    // NEW !! use a negation : no winner = not ( A wins or B wins )
    Hom res = ! ( CheckIsWinner ( PA ) + CheckIsWinner ( PB ) );
    return res;
  }
  else
  {
    // copy paste from main
    Hom noWinner;
    typedef std::vector<game_status_type> players_array_type;
    players_array_type players;
    players.push_back(EMPTY);
    players.push_back(PA);
    players.push_back(PB);

    for(players_array_type::const_iterator i = players.begin() ; i != players.end() ; ++i)
    {
      if ( *i == PA )
      {
        noWinner = ( CheckCellNoWinner ( *i, 0 ) + CheckCellNoWinner ( *i, 1 ) + CheckCellNoWinner ( *i, 2 ) ) ;
      }
      else
      {
        noWinner = noWinner & ( CheckCellNoWinner ( *i, 0 ) + CheckCellNoWinner ( *i, 1 ) + CheckCellNoWinner ( *i, 2 ) ) ;
      }

      noWinner = noWinner & ( CheckCellNoWinner ( *i, 3 ) + CheckCellNoWinner ( *i, 4 ) + CheckCellNoWinner ( *i, 5 ) ) ;

      noWinner = noWinner & ( CheckCellNoWinner ( *i, 6 ) + CheckCellNoWinner ( *i, 7 ) + CheckCellNoWinner ( *i, 8 ) ) ;

      noWinner = noWinner & ( CheckCellNoWinner ( *i, 0 ) + CheckCellNoWinner ( *i, 3 ) + CheckCellNoWinner ( *i, 6 ) ) ;
      noWinner = noWinner & ( CheckCellNoWinner ( *i, 1 ) + CheckCellNoWinner ( *i, 4 ) + CheckCellNoWinner ( *i, 7 ) ) ;
      noWinner = noWinner & ( CheckCellNoWinner ( *i, 2 ) + CheckCellNoWinner ( *i, 5 ) + CheckCellNoWinner ( *i, 8 ) ) ;

      noWinner = noWinner & ( CheckCellNoWinner ( *i, 0 ) + CheckCellNoWinner ( *i, 4 ) + CheckCellNoWinner ( *i, 8 ) ) ;
      noWinner = noWinner & ( CheckCellNoWinner ( *i, 2 ) + CheckCellNoWinner ( *i, 4 ) + CheckCellNoWinner ( *i, 6 ) ) ;
    }

    return noWinner;
  }
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
  
  M0 = GDDD(NBCASE, EMPTY) ^  M0 ;
  DDD::varName(NBCASE,"State");
  return M0;
}





















