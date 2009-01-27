//
// C++ Implementation: hom/winner
//
// Description: 
//
//
// Author: Yann Thierry-Mieg <LIP6, Yann.Thierry-Mieg@lip6fr > (2003-), Jean-Michel Couvreur <LaBRi > (2001), and Denis Poitrenaud (2001) <LIP6>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "hom/winner.hpp"
#include <boost/functional/hash.hpp>

/**
 * An inductive homomorphism
 */

class _CheckCellWinner: public StrongHom
{
  private:
    /**
   * Liste of member variables = homomorphism parameters
     */
    int cell;     // The cell game
    game_status_type player;   // The player taking the cell : 0 or 1
  public:

    /**
   * The constructor binds the homomorphism parameters
     */
    _CheckCellWinner ( game_status_type p, int c )
  : cell ( c ), player ( p )
    {
    }

    /**
     * Define the target variable to skip in the current route in apply
     */
    bool
        skip_variable ( int vr ) const
    {
      return vr != cell;
    }

    // tag this homomorphism as only making selections
    bool
        is_selector () const
    {
      return true;
    }

    /**
     * PHI [1] : called if the terminal 1 is encountered, returns a constant DDD
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
        phi ( int vr, int vl ) const
    {
      if ( vl == player )
      {
        return GHom ( vr, vl, GHom::id ); // e-(joueur)-> ID
      }
      else
      {
        return GHom ( DDD::null ); // Cut the way (0)
      }
    }

    /**
     * Hash function used for unique table storage.
     */
    size_t
        hash() const
    {
      // hash function should exhibit reasonable spread and involve as many parameters as possible.
      std::size_t seed = 4789;
      boost::hash_combine ( seed, player );
      boost::hash_combine ( seed, cell );
      return seed ;
    }

    /**
     * Overloading StrongHom default print with a customized pretty-print
     */
    void
        print ( std::ostream & os ) const
    {
      os << "checkCellWin(player:" << player << ", cell:" << cell <<  ")";
    }

    /**
     * Overload of operator== necessary for unique table storage
     * argument is typed StrongHom as == is part of StrongHom contract, simlarly to java's bool equals(Object)
     */
    bool
        operator== ( const StrongHom &s ) const
    {
      // direct "hard" cast to own type is ok, type checks already made in library
      const _CheckCellWinner& ps = dynamic_cast<const _CheckCellWinner&> ( s );
      // basic comparator behavior, just make sure you put all attributes there.
      return player == ps.player && cell == ps.cell ;
    }

    /**
     * Clone current homomorphism, used for unique storage.
     */
    _GHom *
        clone () const
    {
      return new _CheckCellWinner ( *this );
    }
};


/**
 * Factory of _TakeCellWithCheckWinner Instance
 */
Hom
    CheckCellWinner ( game_status_type player , int cell )
{
  return GHom(_CheckCellWinner ( player, cell ));
}


