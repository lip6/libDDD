//
// C++ Implementation: hom/take_cell
//
// Description:
//
//
// Author: Yann Thierry-Mieg <LIP6, Yann.Thierry-Mieg@lip6fr > (2003-), Jean-Michel Couvreur <LaBRi > (2001), and Denis Poitrenaud (2001) <LIP6>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "take_cell.hpp"
#include <boost/functional/hash.hpp>



/**
 * An inductive homomorphism TakeCell to play a move of a given player in a given cell.
 */
class _TakeCellWithCheckWinner:public StrongHom
{
  private:
  /**
   * Liste of member variables = homomorphism parameters
   */
    int cell;     // The cell to play : 0 <= cell < 9
    int player;   // The player taking the cell : 0 or 1
  public:

    /**
   * The constructor binds the homomorphism parameters cell and player
     */
    _TakeCellWithCheckWinner ( int c, int p)
  : cell(c), player(p)
    {
    }

    /**
    * Define the target variable to skip in the current route in apply :
     * Here we must skip all variable except the node cell and the 0 Node.
     * 0 Node is the last node of the way, it is the variable used for check if there is already a winner
     */
    bool
        skip_variable(int vr) const
    {
      return vr != cell && vr != 0;
    }

    /**
     * PHI [1] : called if the terminal 1 is encountered, returns a constant DDD.
    * If the cell was correct 0 <= cell < 9 and the state ok this should not happen :
     * return TOP to indicate an error.
     */
    GDDD
        phiOne() const
    {
      return DDD::top;
    }

    /**
     * PHI [vr,vl] : called on each arc labeled with "vl" when the homomorphism is applied to a node
     * of variable "vr" such that vr is NOT skipped.
     * When the hom is applied to a node this function is called for each arc, and the result
     * (a homomorphism) is applied to the successor node pointed by the arc.
     *  vr ==> Variable of current node
     *  vl ==> arc label (an integer) of current arc
     */
    GHom
        phi(int vr, int vl) const
    {
      if(vr!=0)
      {
        /* Configuration 1 : We can take a cell only if nobody get it*/
        if (vl == EMPTY)
        {
    // Take the cell and resume the recursion for check if there is a winner
          return GHom (vr, player, GHom(this) ); // e-(joueur)-> ID
        }
        else
        {
    // the cell is not empty, move is illegal, abort this move.
          return GHom(DDD::null); // Cut the way (0)
        }
      }
      else
      {
        /* Configuration 2 : Check if there is already a winner */
        if(vl != EMPTY)
        {
    // There is a winner : cut the way
          return GHom(DDD::null);
        }
        else
        {
    // No winner detected, stop the recursion
          return GHom (vr, vl, GHom::id );// e-(joueur)-> ID
        }
      }
    }

    /**
     * Hash function used for unique table storage.
     */
    size_t
        hash() const
    {
      // hash function should exhibit reasonable spread and involve as many parameters as possible.
      std::size_t seed = 1291;
      boost::hash_combine(seed, cell);
      boost::hash_combine(seed, player);
      return seed ;
    }

    /**
     * Overloading StrongHom default print with a customized pretty-print
     */
    void
        print (std::ostream & os) const
    {
      os << "takeCellWithCheckWinner( cell:" << cell << ", player:" << player << " )";
    }

    /**
     * Overload of operator== necessary for unique table storage
     * argument is typed StrongHom as == is part of StrongHom contract, simlarly to java's bool equals(Object)
     */
    bool
        operator==(const StrongHom &s) const
    {
      // direct "hard" cast to own type is ok, type checks already made in library
      const _TakeCellWithCheckWinner& ps = dynamic_cast<const _TakeCellWithCheckWinner&>(s);
      // basic comparator behavior, just make sure you put all attributes there.
      return cell == ps.cell && player == ps.player ;
    }

    /**
     * Clone current homomorphism, used for unique storage.
     */
    _GHom *
        clone () const
    {
      return new _TakeCellWithCheckWinner(*this);
    }
};


/**
 * Factory of _TakeCellWithCheckWinner Instance
 */
GHom
    TakeCellWithCheckWinner ( int c1, int player)
{
  return _TakeCellWithCheckWinner(c1,player);
}

