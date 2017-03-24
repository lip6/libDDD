#ifndef __PERMUTE_HH__
#define __PERMUTE_HH__

#include "ddd/Hom.h"

// User function : Construct a Hom 
// Will permute or swap the value of var1 and var2
// e.g.  v1=2 & v2=3    =>> v1=3 & v2=2
GHom permute(int var1,int var2);


#endif
