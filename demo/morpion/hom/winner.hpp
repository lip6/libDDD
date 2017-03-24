//
// C++ Interface: hom_winner
//
// Description: 
//
//
// Author: Yann Thierry-Mieg <LIP6, Yann.Thierry-Mieg@lip6fr > (2003-), Jean-Michel Couvreur <LaBRi > (2001), and Denis Poitrenaud (2001) <LIP6>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#pragma once

#include "ddd/DDD.h"
#include "ddd/Hom.h"
#include "hom/const.hpp"

/**
 * Factory of _TakeCellWithCheckWinner Instance
 */
Hom
    CheckCellWinner ( game_status_type player , int cell );
