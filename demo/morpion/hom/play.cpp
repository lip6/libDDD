//
// C++ Implementation: hom/play
//
// Description:
//
//
// Author: Yann Thierry-Mieg <LIP6, Yann.Thierry-Mieg@lip6fr > (2003-), Jean-Michel Couvreur <LaBRi > (2001), and Denis Poitrenaud (2001) <LIP6>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "hom/play.hpp"
#include <cassert>


/**
 * An inductive homomorphism to play a move of a given player in a given cell.
 */

class _Play: public StrongHom
{

  private:
    /**
   * Liste of member variables = homomorphism parameters
     */
    int cell;     // The cell to play : 0 <= cell < 9
    game_status_type player;   // The player taking the cell : 0 or 1
  public:

    /**
   * The constructor binds the homomorphism parameters cell and player
     */
    _Play ( int c, game_status_type p )
  : cell ( c ), player ( p )
    {
    }

    /**
     * Define the target variable to skip in the current route in apply
     */
    bool
        skip_variable ( int vr ) const
    {
      return vr != cell && vr != static_cast<int>(STATE_SYSTEM_CELL);
    }

    /**
     * PHI [1] : called if the terminal 1 is encountered, returns a constant DDD
     */
    GDDD
        phiOne() const
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
    GHom
        phi ( int vr, int vl ) const
    {
      if ( vr != static_cast<int>(STATE_SYSTEM_CELL) )
      {
        /* Configuration 1 : We can take a cell only if nobody get it */
        if ( vl == EMPTY )
        {
          // Take the cell and resume the recursion for check if there is a winner
          return GHom ( vr, player, GHom::id ); // e-(joueur)-> ID
        }
        else
        {
          // the cell is not empty, move is illegal, abort this move.
          return GHom ( DDD::null ); // Cut the way (0)
        }
      }
      else
      {
        if ( vl < 0 )
        {
          /* It stays some hit to play */
          if ( vl == TO_PA )
          {
            /* Pair number : Player A must play : Decrement the number by one */
            return GHom ( vr, TO_PB, _Play ( cell, PA ) ); // Say player A to play
          }
          else if ( vl == TO_PB )  // If number is impair, player B must play
          {
            /* Impair Number : Player B must play :  Decrement the number by one */
            return GHom ( vr, TO_PA, _Play ( cell, PB ) ); // Say player B to play
          }
          else
          {
            assert(false);
          }
        }
        else
        {
          return GHom ( DDD::null ); // Cut the way (0)
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
      std::size_t seed = 3863;
      seed ^=  cell + (34527* (player+2));
      return seed ;
    }

    /**
     * Overloading StrongHom default print with a customized pretty-print
     */
    void
        print ( std::ostream & os ) const
    {
      os << "Player( cell:" << cell << ", player:" << player << " )";
    }

    /**
     * Overload of operator== necessary for unique table storage
     * argument is typed StrongHom as == is part of StrongHom contract, simlarly to java's bool equals(Object)
     */
    bool
        operator== ( const StrongHom &s ) const
    {
      // direct "hard" cast to own type is ok, type checks already made in library
      const _Play& ps = dynamic_cast<const _Play&> ( s );
      // basic comparator behavior, just make sure you put all attributes there.
      return cell == ps.cell && player == ps.player ;
    }

    /**
     * Clone current homomorphism, used for unique storage.
     */
    _GHom *
        clone () const
    {
      return new _Play ( *this );
    }
};


/**
 * Factory of _TakeCellWithCheckWinner Instance
 */
Hom
    Play ( int c1, game_status_type player )
{
  return GHom(_Play ( c1, player ));
}

/**
 * Factory of _TakeCellWithCheckWinner Instance
 */
Hom
    Play ( int c1 )
{
  return GHom(_Play ( c1, PA ));
}



