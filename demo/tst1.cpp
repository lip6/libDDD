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
#include "statistic.hpp"

typedef enum {A, B, C, D,E, F, G} var;
var variables;
const char* vn[]= {"A", "B", "C", "D", "E", "F", "G"};

void initName() {
	for (int i=A; i<=G; i++)
		DDD::varName(i,vn[i]);
}

int main(){
	// Define a name for each variable
	initName();

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

	DDD a(A,1);
	DDD b(A,1,DDD(B,2));
	DDD c=DDD(A,1,DDD(A,1))+DDD(A,2,DDD(B,1));
	DDD d=DDD(A,1,DDD(A,1))+DDD(A,2,DDD(B,2));
	DDD e=DDD(A,1,5,c);

	cout <<"a = A-1-><1> ="<<endl<<a<<endl;
	cout <<"b = A-1->B-2-><1> ="<<endl<<b<<endl;
	cout <<"c = A-1->A-1-><1> + A-2->B-1-><1> ="<< endl<<c<<endl;
	cout <<"d = A-1->A-1-><1> + A-2->B-2-><1>"<< endl<<d<<endl;
	cout <<"e = A-[1,5]->c ="<< endl<<e<<endl;

	// Operations + (union), * (intersection), ^ (concatenation)
	cout <<"*************************************************************"<<endl;
	cout <<"* Operations + (union), * (intersection), ^ (concatenation) *"<<endl;
	cout <<"*************************************************************"<<endl;

	cout <<"c+d="<<endl<<c+d<<endl;
	cout <<"c*d ="<<endl<<c*d<<endl;
	cout <<"c-d ="<<endl<<c-d<<endl;
	cout <<"a^c ="<<endl<<(a^c)<<endl;
	cout <<"c^a ="<<endl<<(c^a)<<endl;
	cout <<"c^d ="<<endl<<(c^d)<<endl;


	DDD f(A,1,DDD(B,1));
	DDD g(A,1,DDD(A,1));
	DDD h = f+g;
	cout << h <<endl;

	Statistic s(h,"b4 garbage",CSV);

	MemoryManager::garbage();

	Statistic s2(h,"after garbage",CSV);

	s.print_header(std::cout);
	s.print_line(std::cout);
	s2.print_line(std::cout);

	return 1;
}
