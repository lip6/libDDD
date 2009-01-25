// author : S. Hong,Y.Thierry-Mieg
// date : Jan 2009

#include "morpion_hom_v2.hpp"
#include <boost/functional/hash.hpp>


// The player designated plays in a free cell, if any are available. 
GHom 
PlayAnyFreeCell (int player) {
  std::set<GHom> nextAAset;
  for (int i=0; i< NBCASE; ++i)
  {
    nextAAset.insert( Play (i, player) );
  }
  return GHom::add(nextAAset);
}


// check if a player has won
// Select any path where the designated player has won : i.e has three aligned cross or circle.
GHom 
CheckIsWinner (int player) {
    std::set<GHom> winAAset;
   
    // lines 
  winAAset.insert ( CheckCellWinner (player, 0) & CheckCellWinner (player, 1) & CheckCellWinner (player, 2) ) ;
  winAAset.insert( CheckCellWinner (player, 3) & CheckCellWinner (player, 4) & CheckCellWinner (player, 5) ) ;
  winAAset.insert( CheckCellWinner (player, 6) & CheckCellWinner (player, 7) & CheckCellWinner (player, 8) ) ;
  
  // cols
  winAAset.insert( CheckCellWinner (player, 0) & CheckCellWinner (player, 3) & CheckCellWinner (player, 6) ) ;
  winAAset.insert( CheckCellWinner (player, 1) & CheckCellWinner (player, 4) & CheckCellWinner (player, 7) ) ;
  winAAset.insert( CheckCellWinner (player, 2) & CheckCellWinner (player, 5) & CheckCellWinner (player, 8) ) ;

  // diagonals
  winAAset.insert( CheckCellWinner (player, 0) & CheckCellWinner (player, 4) & CheckCellWinner (player, 8) ) ;
  winAAset.insert( CheckCellWinner (player, 2) & CheckCellWinner (player, 4) & CheckCellWinner (player, 6) ) ;

  return GHom::add(winAAset);
}



/**
 * An inductive homomorphism to play a move of a given player in a given cell.
 */
