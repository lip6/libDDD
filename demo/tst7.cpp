#include <iostream>
using namespace std;

#include "DDD.h"
#include "DED.h"
#include "MemoryManager.h"

enum {A, B, C, D,E, F, G} variables;
char* vn[]= {"A", "B", "C", "D", "E", "F", "G"};

void initName() {
  for (int i=A; i<=G; i++)
    DDD::varName(i,vn[i]);
}

int main(){
  // Define a name for each variable
  initName();

  DDD d1(1,1);
  DDD d2(2,2);
  d2=d1;
  cout<<"nbDDD="<<MemoryManager::nbDDD()<<endl;
  MemoryManager::garbage();
  cout<<"nbDDD="<<MemoryManager::nbDDD()<<endl;
  return 0;
}
