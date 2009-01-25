// author : S. Hong,Y.Thierry-Mieg
// date : Jan 2009
#ifndef __MORPION__HOM__V2__
#define __MORPION__HOM__V2__

#include <iostream>
#include "boost/multi_array.hpp"

#include "DDD.h"
#include "Hom.h"
#include "SDD.h"
#include "SHom.h"

extern const int EMPTY;
extern const int NBCASE;
extern const int LINE;
extern const int COLUMN;

GHom
    NoteWinner (int player);

GHom 
    CheckCellNoWinner (int player,int cell);

GHom 
    Play ( int c1, int player);

// The player designated plays in a free cell, if any are available. 
GHom 
    PlayAnyFreeCell (int player);

// Select any path where the designated player has won : i.e has three aligned cross or circle.
GHom 
     CheckIsWinner (int player);

GHom 
    CheckCellWinner (int player , int cell);
    
#endif 
