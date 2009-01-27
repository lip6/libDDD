//
// C++ Implementation: check_impossible
//
// Description: 
//
//
// Author: Yann Thierry-Mieg <LIP6, Yann.Thierry-Mieg@lip6fr > (2003-), Jean-Michel Couvreur <LaBRi > (2001), and Denis Poitrenaud (2001) <LIP6>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "hom/check_impossible.hpp"


  check_lines::check_lines()
     : validate_base("Impossible configuration detected on line : there are two winners.")
  {
  }

    bool
      check_lines::check(const array_type& cc)
      const
  {
    size_t nbline = 0;
    // Check the impossible line
    for(size_t i = 0; i< LINE ; ++i)
    {
      nbline += check_line(cc,i) ? 1 : 0;
    }
    
    return nbline > 1;
  }