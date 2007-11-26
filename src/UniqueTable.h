/* -*- C++ -*- */
#ifndef UNIQUETABLE_H
#define UNIQUETABLE_H


#include <ext/hash_set>

/// This class implements a unicity table mechanism, based on an STL hash_set.
/// Requirements on the contained type are thus those of hash_set.
template<typename T>
class UniqueTable{

private:
	
public:
	/// Constructor, builds a default table.
	UniqueTable()
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
    std::pair<typename Table::iterator, bool> ref=table.insert(_g); 

    
	typename Table::iterator ti=ref.first;
	if (!ref.second){
		delete _g;
	}

	return *ti;
// scoped lock released
	}

	/// Returns the current number of filled entries in the table.
	size_t size() const{
		return table.size();
	}

};

#endif
