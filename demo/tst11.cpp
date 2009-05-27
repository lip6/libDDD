#include <iostream>
#include <vector>
#include <map>

#include "IntDataSet.h"
#include "DDD.h"
#include "SDD.h"
#include "SDED.h"
#include "MemoryManager.h"

#include "statistic.hpp"

int
main(int argc, char** argv)
{

	int size = 100;
  if( argc == 2 )
  {
    size = atoi(argv[1]);
  }

	SDD c = SDD::null;
	for( int i = 0 ; i < size ; ++i )
	{
			std::vector<int> local_vec;
			local_vec.push_back(i);
			IntDataSet local_ids(local_vec);
			c = c + SDD(3,local_ids);
	}
	
	SDD b = SDD::null;
	for( int i = 0 ; i < size ; ++i )
	{
			std::vector<int> local_vec;
			local_vec.push_back(i);
			IntDataSet local_ids(local_vec);
			b = b + SDD(2,local_ids,c);
	}

	SDD a =  SDD::null;
	for( int i = 0 ; i < size ; ++i )
	{
			std::vector<int> local_vec;
			local_vec.push_back(i);
			IntDataSet local_ids(local_vec);
			a = a + SDD(1,local_ids,b);
	}

	Statistic S = Statistic(a,"tst11");

	S.print_line(std::cout);

}
