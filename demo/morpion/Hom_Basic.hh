#ifndef __HOM_BASIC__H_
#define __HOM_BASIC__H_

#include "DDD.h"
#include "Hom.h"
#include "SDD.h"
#include "SHom.h"


// keep paths where variable var is equal to val
GHom varEqState (int var, int val) ;
// keep paths where variable var is NOT equal to val
GHom varNeqState (int var, int val) ;
// keep paths where variable var is strictly greater than val
GHom varGtState (int var, int val) ;
// keep paths where variable var is <= to val
GHom varLeqState (int var, int val) ;

GHom setVarConst (int var, int val) ;

#endif
