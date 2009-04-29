//
// C++ Implementation: validate_base
//
// Description: 
//
//
// Author: Yann Thierry-Mieg <LIP6, Yann.Thierry-Mieg@lip6fr > (2003-), Jean-Michel Couvreur <LaBRi > (2001), and Denis Poitrenaud (2001) <LIP6>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "hom/validate_base.hpp"

#include "hom/check_imp.hpp"

validate_base::validate_base(const std::string& mess)
  : message(mess)
{
}

validate_base::~validate_base()
{
}

validate_base::validate_base(const validate_base& rhs)
  : message(rhs.message)
{
}


validate_base&
    validate_base::operator=(const validate_base& rhs)
{
  message=rhs.message;
  return *this;
}



void
    validate_base::operator()(const array_type& cc)
    const
{
  if(this->check(cc))
  {
    std::clog << message << std::endl;
    printState(cc);
  }
}



 bool
     validate_base::check_line(const array_type& cc,int line)
     const
{
  bool res = true;
  int ref = cc[line][0];
  
  for(size_t j=1;j<COLUMN;++j)
  {
    res = res and (cc[line][j] != EMPTY)
        and (cc[line][j] == ref);
  }

  return res;
}

 bool
     validate_base::check_column(const array_type& cc,int column)
     const
{
  bool res = true;
  int ref = cc[0][column];
  
  for(size_t j=1;j<LINE;++j)
  {
    res = res and (cc[j][column] != EMPTY)
        and (cc[j][column] == ref);
  }

  return res;
}

 bool
     validate_base::check_cross(const array_type& cc)
     const
{
  bool res_1 = true;
  bool res_2 = true;
  int ref = cc[0][0];

  // First Cross
  for(size_t j=1;j<LINE;++j)
  {
    res_1 = res_1 and (cc[j][j] != EMPTY)
        and (cc[j][j] == ref);
    res_2 = res_2 and (cc[j][LINE - 1 - j] != EMPTY)
        and (cc[j][LINE - 1 - j] == ref);
  }

  return res_1 or res_2;
}

