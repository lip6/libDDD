#ifndef __HOM_BASIC__H_
#define __HOM_BASIC__H_

#include "ddd/DDD.h"
#include "ddd/Hom.h"
#include "ddd/SDD.h"
#include "ddd/SHom.h"


// keep paths where variable var is equal to val
GHom varEqState (int var, int val) ;
// keep paths where variable var is NOT equal to val
GHom varNeqState (int var, int val) ;
// keep paths where variable var is strictly greater than val
GHom varGtState (int var, int val) ;
// keep paths where variable var is strictly < val
GHom varLtState (int var, int val) ;
// keep paths where variable var is <= to val
GHom varLeqState (int var, int val) ;
// keep paths where variable var is >= to val
GHom varGeqState (int var, int val) ;
// set a var to a constant
GHom setVarConst (int var, int val) ;
// increment or decrement the value of var by val
GHom incVar (int var, int val) ;

// keep paths where variable var is equal to val
GHom varEqVar (int var, int var2) ;
// keep paths where variable var is NOT equal to var2
GHom varNeqVar (int var, int var2) ;
// keep paths where variable var is strictly greater than var2
GHom varGtVar (int var, int var2) ;
// keep paths where variable var is strictly < var2
GHom varLtVar (int var, int var2) ;
// keep paths where variable var is <= to var2
GHom varLeqVar (int var, int var2) ;
// keep paths where variable var is >= to var2
GHom varGeqVar (int var, int var2) ;

#include <functional>

/// an explicit representation of a state
typedef std::vector<DDD::val_t> state_t;
/// A visitor callback function
typedef std::function<void(state_t &)> callback_t;
/// Explicit conversion : visit every path in the DDD (variable ids are removed)
void iterate (const GDDD & node, callback_t * cb);
/// Explicit conversion : visit every path in the DDD (variable ids are removed)
void iterate (const GSDD & node, callback_t * cb);


#endif
