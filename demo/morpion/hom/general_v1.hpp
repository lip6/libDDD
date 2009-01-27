// author : S. Hong,Y.Thierry-Mieg
// date : Jan 2009
#pragma once

#include <iostream>
#include <boost/multi_array.hpp>

#include "DDD.h"
#include "Hom.h"
#include "hom/const.hpp"


Hom
    createWinnerConfiguration( int player);

Hom
    createPlayerHit( int player);

DDD
    createInitial2 ();

