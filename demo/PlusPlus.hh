#ifndef __PLUSPLUS_HH
#define __PLUSPLUS_HH

#include "Hom.h"
// User function : Construct a Hom for a Strong Hom _plusplus
// Increments the value of the first occurrence of var
GHom plusplus(int vr);

// Increments the  value of all occurrences of var
GHom plusplusAll(int vr);

// Increments the  value the next variable 
GHom plusplusFirst();

#endif
