// author : S. Hong,Y.Thierry-Mieg
// date : Jan 2009
#pragma once

#include "ddd/DDD.h"
#include "ddd/Hom.h"
#include "hom/const.hpp"


// The player designated plays in a free cell, if any are available.
Hom
    PlayAnyFreeCell ();

// The player designated plays in a free cell, if any are available.
Hom
    PlayAnyFreeCell (game_status_type player);

// Select any path where the designated player has won : i.e has three aligned cross or circle.
Hom
    CheckIsWinner (game_status_type player);

// Selects paths such that there is no winner
Hom
     CheckNoWinner ();

DDD
    createInitial ();
