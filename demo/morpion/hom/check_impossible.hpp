//
// C++ Interface: check_impossible
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

#include "hom/validate_base.hpp"

class check_lines
  : public validate_base
{
  public:

    check_lines();

    virtual
        bool
        check(const array_type& cc)
        const;

  
};

