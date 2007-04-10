#include <iostream>
#include <cassert>
using namespace std;
#include "DataSet.h"
#include "DDD.h"
#include "DED.h"
#include "SDD.h"
#include "SDED.h"
#include "MemoryManager.h"
#define HADDOCK_EOB 666
#define HADDOCK_INTEGER 1


class _selectAndSet : public StrongHom {
    int oldPC; int newPC;
    public :
    _selectAndSet (int selectAndSet_oldPC, int selectAndSet_newPC) : oldPC(selectAndSet_oldPC), newPC(selectAndSet_newPC){}
    GDDD phiOne() const { return GDDD::top;}

    GHom phi(int var, int val) const{
	assert (var==HADDOCK_INTEGER);
	if (val==oldPC) return DDD(var,newPC);
	else return GDDD::null;
    }

    size_t hash() const {return 640049*oldPC+640049^2*newPC;}

    bool operator ==(const StrongHom &s) const {_selectAndSet *ps = (_selectAndSet*) &s; return oldPC == ps->oldPC && newPC == ps->newPC;}
};

GHom selectAndSet(int oldPC, int newPC) {return GHom(new _selectAndSet(oldPC, newPC));}

class _rebuild : public StrongShom {
int v; SDD t; 
public :
_rebuild (int rebuild_v, const GSDD& rebuild_t) : v(rebuild_v), t(rebuild_t){}
GSDD phiOne() const { return GSDD::top;}

GShom phi(int var, const DataSet& val) const{
//    cout<<"rebuild "<<v<<" "<<var<<" "<<t<<endl;
  //  cout<<((GDDD)((DDD&)val))<<endl;
  //  cout<<((GSDD)((SDD&)val))<<endl;
    if (var==HADDOCK_EOB)
//	return SDD(v,t)^GShom::id;
	return GShom(v,t,GShom::id);
    else
//	return GShom(new _rebuild(v,t^((GSDD) ((SDD&)val))));
	return GShom(new _rebuild(v,t^GSDD(var,val)));
}

size_t hash() const {return 640061*v;} //TODO

/*void mark() {
    t.mark();
}*/

bool operator ==(const StrongShom &s) const {_rebuild *ps = (_rebuild*) &s; return v == ps->v && t == ps->t;}
};

GShom rebuild(int v) {
    //return GShom::id;
    return GShom(new _rebuild(v, GSDD::one));
}


class _updatePC : public StrongShom {
int oldPC; int newPC; GShom hom; 
public :
_updatePC (int updatePC_oldPC, int updatePC_newPC, const GShom& updatePC_hom) : oldPC(updatePC_oldPC), newPC(updatePC_newPC), hom(updatePC_hom){}
GSDD phiOne() const { return GSDD::top;}

GShom phi(int var, const DataSet& val) const{
    //cout<<"update"<<endl;
    assert (typeid(val)==typeid(DDD));
    assert (var==1);
    DDD in = selectAndSet(oldPC,newPC) ((GDDD)((DDD&)val));
    if (in == GDDD::null) {
	//cout<<"NULL : "<<oldPC<<" "<<((GDDD)((DDD&)val));
	return GSDD::null;
    }
    else {
	//cout<<in<<endl;
	return GShom (var,in,GShom(hom));
    }
}

size_t hash() const {return oldPC+newPC/*+hom*/;}

/*void mark() const {
    hom.mark();
}*/

bool operator ==(const StrongShom &s) const {_updatePC *ps = (_updatePC*) &s; return oldPC == ps->oldPC && newPC == ps->newPC && hom == ps->hom;}
};

GShom updatePC(int oldPC, int newPC, const GShom& hom) {return GShom(new _updatePC(oldPC, newPC, hom));}


class _setPC : public StrongShom {
int index; int oldPC; int newPC; 
DataSet* essai;
public :
_setPC (int setPC_index, int setPC_oldPC, int setPC_newPC, const DataSet& _essai) : index(setPC_index), oldPC(setPC_oldPC), newPC(setPC_newPC), essai(_essai.newcopy()){}
~_setPC(){delete essai;}
GSDD phiOne() const {
    cout<<"ONE SETPC "<<index<<" "<<oldPC<<" "<<newPC<<endl;
    return GSDD::top;}

GShom phi(int var, const DataSet& val) const{
    if (var==index) {
    DDD in = DDD(HADDOCK_EOB,0,GDDD::one);
	return rebuild(var)&
	    updatePC(oldPC, newPC,GShom::id)&
	    (((GSDD)((SDD&)val))^
	     SDD(HADDOCK_EOB,in)^
	     (GShom::id));
    }
    else {
	return GShom(var,val,this);
    }
}

/*void mark() {essai->set_mark();}*/

size_t hash() const {return 746981*(index^(747547*oldPC+newPC));}

bool operator ==(const StrongShom &s) const {_setPC *ps = (_setPC*) &s; return index == ps->index && oldPC == ps->oldPC && newPC == ps->newPC && essai->set_equal(*(ps->essai));}
};

GShom setPC(int index, int oldPC, int newPC, const DataSet& essai) {
    return GShom(new _setPC(index, oldPC, newPC, essai));
}


int main(){

  // Constants null, one , top
  cout <<"*****************************"<<endl;
  cout <<"* Tracking memory leaks in SDD *"<<endl;
  cout <<"* You probably wish to run : *"<<endl;
  cout <<"valgrind --tool=memcheck --leak-check=yes tst11"<<endl;
  cout <<"*****************************"<<endl;


 // int i =0;
 // while(++i < 1000) 
  {
            DDD d1 =  DDD(1,0);
            DDD d2 =  DDD(1,0);
            SDD s1 = SDD(1,d1);
            SDD s2 = SDD(1,d2);
            SDD r = SDD(1,s1,SDD(2,s1));
            for (int j=0; j<30; j++) {
                for (int k=0; k<30; k++) {
                   r = r + setPC(2,k,k+1,s2)(r);
                }
                r = r+ setPC(1,j,j+1,s1)(r);
            MemoryManager::garbage();
            }
            cout<<r;
            MemoryManager::garbage();
            MemoryManager::garbage();
            
  }

  cout<<MemoryManager::nbDDD()<<endl;
  cout<<MemoryManager::nbHom()<<endl;
  cout<<MemoryManager::nbDED()<<endl<<endl;
 
  MemoryManager::garbage();
  
  cout<<MemoryManager::nbDDD()<<endl;
  cout<<MemoryManager::nbHom()<<endl;
  cout<<MemoryManager::nbDED()<<endl;

  // This is not nice at all, but a second garbage is necessary to really clear the nodes
  // hopefully will be corrected in the future
  MemoryManager::garbage();
  MemoryManager::pstats();
  MemoryManager::garbage();
  MemoryManager::garbage();


  return 1;
}
