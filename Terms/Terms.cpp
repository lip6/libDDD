#include <cstring>
#include <string>
#include <iostream>
using namespace std;

#include "IntDataSet.h"
#include "DDD.h"
#include "DED.h"
#include "MemoryManager.h"

#include "Nat_PlusZero.hpp"
#include "Nat_Const.hpp"

void initName() {
  DDD::varName (NAT,"NAT");
}

// SDD application
GShom saturateSDD () {
  return GShom::id ;
}

int main(int argc, char **argv){

  // Define a name for each variable
  initName();

  // a dataset for "+"
  IntDataSet plus (vector<int> (1,PLUS) );
  // a dataset for constant 0
  IntDataSet natZero (vector<int> (1, 0) );
  // a dataset for constant 1
  IntDataSet natOne (vector<int> (1, 1) );


  SDD ZeroPlusOne = SDD (NAT, plus) 
    ^ SDD ( NAT , natZero )
    ^ SDD ( NAT , natOne  );
  SDD OnePlusOne = SDD (NAT, plus) 
    ^ SDD ( NAT , natOne )
    ^ SDD ( NAT , natOne  );
  
  // The initial state
  SDD M0 = OnePlusOne + ZeroPlusOne;
  
  // Consider one single saturate event that recursively fires all events 
  // Saturate topmost node <=> reach fixpoint over transition relation
  SDD ss =  saturateSDD() (M0) ;

  cout << "Result : " << ss <<endl ;
  // stats
  cout << "Number of states : " << ss.nbStates() << endl ;
  cout << "DDD Final/Peak nodes : " << ss.node_size().second << "/" << DDD::peak() << endl;
  cout << "SDD Final/Peak nodes : " << ss.node_size().first << "/" << SDD::peak() << endl;
  cout << "Cache entries DDD/SDD : " << MemoryManager::nbDED() <<  "/" <<  MemoryManager::nbSDED() << endl ;
}
