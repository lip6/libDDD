#ifndef __HANOI_HOM_HH__
#define __HANOI_HOM_HH__

#include <string>
#include "Hom.h"
#include "SHom.h"
#include "statistic.hpp"

// Global constants.
// we use one DDD variable per ring, ring 0 is the topmost, 
// and is stored at the bottom of the DDD
extern int NB_RINGS ;
// Each variable domain is {0,1,2} expressing the pole the variable is on
extern int NB_POLES ; 

// name the DDD variables for prettier print.
void initName() ;

// int -> string
std::string toString (int i);
// float -> string
std::string toString (double i);

// moves ring "ring" from pole ori to pole dest if possible.
GHom swap_pole ( int ring, int ori, int dest ) ;

// attempt to move a ring to all other poles. Origin pole is read from the structure.
// returns strictly successors
GHom move_ring ( int ring ) ;

// attempt to move a ring to all other poles. Origin pole is read from the structure.
// equivalent behavior to (move_ring + id), i.e. adds to existing states
GHom move_ring_id ( int ring ) ;

// move the ring "ring" and all rings above it (below in the encoding) using a fixpoint.
// Adds to existing states similarly to move_ring_id.
// Normally the user will just call move_ring_sat (NB_RING -1).
GHom move_ring_sat ( int ring ) ;

// move all the rings using a fixpoint. Generic version with no "ring" parameter.
// Adds to existing states similarly to move_ring_id.
GHom move_ring_sat_gen () ;

// move all the rings using a fixpoint. Uses recursive calls to the explicit saturate function
// Adds to existing states similarly to move_ring_id.
GHom saturate () ;

// SDD application to saturate. 
// Used in the case there is only one SDD variable that contains the whole ring problem.
// Uses saturate (see above) internally.
GShom saturateSDD_singleDepth ();

// SDD application to saturate when working with IntDataSet SDD. 
// Used in the single depth case, i.e. all SDD variables have domain Natural (IntDataSet)
// Builds upon saturate version for DDD.
GShom saturateSDD_IntData ();

#endif
