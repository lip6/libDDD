// author : S. Hong,Y.Thierry-Mieg
// date : Jan 2009
#ifndef __MORPION__HOM__
#define __MORPION__HOM__

#include "DDD.h"
#include "Hom.h"
#include "SDD.h"
#include "SHom.h"
#include <iostream>






extern const int EMPTY;
extern const int NBCASE;
extern const int LINE;
extern const int COLUMN;

// play a move
GHom take_cell (int cell, int player) ;

GHom SelectWin (int c1, int c2, int c3, int nb, int player);

GHom checkWinner (int t[3][3]);
#endif 
