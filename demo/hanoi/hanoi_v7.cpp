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
 *  v7 : This variant introduces the use of SDD  *
 *  A single SDD variable is used, the program builds upon v6. *
*/


#include <cstring>
#include <string>
#include <iostream>
using namespace std;

#include "DDD.h"
#include "DED.h"
#include "MemoryManager.h"
#include "init.hh"
#include "hanoiHom.hh"
  
  
int main(int argc, char **argv){
  if (argc == 2) {
    NB_RINGS = atoi(argv[1]);
  }

 	d3::init init;

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

  // Add an SDD external var, bearing number 0
  SDD M1 = SDD ( 0 , M0 ) ;

  // Consider one single saturate event that recursively fires all events 
  // Saturate topmost node <=> reach fixpoint over transition relation
  SDD ss =  saturateSDD_singleDepth() (M1) ;

  // stats
  cout << "Number of states : " << ss.nbStates() << endl ;
  cout << "DDD Final/Peak nodes : " << ss.node_size().second << "/" << DDD::peak() << endl;
  cout << "SDD Final/Peak nodes : " << ss.node_size().first << "/" << SDD::peak() << endl;
  cout << "Cache entries DDD/SDD : " << MemoryManager::nbDED() <<  "/" <<  MemoryManager::nbSDED() << endl ;
}
