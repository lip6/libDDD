
#include "IntDataSet.h"


// static initialization
UniqueTable<std::vector<int> > IntDataSet::canonical = UniqueTable<vector<int> > ();
vector<int> * IntDataSet::empty_ = canonical(new vector<int>(0));
