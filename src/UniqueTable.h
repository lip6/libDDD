/* -*- C++ -*- */
#ifndef UNIQUETABLE_H
#define UNIQUETABLE_H


#include <ext/hash_set>

#ifdef PARALLEL_DD
#include "tbb/queuing_mutex.h"
#include "tbb/mutex.h"
#endif

/// This class implements a unicity table mechanism, based on an STL hash_set.
/// Requirements on the contained type are thus those of hash_set.
template<typename T>
class UniqueTable{

#ifdef INST_STL
  /// in the context of INST_STL version counts number of lookups in the table.
  long NbAcces;
  /// In the context of INST_STL version, counts number of misses.
  long NbInsertion;
#endif

private:
	
	// typedef tbb::queuing_mutex table_mutex_t;
	typedef tbb::mutex table_mutex_t;
	table_mutex_t table_mutex_;

public:
	/// Constructor, builds a default table.
	UniqueTable()
#ifdef PARALLEL_DD
		:
		table_mutex_()
#endif
#ifdef INST_STL
		:
		NbAcces(0),
		NbInsertion(0)
#endif
	{
	}

  /// Typedef helps hide implementation type (currently gnu gcc's hash_set).
  typedef __gnu_cxx::hash_set<T*> Table;
  /// The actual table, operations on the UniqueTable are delegated on this.
  Table table; // Unique table of GDDD

/* Canonical */
  /// The application operator, returns the address of the value already in 
  /// the table if it exists, or inserts and returns the address of the value inserted.
  /// \param _g the pointer to the value we want to find in the table.
  /// \return the address of an object stored in the UniqueTable such that (*_g == *return_value)
  T *operator()(T *_g){
#ifdef INST_STL
    NbAcces++;
    int nbjumps=0;
    std::pair<typename Table::iterator, bool> ref=table.insert(_g, nbjumps); 
    _g->InstrumentNbJumps(nbjumps);
#else



#ifdef PARALLEL_DD
	table_mutex_t::scoped_lock lock(table_mutex_);
#endif

    std::pair<typename Table::iterator, bool> ref=table.insert(_g); 
#endif
    
	typename Table::iterator ti=ref.first;
	if (!ref.second){
		delete _g;
	}
#ifdef INST_STL
	else {
		NbInsertion++;
	}
#endif

	return *ti;
// scoped lock released
	}

	/// Returns the current number of filled entries in the table.
	size_t size() const{
		return table.size();
	}

#ifdef INST_STL
	/// Prints some statistics relating to UniqueTable effectiveness (only INST_STL version).
	void pstat(bool reinit=true){
		cout << "NbInsertion(" <<NbInsertion << ")*100/NbAccess(" << NbAcces<< ")  = " ;
		cout << ((long)(((long)NbInsertion) * 100)) / ((long)NbAcces) << endl;
		if (reinit ){
			NbAcces=0;
			NbInsertion=0;
		}
	}
#endif
};

#endif
