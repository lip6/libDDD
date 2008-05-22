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
 *  This variant uses only DDD and no saturation */
#include <cstring>
#include <string>
#include <iostream>
using namespace std;

#include "DDD.h"
#include "DED.h"
#include "MemoryManager.h"
#include "hanoiHom.hh"

int 
main(int argc, char **argv)
{
	if (argc == 2)
    {
		NB_RINGS = atoi(argv[1]);
	}
    
    // Define a name for each variable
	initName();
    
    // The initial state
    // User program variables should be DDD not GDDD, to prevent their garbage collection
	DDD M0 = DDD::one ;
    // construct an initial state for the problem, all rings are on pole 0
	for (int i=0; i<NB_RINGS ; i++ )
    {
        // note the use of left-concat (adding at the top of the structure), 
		M0 = DDD(i,0, M0);
        // expression is equivalent to : DDD(i,0) ^ MO
        // less expensive than right-concat which forces to recanonize nodes
        // would be written : for ( i--) M0 = M0 ^ DDD(i,0);
	}
    
    // To store the set of events
	Hom events = GHom::id;

    std::set<GHom> union_hom;

	for (int i=0 ; i < NB_RINGS ; i++)
    {
        // No tricks used : consider all 6 = NB_POLES * (NB_POLES-1) events per ring 
        // ie 3 poles : 0->1, 0->2, 1->2, 1->0, 2->1, 2->0
		for (int ori = 0; ori < NB_POLES ; ori++ )
        {
			for (int dest = 0; dest < NB_POLES ; dest++ )
            {
				if (ori != dest)
                {
                    union_hom.insert(swap_pole(i,ori,dest));
				}
			}
		}
	}

    union_hom.insert(GHom::id);
    events = GHom::add(union_hom);
    
    // Fixpoint over events + Id
	DDD ss, tmp = M0;
	do {
		ss = tmp;
        tmp = events(tmp);
	} while (ss != tmp);
    
 // stats
  Statistic S = Statistic(ss,"hanoiv1bis." + toString(NB_RINGS) + "." + toString(NB_POLES),CSV);  
  S.print_header(std::cout);
  S.print_line(std::cout);

}
