//
// C++ Implementation: hom/full
//
// Description: 
//
//
// Author: Yann Thierry-Mieg <LIP6, Yann.Thierry-Mieg@lip6fr > (2003-), Jean-Michel Couvreur <LaBRi > (2001), and Denis Poitrenaud (2001) <LIP6>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "hom/full.h"
#include <boost/functional/hash.hpp>


/**
 * An inductive homomorphism Full for check if all hit have been played in the current configuration.
 * If yes we must return it else we cut the way
 */
class _Full:public StrongHom 
{

  private:
  /**
   * List of member variables = homomorphism parameters
   */
    int node_to_ignore;
  public:
    
    /**
   * Default constructor
     */
    _Full() : node_to_ignore(0)
    {
    }

    /**
     * Default constructor
     */
    _Full(int nti) : node_to_ignore(nti)
    {
    }
    
    /**
     * No swkip variable declared beacause we must check all node of the way
     */
  
    /**
     * PHI [1] : called if the terminal 1 is encountered, returns a constant DDD.
     * We can't arrive at this node
     */
    GDDD phiOne() const
    {
      return DDD::one;
    }

    /**
     * PHI [vr,vl] : called on each arc labeled with "vl" when the homomorphism is applied to a node
     * of variable "vr" such that vr is NOT skipped.
     * When the hom is applied to a node this function is called for each arc, and the result
     * (a homomorphism) is applied to the successor node pointed by the arc.
     *  vr ==> Variable of current node
     *  vl ==> arc label (an integer) of current arc
     */
    GHom phi(int vr, int vl) const
    {
      /* Check all node except Node 0 is not empty */
      if(vr != node_to_ignore)
      {
        if(vl==EMPTY)
        {
          return GHom(DDD::null);
        }
        else
        {
          return GHom(vr,vl,GHom(this)); // e-x->_Full()
        }
      }
      else
      {
        return GHom(vr,vl,GHom(this));  // e-x->_Full()
      }
    }
    
    /**
     * Hash function used for unique table storage.
     */
    size_t hash() const
    {
      // hash function should exhibit reasonable spread and involve as many parameters as possible.
      std::size_t seed = 4789;
      boost::hash_combine(seed, node_to_ignore);
      return seed ;
    }

    /**
     * Overloading StrongHom default print with a customized pretty-print
     */
    void print (std::ostream & os) const
    {
      os << "Search for full configuration";
    }

    /**
     * Overload of operator== necessary for unique table storage
     * argument is typed StrongHom as == is part of StrongHom contract, simlarly to java's bool equals(Object)
     */
    bool operator==(const StrongHom &s) const
    {
      // direct "hard" cast to own type is ok, type checks already made in library
      const _Full& ps = dynamic_cast<const _Full&>(s);
      // basic comparator behavior, just make sure you put all attributes there.
      return ps.node_to_ignore == node_to_ignore ;
    }
    
    /**
     * Clone current homomorphism, used for unique storage.
     */
    _GHom * clone () const
    {
      return new _Full(*this);
    }
};


/**
 * Factory of _Full instance
 */
GHom Full ()
{
  return _Full();
}

/**
 * Factory of _Full instance
 */
GHom Full (int nti)
{
  return _Full(nti);
}