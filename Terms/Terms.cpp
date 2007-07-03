#include <cstring>
#include <string>
#include <iostream>
using namespace std;

#include "IntDataSet.h"
#include "DDD.h"
#include "DED.h"
#include "MemoryManager.h"

#include "Nat_PlusZero.hpp"
#include "Nat_PlusSucc.hpp"
#include "Nat_Const.hpp"

void initName() {
  DDD::varName (NAT,"NAT");
}



// SDD application
SDD saturateSDD (SDD d) {  
  SDD d1 = d;
  do {
    d1 = d ;
    d = applyZeroPlusX (d) ;
    d = applySuccPlusXY(d) ;
  } while (d != d1);
  return d1;
}

int main(int argc, char **argv){

  // Define a name for each variable
  initName();



  SDD ZeroPlusOne = SDDnatPlus 
    ^ SDD ( LEFT, SDDnatZero )
    ^ SDD ( RIGHT, SDDnatOne)  ;
  SDD OnePlusOne = SDDnatPlus
    ^ SDD ( LEFT, SDDnatOne )
    ^ SDD ( RIGHT, SDDnatOne)  ;
  SDD OnePlusTwo = SDDnatPlus
    ^ SDD ( LEFT, SDDnatOne )
    ^ SDD ( RIGHT, SDD(SDDnatSucc ^ SDD ( SUCCARG, SDDnatOne)) )  ;
  
  // The initial state
  SDD M0 = OnePlusOne + ZeroPlusOne+ OnePlusTwo;
  
  cout << "Input : " << M0 << endl ;
  // Consider one single saturate event that recursively fires all events 
  // Saturate topmost node <=> reach fixpoint over transition relation
  SDD ss = saturateSDD (M0);

  cout << "Result : " << ss <<endl ;
  // stats
  cout << "Number of states : " << ss.nbStates() << endl ;
//  cout << "DDD Final/Peak nodes : " << ss.node_size().second << "/" << DDD::peak() << endl;
  cout << "SDD Final/Peak nodes : " << ss.node_size().first << "/" << SDD::peak() << endl;
  cout << "Cache entries SDD : " <<  MemoryManager::nbSDED() << endl ;
}
