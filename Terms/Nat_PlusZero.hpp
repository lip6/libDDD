#ifndef NAT_PLUS_ZERO__HH
#define NAT_PLUS_ZERO__HH

#include "Nat_Const.hpp"
#include "Hom_Select.hpp"

SDD applyZeroPlusX (const SDD & d) {
  GShom select_zero_plus_test = select_hom(NAT, &natPlus, select_hom(LEFT, &SDDnatZero, GShom::id));
  GSDD d1 = select_zero_plus_test(d);
  return extract_value (RIGHT) (d1) 
    + (d-d1);
}

#endif // NAT_PLUS_ZERO__HH
