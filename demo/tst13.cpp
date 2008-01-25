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

#include <iostream>
using namespace std;

#include "DDD.h"
#include "DED.h"
#include "MemoryManager.h"
#include "dotExporter.h"


typedef enum {A, B, C} var;
var variables;
char* vn[]= {"A", "B",  "C"};


void initName() {
  for (int i=A; i<=C; i++)
    DDD::varName(i,vn[i]);
}


int main(){
  // Define a name for each variable
  initName();

  // See example from FMCAD'2002 Ciardo, Radu
  // A first evDD
  DDD ev1 = GDDD(DISTANCE,0,GDDD(A,0,GDDD(DISTANCE,0,GDDD(B,0,GDDD(DISTANCE,0,GDDD(C,0,GDDD(DISTANCE,0))))))) 
    + GDDD(DISTANCE,0,GDDD(A,0,GDDD(DISTANCE,0,GDDD(B,1,GDDD(DISTANCE,2,GDDD(C,0,GDDD(DISTANCE,0)))))))
    + GDDD(DISTANCE,0,GDDD(A,1,GDDD(DISTANCE,1,GDDD(B,0,GDDD(DISTANCE,1,GDDD(C,0,GDDD(DISTANCE,0))))))) 
    + GDDD(DISTANCE,0,GDDD(A,1,GDDD(DISTANCE,1,GDDD(B,1,GDDD(DISTANCE,0,GDDD(C,1,GDDD(DISTANCE,0))))))) 
    + GDDD(DISTANCE,0,GDDD(A,2,GDDD(DISTANCE,2,GDDD(B,0,GDDD(DISTANCE,1,GDDD(C,0,GDDD(DISTANCE,0))))))) 
    + GDDD(DISTANCE,0,GDDD(A,2,GDDD(DISTANCE,2,GDDD(B,1,GDDD(DISTANCE,0,GDDD(C,1,GDDD(DISTANCE,0))))))) ;
  // Another evDD
  DDD ev2 = GDDD(DISTANCE,0,GDDD(A,0,GDDD(DISTANCE,0,GDDD(B,0,GDDD(DISTANCE,0,GDDD(C,0,GDDD(DISTANCE,0))))))) 
    + GDDD(DISTANCE,0,GDDD(A,0,GDDD(DISTANCE,0,GDDD(B,0,GDDD(DISTANCE,0,GDDD(C,1,GDDD(DISTANCE,2))))))) 
    + GDDD(DISTANCE,0,GDDD(A,1,GDDD(DISTANCE,2,GDDD(B,0,GDDD(DISTANCE,0,GDDD(C,0,GDDD(DISTANCE,0))))))) 
    + GDDD(DISTANCE,0,GDDD(A,1,GDDD(DISTANCE,2,GDDD(B,0,GDDD(DISTANCE,0,GDDD(C,1,GDDD(DISTANCE,2))))))) 
    + GDDD(DISTANCE,0,GDDD(A,2,GDDD(DISTANCE,1,GDDD(B,0,GDDD(DISTANCE,0,GDDD(C,0,GDDD(DISTANCE,0))))))) 
    + GDDD(DISTANCE,0,GDDD(A,2,GDDD(DISTANCE,1,GDDD(B,0,GDDD(DISTANCE,0,GDDD(C,1,GDDD(DISTANCE,2))))))) 
    + GDDD(DISTANCE,0,GDDD(A,2,GDDD(DISTANCE,1,GDDD(B,1,GDDD(DISTANCE,2,GDDD(C,1,GDDD(DISTANCE,0))))))) ;

  DDD ev3 = ev1 + ev2 ;
  cout << ev3;

  SDD ev1sdd = SDD(0,ev1);
  cout << ev1sdd <<endl;
  exportDot(ev1sdd,"ev1");
  SDD ev2sdd = SDD(0,ev2);
  cout << ev2sdd <<endl;
  exportDot(ev2sdd,"ev2");
  SDD ev3sdd = SDD(0,ev3);
  cout << ev3sdd <<endl;
  exportDot(ev3sdd,"ev3");


   DDD ev4 = ev3 * ev1;  
   SDD ev4sdd = SDD(0,ev4);
   cout << "ev4 = ev3 * ev1 == ev1 ?" << (ev4==ev1?"true":"false") <<endl;
   exportDot(ev4sdd,"ev4");

   DDD ev5 =  GDDD(DISTANCE,0,GDDD(A,1,GDDD(DISTANCE,0,GDDD(B,1,GDDD(DISTANCE,0,GDDD(C,1,GDDD(DISTANCE,0))))))) ;
   DDD ev6 = ev5 * ev1;
   SDD ev6sdd = SDD(0,ev6);
   cout << "ev6 = ev5 * ev1 " << ev6 <<endl;
   exportDot(ev6sdd,"ev6");


}
