#include <iostream>
using namespace std;

#include "DDD.h"
#include "DED.h"
#include "MemoryManager.h"


int main(){

  // Constants null, one , top
  cout <<"*****************************"<<endl;
  cout <<"* Constants null, one , top *"<<endl;
  cout <<"*****************************"<<endl;

  cout <<"DDD::null="<<endl<<DDD::null<<endl;
  cout <<"DDD::one="<<endl<<DDD::one<<endl;
  cout <<"DDD::top="<<endl<<DDD::top<<endl;

  // Create a DDD: var -- val -> d
  cout <<"***************"<<endl;
  cout <<"* Create DDDs *"<<endl;
  cout <<"***************"<<endl;

  while(1) {
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
