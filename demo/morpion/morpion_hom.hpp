// author : S. Hong,Y.Thierry-Mieg
// date : Jan 2009
#ifndef __MORPION__HOM__
#define __MORPION__HOM__

#include "DDD.h"
#include "Hom.h"
#include "SDD.h"
#include "SHom.h"


extern const int EMPTY;
// play a move
GHom take_cell (int cell, int player) ;

GHom SelectWin (int c1, int c2, int c3, int nb, int player);


#endif 
