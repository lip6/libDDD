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
#include "Nat_OneSuccZero.hpp"
#include "Nat_Const.hpp"


void initName() {
  DDD::varName (NAT,"NAT");
  DDD::varName (LEFT,"LEFT");
  DDD::varName (RIGHT,"RIGHT");
}



// SDD application
SDD saturateSDD (SDD d) {  
  SDD d1 = d;
  do {
    d1 = d;
    d = saturateHom() (d);
    d = saturateNat (d);
  } while ( d != d1);
  return d;
//  return saturateNat(d);
}

int main(int argc, char **argv){

  // Define a name for each variable
  initName();



  SDD ZeroPlusOne = SDDnatPlus 
    ^ SDD ( LEFT, SDDnatZero )
    ^ SDD ( RIGHT, SDDnatOne)  ;
  cout << "0 + 1 : "  ;  printExpression(cout, & ZeroPlusOne); 
  
  SDD OnePlusZero = SDDnatPlus 
    ^ SDD ( LEFT, SDDnatOne )
    ^ SDD ( RIGHT, SDDnatZero)  ;
  cout << "1 + 0 : "  ;  printExpression(cout, & OnePlusZero); 

  SDD OnePlusOne = SDDnatPlus
    ^ SDD ( LEFT, SDDnatOne )
    ^ SDD ( RIGHT, SDDnatOne)  ;
  cout << "1 + 1 : "  ;  printExpression(cout, & OnePlusOne); 


  SDD OnePlusSuccOne = SDDnatPlus
    ^ SDD ( LEFT, SDDnatOne )
    ^ SDD ( RIGHT, SDDnatSucc ^ SDD ( SUCCARG, SDDnatOne))  ;
  cout << "1 + 2 <=> 1 + succ(1) : "  ;  printExpression(cout, & OnePlusSuccOne); cout << endl ;
  

  // The initial state
  SDD M0 =  OnePlusOne + ZeroPlusOne + OnePlusSuccOne + OnePlusZero;
  
  cout << "Input : "  << endl; // << M0 << endl ;
  printExpression(cout, &M0);
  cout << "Number of states : " << M0.nbStates() << endl ;

  // Consider one single saturate event that recursively fires all events 
  // Saturate topmost node <=> reach fixpoint over transition relation
  SDD ss = saturateSDD (M0);

  cout << "Result : " <<endl; // << ss <<endl ;
   printExpression(cout, &ss);
  // stats
  cout << "Number of states : " << ss.nbStates() << endl ;
//  cout << "DDD Final/Peak nodes : " << ss.node_size().second << "/" << DDD::peak() << endl;
  cout << "SDD Final/Peak nodes : " << ss.node_size().first << "/" << SDD::peak() << endl;
  cout << "Cache entries SDD : " <<  MemoryManager::nbSDED() << endl ;
}
