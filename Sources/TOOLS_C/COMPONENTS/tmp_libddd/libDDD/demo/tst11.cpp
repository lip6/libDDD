#include <iostream>
using namespace std;

#include "DDD.h"
#include "DED.h"
#include "MemoryManager.h"


int main(){

  // Constants null, one , top
  cout <<"*****************************"<<endl;
  cout <<"* Tracking memory leaks in SDD *"<<endl;
  cout <<"* You probably wish to run : *"<<endl;
  cout <<"valgrind --tool=memcheck --leak-check=yes tst11"<<endl;
  cout <<"*****************************"<<endl;


  int i =0;
  while(++i < 1000) {
            DDD d1 =  DDD(1,1);
            DDD d2 =  DDD(1,2);
            SDD s1 = SDD(1,d1);
            SDD s2 = SDD(1,d2);
            SDD s3 = s1+s2;
            //assert(s3==SDD(1,d1)+SDD(1,d2));
            MemoryManager::garbage();
  }

  cout<<MemoryManager::nbDDD()<<endl;
  cout<<MemoryManager::nbHom()<<endl;
  cout<<MemoryManager::nbDED()<<endl<<endl;
 
  MemoryManager::garbage();
  
  cout<<MemoryManager::nbDDD()<<endl;
  cout<<MemoryManager::nbHom()<<endl;
  cout<<MemoryManager::nbDED()<<endl;

  MemoryManager::pstats();


  return 1;
}
