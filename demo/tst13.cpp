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
