//
// C++ Interface: hom_check_imp
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

#include <boost/shared_ptr.hpp>

#include "DDD.h"
#include "Hom.h"
#include "hom/const.hpp"
#include "validate_base.hpp"

typedef boost::shared_ptr<const validate_base> ref_validate_base_type;
typedef std::vector<ref_validate_base_type> ref_tests;


Hom
    checkImpossible (const ref_tests& tts);

Hom
    checkImpossible (int fi,int nti,const ref_tests& tts);

void
    printState (const array_type& cc);



