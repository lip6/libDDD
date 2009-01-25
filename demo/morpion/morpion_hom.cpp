// author : S. Hong,Y.Thierry-Mieg
// date : Jan 2009

#include "morpion_hom.hpp"
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


// pretty print of a cell state
static const char * printCell (int cellState) {
  static const char * PA_str = "O";
  static const char * PB_str = "X";
  static const char * E_str =  ".";
  switch (cellState) {
  case 0 :
    return PA_str;
  case 1 :
    return PB_str;
  default :
    return E_str;
  }
}

// pretty print of a game state stored in an array
static void printState (array_type cc) {
  for (int lig = 0; lig < 3 ; ++lig) {
    for (int col = 0; col < 3 ; ++col) {
      std::cout << printCell(cc[lig][col]);
    }
    std::cout << std::endl;
  }
}




/**
 * An inductive homomorphism _CheckImpossible used only for check if there is some impossible configuration.
 * Validation Class
 */
class _checkImpossible:public StrongHom {

  /**
   * List of variable
   */
  array_type cc;
  int first_index;
  int node_to_ignore;
  public:
    
    /**
   * Constructor with initialisation
     */
    _checkImpossible (const array_type& c)
  : cc(c),first_index(1),node_to_ignore(0)
    {
    }

    /**
     * Constructor with initialisation
     */
    _checkImpossible (const array_type& c,int fi,int nti)
  : cc(c),first_index(fi),node_to_ignore(nti)
    {
    }

    /**
     * Skip all variable except Node 0
     */
    bool
        skip_variable(int vr) const
    {
      return (vr == node_to_ignore);
    }
    
    
  
    /**
     * PHI [1] : We have all state of the system and we can check if there are any impossible state
     */
    GDDD phiOne() const {

      bool cond = true;
      
      for(int i=0;i<LINE;++i)
      {
        for(int j=0;j<COLUMN;++j)
        {
          if(cc[i][j]==EMPTY)
          {
            cond =false;
          }
        }
      }
      
      if(cond)
      {
        /* Check if there is some winner on first line */
        std::cout << "Find one configuration completed :" << std::endl;
	printState(cc);
      }
      
      if((cc[0][0]==cc[0][1] && cc[0][0]==cc[0][2] && cc[0][0]!=EMPTY && cc[0][0]==cc[1][0] && cc[0][0]==cc[2][0])){
        /* Check if there is some winner on first line */
        std::cout << "One configuration Winner OK on line 0 :" << std::endl;
	printState(cc);
      }

      
      if((cc[0][0]==cc[1][1] && cc[0][0]==cc[2][2] && cc[0][0]!=EMPTY)){
        /* Check if there is some winner on first line */
        std::cout << "One configuration Winner OK on diagonnal :" << std::endl;
	printState(cc);
      }
      
      // Check the impossible line
      if((cc[0][0]==cc[0][1] && cc[0][0]==cc[0][2] && cc[0][0]!=EMPTY) &&
          ((  cc[1][0]==cc[1][1] && cc[1][0]==cc[1][2] && cc[1][0]!=EMPTY) || (cc[2][0]==cc[2][1] && cc[2][0]==cc[2][2] && cc[2][0]!=EMPTY)))
      {
        std::cout << "Impossible configuration detected 1 :" << std::endl;
	printState(cc);
	//return DDD::null; // Way forbidden, two different winner
      }
      if((cc[1][0]==cc[1][1] && cc[1][0]==cc[1][2] && cc[1][0]!=EMPTY) && ((cc[0][0]==cc[0][1] && cc[0][0]==cc[0][2] && cc[0][0]!=EMPTY) || (cc[2][0]==cc[2][1] && cc[2][0]==cc[2][2] && cc[2][0]!=EMPTY)))
      {
        std::cout << "Impossible configuration detected 2 :" << std::endl;
	printState(cc);
	//return DDD::null; // Way forbidden, two different winner
      }
      if((cc[2][0]==cc[2][1] && cc[2][0]==cc[2][2] && cc[2][0]!=EMPTY) && ((cc[0][0]==cc[0][1] && cc[0][0]==cc[0][2] && cc[0][0]!=EMPTY) || (cc[1][0]==cc[1][1] && cc[1][0]==cc[1][2] && cc[1][0]!=EMPTY)))
      {
        std::cout << "Impossible configuration detected 3 :" << std::endl;
	printState(cc);
	//return DDD::null; // Way forbidden, two different winner
      }
      
      // Check the impossible Column
      if((cc[0][0]==cc[1][0] && cc[0][0]==cc[2][0] && cc[0][0]!=EMPTY) && ((cc[0][1]==cc[1][1] && cc[0][1]==cc[2][1] && cc[0][1]!=EMPTY) || (cc[0][2]==cc[1][2] && cc[0][2]==cc[2][2] && cc[0][2]!=EMPTY)))
      {
        std::cout << "Impossible configuration detected 4 :" << std::endl;
	printState(cc);
	//return DDD::null; // Way forbidden, two different winner
      }
      if((cc[0][1]==cc[1][1] && cc[0][1]==cc[2][1] && cc[0][1]!=EMPTY) && ( (cc[0][0]==cc[1][0] && cc[0][0]==cc[2][0] && cc[0][0]!=EMPTY) || (cc[0][2]==cc[1][2] && cc[0][2]==cc[2][2] && cc[0][2]!=EMPTY)))
      {
        std::cout << "Impossible configuration detected 5 :" << std::endl;
	printState(cc);
	//return DDD::null; // Way forbidden, two different winner
      }
      if((cc[0][2]==cc[1][2] && cc[0][2]==cc[2][2] && cc[0][2]!=EMPTY) && ( (cc[0][0]==cc[1][0] && cc[0][0]==cc[2][0] && cc[0][0]!=EMPTY) || (cc[0][1]==cc[1][1] && cc[0][1]==cc[2][1] && cc[0][1]!=EMPTY)))
      {
        std::cout << "Impossible configuration detected 6 :" << std::endl;
	printState(cc);
	//return DDD::null; // Way forbidden, two different winner
      }

      return GDDD::one;
    }

