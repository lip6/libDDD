/****************************************************************************/
/*								            */
/* This file is part of libDDD, a library for manipulation of DDD and SDD.  */
/*     						                            */
/*     Copyright (C) 2001-2008 Yann Thierry-Mieg, Jean-Michel Couvreur      */
/*                             and Denis Poitrenaud                         */
/*     						                            */
/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU Lesser General Public License as       */
/*     published by the Free Software Foundation; either version 3 of the   */
/*     License, or (at your option) any later version.                      */
/*     This program is distributed in the hope that it will be useful,      */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/*     GNU LEsserGeneral Public License for more details.                   */
/*     						                            */
/* You should have received a copy of the GNU Lesser General Public License */
/*     along with this program; if not, write to the Free Software          */
/*Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*     						                            */
/****************************************************************************/

/** An example resolution of the famous towers of Hanoi puzzle. *
 *  This variant uses only DDD and no saturation,  *
 *  but starts operations when the ring is reached */


#include <cstring>
#include <string>
#include <iostream>
using namespace std;

#include "DDD.h"
#include "DED.h"
#include "MemoryManager.h"
#include "hanoiHom.hh"

    
int main(int argc, char **argv){
  if (argc == 2) {
    NB_RINGS = atoi(argv[1]);
  }

  // Define a name for each variable
  initName();

  // The initial state
  // User program variables should be DDD not GDDD, to prevent their garbage collection
  DDD M0 = GDDD::one ;
  // construct an initial state for the problem, all rings are on pole 0
  for (int i=0; i<NB_RINGS ; i++ ) {
    // note the use of left-concat (adding at the top of the structure), 
    M0 = DDD(i,0, M0);
    // expression is equivalent to : DDD(i,0) ^ MO
    // less expensive than right-concat which forces to recanonize nodes
    // would be written : for ( i--) M0 = M0 ^ DDD(i,0);
  }

  // To store the set of events
  vector<Hom> events;
  for (int i=0 ; i < NB_RINGS ; i++) {
    // Consider one event per ring, it will split into the possible moves when it reaches the ring
    // ie : ev = \Sum_{i \neq j} swap_pole (ring,i,j)
    // with swap_pole defined as in hanoi_v1
    events.push_back(move_ring(i));
  }

  // Fixpoint over events + Id
  DDD ss, tmp = M0;
  do {
    ss = tmp;
    for (vector<Hom>::reverse_iterator it = events.rbegin(); it != events.rend(); it++) {
      tmp = tmp + (*it) (tmp);
    }
  } while (ss != tmp);

  // stats
  cout << "Number of states : " << ss.nbStates() << endl ;
  cout << "Final/Peak nodes : " << ss.size() << "/" << DDD::peak() << endl;
  cout << "Cache entries : " << MemoryManager::nbDED() <<endl ;
  
  MemoryManager::pstats();
  
}
