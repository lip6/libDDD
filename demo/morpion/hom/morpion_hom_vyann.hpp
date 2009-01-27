// author : S. Hong,Y.Thierry-Mieg
// date : Jan 2009

#pragma once


#include "DDD.h"
#include "Hom.h"
#include "hom/const.hpp"


enum GAMESTATUS
{ P1TURN
, P2TURN
, P1WIN
, P2WIN
};

// update the variable representing game status to "gs"
Hom
updateGameStatus (GAMESTATUS gs);
// check the variable representing game status is set to "gs"
Hom
    testGameStatus (GAMESTATUS gs);
// The player designated plays in a free cll, if any are available.
Hom
  PlayAnyFreeCell (int player);
// Select any path where the designated player has won : i.e has three aligned cross or circle.
Hom
  CheckIsWinner (int player);

Hom
CheckNoWinner ();

/**
 * Factory of _TakeCellWithCheckWinner Instance
 */
Hom
    Play ( int c1, int player);

/**
 * Factory of _TakeCellWithCheckWinner Instance
 */
Hom
    NoteWinner (int player);

/**
 * Factory of _TakeCellWithCheckWinner Instance
 */
Hom
    CheckCellWinner (int player , int cell);

    
/**
 * Factory of _TakeCellWithCheckWinner Instance
 */
Hom
    CheckCellNoWinner (int player,int cell);
