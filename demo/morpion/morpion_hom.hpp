// author : S. Hong,Y.Thierry-Mieg
// date : Jan 2009
#ifndef __MORPION__HOM__
#define __MORPION__HOM__

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

enum content_type{ Vide = -1, JoueurA = 0, JoueurB = 1 } ;

typedef boost::multi_array<content_type, 2> array_type;

GHom TakeCellWithCheckWinner ( int c1, int player);

GHom NoteWinner (int c1, int c2, int c3, int nb, int player);

GHom Full ();

GHom checkImpossible (const array_type& t);
#endif 
