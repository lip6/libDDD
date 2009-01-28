// author : S. Hong,Y.Thierry-Mieg
// date : Jan 2009
#pragma once

#include <boost/multi_array.hpp>
#include <cmath>

// Const variable
static const size_t NBCELL = 16;
static const size_t STATE_SYSTEM_CELL = NBCELL;
static const size_t LINE = sqrt(NBCELL);
static const size_t COLUMN = sqrt(NBCELL);

// players
/**
 *
 */
enum game_status_type
{ EMPTY = -1 /// Ligne Vide
, PA    =  0 /// Joueur A
, PB    =  1 /// Joueur B
};

// Type definition
typedef boost::multi_array<int, 2> array_type;
