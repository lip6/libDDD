//
// C++ Interface: validate_base
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

#include <iostream>
#include "hom/const.hpp"


class validate_base
{
private:
  std::string message;

protected:

  validate_base(const std::string& mess);
  
  validate_base(const array_type& cc);

  validate_base(const validate_base& rhs);
  
  ~validate_base();

  validate_base&
      operator=(const validate_base& rhs);

  bool
      check_line(const array_type& cc,int line)
      const;

  bool
      check_column(const array_type& cc,int column)
      const;

  bool
      check_cross(const array_type& cc)
      const;

  virtual
      bool
      check(const array_type& cc)
      const=0;
       

  public:

  void
      operator()(const array_type& cc)
      const;
};
