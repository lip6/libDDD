#ifndef __SET_VAR_HH__
#define __SET_VAR_HH__

#include "Hom.h"
// User functions : Construct hom

// Assign val to the first occurrence of variable var
GHom setCst(int var,int val);
// Assign the value of variable var2 to var1, order insensitive.
GHom setVar(int var1,int var2);


#endif