class _Play:public StrongHom
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
    _Play ( int c, int p)
  : cell(c), player(p)
    {
    }

    /**
     * Define the target variable to skip in the current route in apply
     */
    bool
        skip_variable(int vr) const
    {
      return vr != cell && vr!=9;
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
        phi(int vr, int vl) const
    {
      if(vr!=9)
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
        if(vl==EMPTY)
        {
          return GHom (vr, vl, GHom(this) ); // e-(joueur)-> ID
        }
        else
        {
          return GHom(DDD::null); // Cut the way (0)
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
      std::size_t seed = 0;
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
      os << "Player( cell:" << cell << ", player:" << player << " )";
    }

    /**
     * Overload of operator== necessary for unique table storage
     * argument is typed StrongHom as == is part of StrongHom contract, simlarly to java's bool equals(Object)
     */
    bool
        operator==(const StrongHom &s) const
    {
      // direct "hard" cast to own type is ok, type checks already made in library
      const _Play& ps = dynamic_cast<const _Play&>(s);
      // basic comparator behavior, just make sure you put all attributes there.
      return cell == ps.cell && player == ps.player ;
    }
    
    /**
     * Clone current homomorphism, used for unique storage.
     */
    _GHom *
        clone () const
    {
      return new _Play(*this);
    }
};


/**
 * Factory of _TakeCellWithCheckWinner Instance
 */
GHom 
    Play ( int c1, int player)
{
  return _Play(c1,player);
}











/**
 * An inductive homomorphism
 */
class _NoteWinner:public StrongHom
{
  private:
  /**
   * Liste of member variables = homomorphism parameters
   */
    int player;   // The player taking the cell : 0 or 1
  public:
    
    /**
   * The constructor binds the homomorphism parameters
     */
    _NoteWinner ( int p)
  : player(p)
    {
    }

    /**
     * Define the target variable to skip in the current route in apply
     */
    bool
        skip_variable(int vr) const
    {
      return vr!=9;
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
        phi(int vr, int vl) const
    {
      if(vl==EMPTY)
      {
        return GHom (vr, player, GHom::id ); // e-(joueur)-> ID
      }
      else if (vl == player)
      {
        return GHom (vr, vl, GHom::id ); // e-(vl)-> ID
      }
      else
      {
        return GHom(DDD::null); // Cut the way (0)
      }
    }
    
    /**
     * Hash function used for unique table storage.
     */
    size_t
        hash() const
    {
      // hash function should exhibit reasonable spread and involve as many parameters as possible.
      std::size_t seed = 0;
      boost::hash_combine(seed, player);
      return seed ;
    }
  
    /**
     * Overloading StrongHom default print with a customized pretty-print
     */
    void
        print (std::ostream & os) const
    {
      os << "Note Winner(player:" << player << " )";
    }

    /**
     * Overload of operator== necessary for unique table storage
     * argument is typed StrongHom as == is part of StrongHom contract, simlarly to java's bool equals(Object)
     */
    bool
        operator==(const StrongHom &s) const
    {
      // direct "hard" cast to own type is ok, type checks already made in library
      const _NoteWinner& ps = dynamic_cast<const _NoteWinner&>(s);
      // basic comparator behavior, just make sure you put all attributes there.
      return player == ps.player ;
    }
    
    /**
     * Clone current homomorphism, used for unique storage.
     */
    _GHom *
        clone () const
    {
      return new _NoteWinner(*this);
    }
};


/**
 * Factory of _TakeCellWithCheckWinner Instance
 */
GHom 
    NoteWinner (int player)
{
  return _NoteWinner(player);
}

















/**
 * An inductive homomorphism
 */
class _CheckCellWinner:public StrongHom
{
  private:
  /**
   * Liste of member variables = homomorphism parameters
   */
    int cell;     // The cell game
    int player;   // The player taking the cell : 0 or 1
  public:
    
    /**
   * The constructor binds the homomorphism parameters
     */
    _CheckCellWinner ( int p,int c)
  : cell(c),player(p)
    {
    }

    /**
     * Define the target variable to skip in the current route in apply
     */
    bool
        skip_variable(int vr) const
    {
      return vr!=cell;
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
        phi(int vr, int vl) const
    {
      if(vl==player)
      {
        return GHom (vr, vl, GHom::id ); // e-(joueur)-> ID
      }
      else
      {
        return GHom(DDD::null); // Cut the way (0)
      }
    }
    
    /**
     * Hash function used for unique table storage.
     */
    size_t
        hash() const
    {
      // hash function should exhibit reasonable spread and involve as many parameters as possible.
      std::size_t seed = 0;
      boost::hash_combine(seed, player);
      return seed ;
    }
  
    /**
     * Overloading StrongHom default print with a customized pretty-print
     */
    void
        print (std::ostream & os) const
    {
      os << "Note Winner(player:" << player << " )";
    }

    /**
     * Overload of operator== necessary for unique table storage
     * argument is typed StrongHom as == is part of StrongHom contract, simlarly to java's bool equals(Object)
     */
    bool
        operator==(const StrongHom &s) const
    {
      // direct "hard" cast to own type is ok, type checks already made in library
      const _CheckCellWinner& ps = dynamic_cast<const _CheckCellWinner&>(s);
      // basic comparator behavior, just make sure you put all attributes there.
      return player == ps.player && cell == ps.cell ;
    }
    
    /**
     * Clone current homomorphism, used for unique storage.
     */
    _GHom *
        clone () const
    {
      return new _CheckCellWinner(*this);
    }
};


/**
 * Factory of _TakeCellWithCheckWinner Instance
 */
GHom 
    CheckCellWinner (int player , int cell)
{
  return _CheckCellWinner(player,cell);
}














/**
 * An inductive homomorphism
 */
class _CheckCellNoWinner:public StrongHom
{
  private:
  /**
   * Liste of member variables = homomorphism parameters
   */
    int cell;     // The cell game
    int player;   // The player taking the cell : 0 or 1
  public:
    
    /**
   * The constructor binds the homomorphism parameters
     */
    _CheckCellNoWinner ( int p,int c)
  : cell(c),player(p)
    {
    }

    /**
     * Define the target variable to skip in the current route in apply
     */
    bool
        skip_variable(int vr) const
    {
      return vr!=cell;
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
        phi(int vr, int vl) const
    {
      if(vl==player || vl == EMPTY)
      {
        return GHom (vr, vl, GHom::id ); // e-(joueur)-> ID
      }
      else
      {
        return GHom(DDD::null); // Cut the way (0)
      }
    }
    
    /**
     * Hash function used for unique table storage.
     */
    size_t
        hash() const
    {
      // hash function should exhibit reasonable spread and involve as many parameters as possible.
      std::size_t seed = 0;
      boost::hash_combine(seed, player);
      return seed ;
    }
  
    /**
     * Overloading StrongHom default print with a customized pretty-print
     */
    void
        print (std::ostream & os) const
    {
      os << "Check configuration with no Winner(player:" << player << " )";
    }

    /**
     * Overload of operator== necessary for unique table storage
     * argument is typed StrongHom as == is part of StrongHom contract, simlarly to java's bool equals(Object)
     */
    bool
        operator==(const StrongHom &s) const
    {
      // direct "hard" cast to own type is ok, type checks already made in library
      const _CheckCellNoWinner& ps = dynamic_cast<const _CheckCellNoWinner&>(s);
      // basic comparator behavior, just make sure you put all attributes there.
      return player == ps.player && cell == ps.cell ;
    }
    
    /**
     * Clone current homomorphism, used for unique storage.
     */
    _GHom *
        clone () const
    {
      return new _CheckCellNoWinner(*this);
    }
};

/**
 * Factory of _TakeCellWithCheckWinner Instance
 */
GHom 
    CheckCellNoWinner (int player,int cell)
{
  return _CheckCellNoWinner(player,cell);
}










