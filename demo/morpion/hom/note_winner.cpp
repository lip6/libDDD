//
// C++ Implementation: hom/note_winner
//
// Description: 
//
//
// Author: Yann Thierry-Mieg <LIP6, Yann.Thierry-Mieg@lip6fr > (2003-), Jean-Michel Couvreur <LaBRi > (2001), and Denis Poitrenaud (2001) <LIP6>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "hom/note_winner.hpp"
#include <boost/functional/hash.hpp>

/**
 * An inductive homomorphism _NoteWinner to play a move of a given player in a given cell.
 */
class _NoteWinner:public StrongHom
{

  /**
   * List of variable
   */
  private:
    int case1;     // Cell to check
    int case2;     // Cell to check
    int case3;     // Cell to check
    int nbCheck;   // Number cell to check
    int player;    // The player game to check
  public:
    
    /**
   * Constructor with initialisation
     */
    _NoteWinner ( int c1, int c2, int c3, int nb, int p)
  : case1(c1),case2(c2),case3(c3),nbCheck(nb), player(p)
    {
    }

    
    /**
     * Skip all variable except case1, case2, case3 and Node 0
     */
    bool
        skip_variable(int vr) const
    {
      return (vr != case1) && (vr != case2) && (vr != case3) && vr != 0;
    }
    
  
    /**
     * PHI [1] : It is an error if we arrive here
     */
    GDDD
        phiOne() const
    {
      return GDDD::top;
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
      if(vr != 0)
      {
        /* We work on cell case1, case2 or case3, and we msut check if all case is equal to player */
        if(vl== player)
        {
          if(nbCheck-1>0)
          {
            /* If there is more cell to check, we resume */
            return GHom (vr, vl, NoteWinner ( case1, case2, case3, nbCheck-1, player) ); // Propagation n-1
          }
          else
          {
            /* We have found a winner, resume the homomorphism until node 0 for note it */
            return GHom (vr, vl, GHom(this) ); //  Couper le chenmin 0
          }
        }
        else
        {
          /* No winner found, stop the recursion */
          return GHom (vr, vl, GHom::id );
        }
      }
      else
      {
        /* We work on node 0 and we have detected a winner */
        if(vl == EMPTY)
        {
          /* If there is no winner, we note it and stop the recursion */
          return GHom (vr, player, GHom::id );
        }
        else
        {
          if(vl == player)
          {
    /* We have already found a winner, and the winner the same player, it is a possible way,
      * for exemple we are in this configuration :
            * XXX    XXX    XXX
            * X00 or 00X or 0X0 or ...
            * X00    00X    0X0
            * So we keep the same winner and stop the recursion
    */
            return GHom (vr, vl, GHom::id );
          }
          else
          {
      /* We have found a double different winner in the configuration game.
            * This is not a possible way of the system, so we cut it
      */
            return GHom(DDD::null);
          }
        }
      }
    }
    
    /**
     * Hash function used for unique table storage.
     */
    size_t
        hash() const
    {
      std::size_t seed = 2179;
      boost::hash_combine(seed, case1);
      boost::hash_combine(seed, case2);
      boost::hash_combine(seed, case3);
      boost::hash_combine(seed, player);
      return seed ;
    }
  
  /**
     * Overloading StrongHom default print with a customized pretty-print
   */
    void
        print (std::ostream & os) const
    {
      os << "winnerCheckOn( case: [" << case1 << case2 << case3 << "]" << " for player:" << player << " )";
    }

    
    
    /**
     * Overload of operator== necessary for unique table storage
     * argument is typed StrongHom as == is part of StrongHom contract, simlarly to java's bool equals(Object)
     */
    bool
        operator==(const StrongHom &s) const
    {
      const _NoteWinner& ps = dynamic_cast<const _NoteWinner&>(s);
      return case1 == ps.case1 && player == ps.player && case2 == ps.case2 && case3 == ps.case3 && nbCheck == ps.nbCheck;
    }
    
    /**
     * Clone current homomorphism, used for unique storage.
     */
    _GHom * clone () const
    {
      return new _NoteWinner(*this);
    }
};


/**
 * Factory of NoteWinner Instance
 */
GHom
    NoteWinner ( int c1, int c2, int c3,int nb, int player)
{
  return _NoteWinner(c1,c2,c3,nb,player);
}