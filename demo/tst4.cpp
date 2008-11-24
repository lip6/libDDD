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
#include "MemoryManager.h"
#include "PlusPlus.hh"
#include "statistic.hpp"


typedef enum {A, B, C, D,E, F, G} var;
var variables;
const char* vn[]= {"A", "B", "C", "D", "E", "F", "G"};

void initName() {
	for (int i=A; i<=G; i++)
		DDD::varName(i,vn[i]);
}

int main(){
	initName();

	cout <<"****************"<<endl;
	cout <<"* Define DDD u *"<<endl;
	cout <<"****************"<<endl;

	DDD a=DDD(A,1,DDD(A,1));
	DDD b=DDD(C,1,DDD(A,1))+DDD(C,2,DDD(B,3));
	DDD u=a^b;

	cout <<"u="<< endl<<u<<endl;

	cout <<"**************************************************"<<endl;
	cout <<"* Strong Hom <!X++> : incremente all values of X *"<<endl;
	cout <<"**************************************************"<<endl;

	Hom fa = plusplusAll(A);
	Hom fb = plusplusAll(B);
	Hom fc = plusplusAll(C);
	Hom fd = plusplusAll(D);

	cout <<"<!A++>(u)="<< endl<<fa(u)<<endl;
	cout <<"<!B++>(u)="<< endl<<fb(u)<<endl;
	cout <<"<!C++>(u)="<< endl<<fc(u)<<endl;
	cout <<"<!D++>(u)="<< endl<<fd(u)<<endl;

	Statistic s(u,"tst4",CSV);

	s.print_header(cout);
	s.print_line(cout);
	MemoryManager::pstats();

	return 1;
}