    /**
     * PHI [vr,vl] : called on each arc labeled with "vl" when the homomorphism is applied to a node
     * of variable "vr" such that vr is NOT skipped.
     * When the hom is applied to a node this function is called for each arc, and the result
     * (a homomorphism) is applied to the successor node pointed by the arc.
     *  vr ==> Variable of current node
     *  vl ==> arc label (an integer) of current arc
     */
    GHom phi(int vr, int vl) const {
      
      /* Create new Homo with the current configuration */
      int i=(vr-first_index)/LINE; // Conversion sur la ligne
      int j=(vr-first_index)%COLUMN; // Conversion sur la colonne
	
	//std::cout << "Node [" << vr << "," << vl << "] Conversion on Grid [" << i << "," << j <<"] = " << vl << std::endl;
      if(cc[i][j]!=vl)
      {
        array_type tab(cc);
        tab[i][j] = static_cast<content_type>(vl);
        return GHom (vr,vl,_checkImpossible(tab));
      }
      else
      {
        return GHom (vr,vl,GHom(this));
      }
    }
    
    /**
     * Hash function used for unique table storage.
     */
    size_t hash() const {
      std::size_t seed = 3833;
      for(int i = 0; i< LINE ; ++i)
      {
        for(int j=0; j<COLUMN ; ++j)
        {
          boost::hash_combine(seed, cc[i][j]);
        }
      }
      
      return seed ;
    }
  
    /**
     * Overloading StrongHom default print with a customized pretty-print
     */
    void print (std::ostream & os) const {
      os << "Configuration Game : [";
      for(int i = 0; i< LINE ; ++i)
      {
        for(int j=0; j<COLUMN ; ++j)
        {
          os << " " << cc[i][j];
        }
        os << "\n";
      }
    }


    /**
     * Overload of operator== necessary for unique table storage
     * argument is typed StrongHom as == is part of StrongHom contract, simlarly to java's bool equals(Object)
     */
    bool operator==(const StrongHom &s) const {
      const _checkImpossible& ps = dynamic_cast<const _checkImpossible&>(s);
      for(int i = 0; i< LINE ; ++i)
      {
        for(int j=0; j<COLUMN ; ++j)
        {
          if(cc[i][j] != ps.cc[i][j])
          {
            return false;
          }
        }
      }
      return true;
    }
    
    /**
     * Clone current homomorphism, used for unique storage.
     */
    _GHom * clone () const {  return new _checkImpossible(*this); }
};


/**
 * Factory function of checkImpossible instance
 */
GHom checkImpossible (const array_type& t)
{
  return _checkImpossible(t);
}

GHom checkImpossible (const array_type& t, int fi,int nti)
{
  return _checkImpossible(t,fi,nti);
}








