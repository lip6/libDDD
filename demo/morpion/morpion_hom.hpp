#ifndef __MORPION__HOM__
#define __MORPION__HOM__

#include "DDD.h"
#include "Hom.h"
#include "SDD.h"
#include "SHom.h"


extern const int EMPTY;
// play a move
GHom take_cell (int cell, int player) ;
GHom _SelectWin (int c1, int c2, int c3, int player);

#endif 
