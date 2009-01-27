// author : S. Hong,Y.Thierry-Mieg
// date : Jan 2009
#pragma once

#include <boost/multi_array.hpp>

// Const variable
static const size_t NBCASE = 9;
static const size_t LINE = 3;
static const size_t COLUMN = 3;

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
