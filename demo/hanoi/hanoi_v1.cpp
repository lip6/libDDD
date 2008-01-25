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


// we use one DDD variable per ring, ring 0 is the topmost, 
// and is stored at the bottom of the DDD
static int NB_RINGS= 5;
// Each variable domain is {0,1,2} expressing the pole the variable is on
static int NB_POLES= 3;

void initName() {
	char buff [12];
	for (int i=0; i< NB_RINGS; i++) {
		sprintf(buff,"ring %d",i);
		DDD::varName(i,buff);
	}
}


// Removes any path such that one of the variables takes value i or value j
// Not test on variable number means this operation should be used from "mid-height"
class _no_ring_above 
    : 
    public StrongHom
    {
        // the 2 poles that have to be clear
        int i_,j_;
        public :
        _no_ring_above (int i, int j) { 
            /// force to have ii < jj since the operation is commutable,  
            // _no_ring_above(i,j) = _no_ring_above(j,i)
            // hence we have a canonical form
            if (i < j) 
            {
                i_ = i; 
                j_ = j; 
            }
            else
            {
                i_ = j ;
                j_ = i ;
            }
        }
        
        GDDD phiOne() const {
            return GDDD::one;
        }     
        
        // reject any path with ANY ring that is on pole i or pole j
        GHom 
        phi(int vr, int vl) const {
            if ( vl == i_ || vl == j_ )
                // cut this branch and exploration
                return GDDD::null;
            else
                // propagate this test
                return GHom(vr,vl,GHom(this));
        }
        
        size_t hash() const {
            return (i_ &  j_<<16)  *  9749;
        }
        
        bool operator==(const StrongHom &s) const {
            _no_ring_above* ps = (_no_ring_above*)&s;
            return i_ == ps->i_ && j_ == ps->j_ ;
        }
        
    };


class _swap_pole : public StrongHom {
    // ring we are trying to move
	int ring_;
    // ori pole number
	int ori_;
    // dest pole number
	int dest_;
    
	public :
	_swap_pole (int ring, int ori, int dest )
    : 
    ring_(ring),
    ori_(ori),
    dest_(dest)
    {
    }
    
	GDDD
    phiOne() const
    {
		return GDDD::one;
	}                   
    
	GHom 
    phi(int vr, int vl) const
    {
		if (vr != ring_ ) 
        {
            // target ring not reached yet : propagate
			return GHom(vr,vl,this);
		} 
        else 
        {
            // ring reached : shift the pole if ori == vl
			if (ori_ != vl) 
            {
                // precondition not met for this event : we try to move ring from ori_ to dest_ but ring is NOT on ori_ !
				return GDDD::null;
			} 
            else 
            {
                // update ring position and test no ring above
                // no_ring_above propagates on the bottom of the DDD ; it returns 0 if preconditions are not met 
                // or a DDD with only paths where the move is legal
				return GHom (ring_ , dest_) & new _no_ring_above(ori_,dest_);
                // another way of writing it is 
                // return GHom (ring_ , dest_, new _no_ring_above(ori_,dest_) );
			}
		}
	}
    
    bool
    skip_variable(int v) const
    {
        return  v != ring_;
    }
    
	size_t
    hash() const
    {
        // a hash function, probably too complex but will work well
        // be careful of multplying by 0 valued attributes...
		return ((6961*(ring_+1)+ 3) * (ori_+7877 )) ^ (dest_-1) ;
	}
    
	bool
    operator==(const StrongHom &s) const
    {
		_swap_pole* ps = (_swap_pole*)&s;
		return ring_ == ps->ring_ && ori_ == ps->ori_ && dest_ == ps->dest_ ;
	}
    
};

// to be more pleasant for users  
GHom swap_pole ( int ring, int ori, int dest ) {
	return new _swap_pole (ring, ori,dest);
}


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
	vector<Hom> events;
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
					events.push_back(swap_pole(i,ori,dest));
				}
			}
		}
	}
    
    // Fixpoint over events + Id
	DDD ss, tmp = M0;
	do {
		ss = tmp;
		for ( vector<Hom>::reverse_iterator it = events.rbegin(); it != events.rend(); it++)
        {
			tmp = tmp + (*it) (tmp);
		}
	} while (ss != tmp);
    
    // stats
	cout << "Number of states : " << ss.nbStates() << endl ;
	cout << "Final/Peak nodes : " << ss.size() << "/" << DDD::peak() << endl;
	cout << "Cache entries : " << MemoryManager::nbDED() <<endl ;
    
    MemoryManager::pstats();
}
