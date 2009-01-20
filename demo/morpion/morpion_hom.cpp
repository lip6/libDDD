// author : S. Hong,Y.Thierry-Mieg
// date : Jan 2009

#include "morpion_hom.hpp"
#include <boost/functional/hash.hpp>

// the cell value indicating it is empty
const int EMPTY = -1;
const int NBCASE = 9;
const int LINE = 3;
const int COLUMN = 3;

/**
* An inductive homomorphism TakeCell to play a move of a given player in a given cell.
*/
class _TakeCell:public StrongHom {

  /**
  * Liste of member variables = homomorphism parameters
  */
  int cell;     // The cell to play in� : 0 <= cell < 9
  int player;   // The player taking the cell : 0 or 1
  public:
    
    /**
    * The constructor binds the homomorphism parameters cell and player
    */
    _TakeCell ( int c, int p) : cell(c), player(p) {}

    /**
    * Define the target variable = cell. It is the only variable that is not skipped (=ignored).
    * This means the behavior of Phi(vr,vl) for any node of variable "vr" starts with :
    * if ( skip_variable(vr)==true )
    *    return GHom (vr, vl, GHom(this) ); // Self propagation without any modification of node or self.
    */
    bool
	skip_variable(int vr) const
    {
      return vr != cell;
    }
  
    /**
    * PHI [1] : called if the terminal 1 is encountered, returns a constant DDD.
    * If the cell was correct 0 <= cell < 9 and the state ok this should not happen :
    * return TOP to indicate an error.
    */
    GDDD phiOne() const {
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
    GHom phi(int vr, int vl) const {
      if (vl == EMPTY) 
	// the cell is empty ! We can play here and set the cell value to "player".
	return GHom (vr, player, GHom::id );// e-(joueur)-> ID
      else
	// the cell is not empty, move is illegal, abort this move.
	return GHom(DDD::null); //  Couper le chenmin 0
    }
     
    /**
    * Hash function used for unique table storage.
    */
    size_t hash() const {
      // hash function should exhibit reasonable spread and involve as many parameters as possible.
      std::size_t seed = 0;
      boost::hash_combine(seed, cell);
      boost::hash_combine(seed, player);
      return seed ;
    }
  
    /**
    * Overloading StrongHom default print with a customized pretty-print
    */
    void print (std::ostream & os) const {
      os << "takeCell( cell:" << cell << ", player:" << player << " )";
    }

    /**
    * Overload of operator== necessary for unique table storage
    * argument is typed StrongHom as == is part of StrongHom contract, simlarly to java's bool equals(Object) 
   */
    bool operator==(const StrongHom &s) const {
       // direct "hard" cast to own type is ok, type checks already made in library
      _TakeCell* ps = (_TakeCell*)&s;
      // basic comparator behavior, just make sure you put all attributes there.
      return cell == ps->cell && player == ps->player ;
    }
    
    /**
    * Clone current homomorphism, used for unique storage.
    */
    _GHom * clone () const {  return new _TakeCell(*this); }
};


/**
 * Fonction publique qui permet de créer des instances de l'homomorphisme décrit ci-dessus
 */
GHom take_cell ( int c1, int player)  {
  return _TakeCell(c1,player);
}



/** Ok donc l� la grosse question c'est que doit faire cet homomorphisme ?
 * A priori il doit ne selectionner que les chemins ou la combinaison choisie gagne ; les autres on rends NULL � un moment pour couper la branche. 
 */
/**
 * Modélisation d'un homomorphisme
 */
class _SelectWin:public StrongHom {

  /**
   * Liste des variables d'entrées (domaine d'entrée) de l'homomorphisme
   */
  int case1;     // La cellule à vérifier
  int case2;     // La cellule à vérifier
  int case3;     // La cellule à vérifier
  int nbCheck;   // Nombre de cellule vérifier
  int player;    // Le joueur à vérifier
  public:
    
    /**
   * Le constructeur avec initialisation de l'homomorphisme
     */
    _SelectWin ( int c1, int c2, int c3, int nb, int p) : case1(c1),case2(c2),case3(c3),nbCheck(nb), player(p) {}

    /**
     * Saturation sur la variable XX
    */ 
    bool
	skip_variable(int vr) const
    {
      return (vr != case1) && (vr != case2) && (vr != case3);
    }
    
  
    /**
     * PHI [1] : Si on rencontre la fin du DDD
     */
    GDDD phiOne() const {
      // Ce cas peut-il se produire ? 
      // si oui est-ce une erreur (T) ou une partie gagnante (1) ou une partie perdue (0)?
      return GDDD::one;
    }

    /**
     * PHI [vr,vl] : représente le noeud courant auquel on applique l'homomorphisme
     *  vr ==> Variable ou Noeud courant
     *  vl ==> Valeur de l'arc du successeur pour le Noeud courant
     */
    GHom phi(int vr, int vl) const {
	if(vl== player)
	{
          if(nbCheck-1>0)
	  {
	      return GHom (vr, vl, SelectWin ( case1, case2, case3, nbCheck-1, player) ); // Propagation n-1
	  }
	  else
	    {
	      //return GHom(DDD::null); //  Couper le chenmin 0
	      std::cout << "Working on node " << vr << " : One line Winner found " << nbCheck << std::endl;
	      //return GHom (vr, vl, SelectWin ( case1, case2, case3, nbCheck-1, player,true) );
	      return GHom(DDD::null); //  Couper le chenmin 1 : plus d'évolution possible
	    }
	}
	else
	{
	  std::cout << "Working on node " << vr << " : No way winner : ID " << nbCheck << std::endl;
	  //return GHom (vr, vl, GHom::id ); // e-(x)-> ID
	  //return GHom (vr, vl, GHom::id );
	  return GHom (vr, vl, GHom::id );
	}
    }
     
    /**
     * Fonction de hash utilisé pour identifier l'unicité de l'homomorphisme. Trés utile pour le calcul de la table de cache
     */
    size_t hash() const 
    {
      std::size_t seed = 0;
      boost::hash_combine(seed, case1);
      boost::hash_combine(seed, case2);
      boost::hash_combine(seed, case3);
      boost::hash_combine(seed, player);
      return seed ;
    }
  
    /**
     * Surcharge de l'opérateur << de std::cout pour imprimer le type de la classe en cours selon un formatage personalisé
     */
    void print (std::ostream & os) const
    {
      os << "winnerCheckOn( case: [" << case1 << case2 << case3 << "]" << " for player:" << player << " )";
    }

    /**
     * Surcharge de l'opérateur == utilisé pour comparer 2 homomorphismes du même type, utilisé par la table de cache
     */
    bool operator==(const StrongHom &s) const
    {
      const _SelectWin& ps = dynamic_cast<const _SelectWin&>(s);
      return case1 == ps.case1 && player == ps.player && case2 == ps.case2 && case3 == ps.case3 && nbCheck == ps.nbCheck;
    }
    
    /**
     * Clonage de l'objet en cours utilisée par la table de cache
     */
    _GHom * clone () const
    {
      return new _SelectWin(*this);
    }
};


/**
* Fonction publique qui permet de créer des instances de l'homomorphisme décrit ci-dessus
*/
GHom SelectWin ( int c1, int c2, int c3,int nb, int player)  {
  return _SelectWin(c1,c2,c3,nb,player);
}













/**
 * Modélisation d'un homomorphisme
 */
class _checkWinner:public StrongHom {

  /**
   * Liste des variables d'entrées (domaine d'entrée) de l'homomorphisme
   */
  
  array_type cc;
  public:
    
    /**
   * Le constructeur avec initialisation de l'homomorphisme
     */
    _checkWinner (const array_type& c)
   : cc(c)
    {
    }

    /**
     * Saturation sur la variable XX
     
    bool
	skip_variable(int vr) const
    {
      return (vr != 9);
    }
     */ 
    
  
    /**
     * PHI [1] : Si on rencontre la fin du DDD
     */
    GDDD phiOne() const {
      
      /* Check if there more that one winner in the configuration */
      
      // Check the impossible line
      if((cc[0][0]==cc[0][1] && cc[0][0]==cc[0][2] && cc[0][0]!=EMPTY) &&
	  ((  cc[1][0]==cc[1][1] && cc[1][0]==cc[1][2] && cc[1][0]!=EMPTY) || (cc[2][0]==cc[2][1] && cc[2][0]==cc[2][2] && cc[2][0]!=EMPTY)))
      {
	std::cout << "Cutting one configuration :" << std::endl;
	std::cout << ((cc[0][0]==EMPTY) ? 9:cc[0][0]) << ((cc[0][1]==EMPTY) ? 9:cc[0][1]) << ((cc[0][2]==EMPTY) ? 9:cc[0][2]) << std::endl;
	std::cout << ((cc[1][0]==EMPTY)  ? 9:cc[1][0]) << ((cc[1][1]==EMPTY) ?   9:cc[1][1]) << (cc[1][2]==EMPTY ?     9:cc[1][2]) << std::endl;
	std::cout << ((cc[2][0]==EMPTY) ? 9:cc[2][0]) << ((cc[2][1]==EMPTY) ? 9:cc[2][1]) << ((cc[2][2]==EMPTY) ? 9:cc[2][2]) << std::endl;
	return DDD::null; // Way forbidden, two different winner
      }
      if((cc[1][0]==cc[1][1] && cc[1][0]==cc[1][2] && cc[1][0]!=EMPTY) && ((cc[0][0]==cc[0][1] && cc[0][0]==cc[0][2] && cc[0][0]!=EMPTY) || (cc[2][0]==cc[2][1] && cc[2][0]==cc[2][2] && cc[2][0]!=EMPTY)))
      {
	std::cout << "Cutting one configuration :" << std::endl;
	std::cout << ((cc[0][0]==EMPTY) ? 9:cc[0][0]) << ((cc[0][1]==EMPTY) ? 9:cc[0][1]) << ((cc[0][2]==EMPTY) ? 9:cc[0][2]) << std::endl;
	std::cout << ((cc[1][0]==EMPTY)  ? 9:cc[1][0]) << ((cc[1][1]==EMPTY) ?   9:cc[1][1]) << (cc[1][2]==EMPTY ?     9:cc[1][2]) << std::endl;
	std::cout << ((cc[2][0]==EMPTY) ? 9:cc[2][0]) << ((cc[2][1]==EMPTY) ? 9:cc[2][1]) << ((cc[2][2]==EMPTY) ? 9:cc[2][2]) << std::endl;
	return DDD::null; // Way forbidden, two different winner
      }
      if((cc[2][0]==cc[2][1] && cc[2][0]==cc[2][2] && cc[2][0]!=EMPTY) && ((cc[0][0]==cc[0][1] && cc[0][0]==cc[0][2] && cc[0][0]!=EMPTY) || (cc[1][0]==cc[1][1] && cc[1][0]==cc[1][2] && cc[1][0]!=EMPTY)))
      {
	std::cout << "Cutting one configuration :" << std::endl;
	std::cout << ((cc[0][0]==EMPTY) ? 9:cc[0][0]) << ((cc[0][1]==EMPTY) ? 9:cc[0][1]) << ((cc[0][2]==EMPTY) ? 9:cc[0][2]) << std::endl;
	std::cout << ((cc[1][0]==EMPTY)  ? 9:cc[1][0]) << ((cc[1][1]==EMPTY) ?   9:cc[1][1]) << (cc[1][2]==EMPTY ?     9:cc[1][2]) << std::endl;
	std::cout << ((cc[2][0]==EMPTY) ? 9:cc[2][0]) << ((cc[2][1]==EMPTY) ? 9:cc[2][1]) << ((cc[2][2]==EMPTY) ? 9:cc[2][2]) << std::endl;
	return DDD::null; // Way forbidden, two different winner
      }
      
      // Check the impossible Column
      if((cc[0][0]==cc[1][0] && cc[0][0]==cc[2][0] && cc[0][0]!=EMPTY) && ((cc[0][1]==cc[1][1] && cc[0][1]==cc[2][1] && cc[0][1]!=EMPTY) || (cc[0][2]==cc[1][2] && cc[0][2]==cc[2][2] && cc[0][2]!=EMPTY)))
      {
	std::cout << "Cutting one configuration :" << std::endl;
	std::cout << ((cc[0][0]==EMPTY) ? 9:cc[0][0]) << ((cc[0][1]==EMPTY) ? 9:cc[0][1]) << ((cc[0][2]==EMPTY) ? 9:cc[0][2]) << std::endl;
	std::cout << ((cc[1][0]==EMPTY)  ? 9:cc[1][0]) << ((cc[1][1]==EMPTY) ?   9:cc[1][1]) << (cc[1][2]==EMPTY ?     9:cc[1][2]) << std::endl;
	std::cout << ((cc[2][0]==EMPTY) ? 9:cc[2][0]) << ((cc[2][1]==EMPTY) ? 9:cc[2][1]) << ((cc[2][2]==EMPTY) ? 9:cc[2][2]) << std::endl;
	return DDD::null; // Way forbidden, two different winner
      }
      if((cc[0][1]==cc[1][1] && cc[0][1]==cc[2][1] && cc[0][1]!=EMPTY) && ( (cc[0][0]==cc[1][0] && cc[0][0]==cc[2][0] && cc[0][0]!=EMPTY) || (cc[0][2]==cc[1][2] && cc[0][2]==cc[2][2] && cc[0][2]!=EMPTY)))
      {
	std::cout << "Cutting one configuration :" << std::endl;
	std::cout << ((cc[0][0]==EMPTY) ? 9:cc[0][0]) << ((cc[0][1]==EMPTY) ? 9:cc[0][1]) << ((cc[0][2]==EMPTY) ? 9:cc[0][2]) << std::endl;
	std::cout << ((cc[1][0]==EMPTY)  ? 9:cc[1][0]) << ((cc[1][1]==EMPTY) ?   9:cc[1][1]) << (cc[1][2]==EMPTY ?     9:cc[1][2]) << std::endl;
	std::cout << ((cc[2][0]==EMPTY) ? 9:cc[2][0]) << ((cc[2][1]==EMPTY) ? 9:cc[2][1]) << ((cc[2][2]==EMPTY) ? 9:cc[2][2]) << std::endl;
	return DDD::null; // Way forbidden, two different winner
      }
      if((cc[0][2]==cc[1][2] && cc[0][2]==cc[2][2] && cc[0][2]!=EMPTY) && ( (cc[0][0]==cc[1][0] && cc[0][0]==cc[2][0] && cc[0][0]!=EMPTY) || (cc[0][1]==cc[1][1] && cc[0][1]==cc[2][1] && cc[0][1]!=EMPTY)))
      {
	std::cout << "Cutting one configuration :" << std::endl;
	std::cout << ((cc[0][0]==EMPTY) ? 9:cc[0][0]) << ((cc[0][1]==EMPTY) ? 9:cc[0][1]) << ((cc[0][2]==EMPTY) ? 9:cc[0][2]) << std::endl;
	std::cout << ((cc[1][0]==EMPTY)  ? 9:cc[1][0]) << ((cc[1][1]==EMPTY) ?   9:cc[1][1]) << (cc[1][2]==EMPTY ?     9:cc[1][2]) << std::endl;
	std::cout << ((cc[2][0]==EMPTY) ? 9:cc[2][0]) << ((cc[2][1]==EMPTY) ? 9:cc[2][1]) << ((cc[2][2]==EMPTY) ? 9:cc[2][2]) << std::endl;
	return DDD::null; // Way forbidden, two different winner
      }
      
      // Check the double winner on the same player, for exemple this way is impossible
      // XXX
      // OOX
      //  OX
      if((cc[0][0]==cc[0][1] && cc[0][0]==cc[0][2] && cc[0][0]!=EMPTY) && ((cc[0][0]==cc[1][0] && cc[0][0]==cc[2][0] && cc[0][0]!=EMPTY) || (cc[0][2]==cc[1][2] && cc[0][2]==cc[2][2] && cc[0][2]!=EMPTY) || (cc[0][1]==cc[1][1] && cc[0][1]==cc[2][1] && cc[0][1]!=EMPTY)))
      {
	std::cout << "Cutting one configuration :" << std::endl;
	std::cout << ((cc[0][0]==EMPTY) ? 9:cc[0][0]) << ((cc[0][1]==EMPTY) ? 9:cc[0][1]) << ((cc[0][2]==EMPTY) ? 9:cc[0][2]) << std::endl;
	std::cout << ((cc[1][0]==EMPTY)  ? 9:cc[1][0]) << ((cc[1][1]==EMPTY) ?   9:cc[1][1]) << (cc[1][2]==EMPTY ?     9:cc[1][2]) << std::endl;
	std::cout << ((cc[2][0]==EMPTY) ? 9:cc[2][0]) << ((cc[2][1]==EMPTY) ? 9:cc[2][1]) << ((cc[2][2]==EMPTY) ? 9:cc[2][2]) << std::endl;
	return DDD::null; // Way forbidden
      }
      if((cc[2][0]==cc[2][1] && cc[2][0]==cc[2][2] && cc[2][0]!=EMPTY) && ((cc[0][0]==cc[1][0] && cc[0][0]==cc[2][0] && cc[0][0]!=EMPTY) || (cc[0][2]==cc[1][2] && cc[0][2]==cc[2][2] && cc[0][2]!=EMPTY) || (cc[0][1]==cc[1][1] && cc[0][1]==cc[2][1] && cc[0][1]!=EMPTY)))
      {
	std::cout << "Cutting one configuration :" << std::endl;
	std::cout << ((cc[0][0]==EMPTY) ? 9:cc[0][0]) << ((cc[0][1]==EMPTY) ? 9:cc[0][1]) << ((cc[0][2]==EMPTY) ? 9:cc[0][2]) << std::endl;
	std::cout << ((cc[1][0]==EMPTY)  ? 9:cc[1][0]) << ((cc[1][1]==EMPTY) ?   9:cc[1][1]) << (cc[1][2]==EMPTY ?     9:cc[1][2]) << std::endl;
	std::cout << ((cc[2][0]==EMPTY) ? 9:cc[2][0]) << ((cc[2][1]==EMPTY) ? 9:cc[2][1]) << ((cc[2][2]==EMPTY) ? 9:cc[2][2]) << std::endl;
	return DDD::null; // Way forbidden
      }
      
      if((cc[0][0]==cc[1][0] && cc[0][0]==cc[2][0] && cc[0][0]!=EMPTY) && ((cc[0][0]==cc[0][1] && cc[0][0]==cc[0][2] && cc[0][0]!=EMPTY) || (cc[1][0]==cc[1][1] && cc[1][0]==cc[1][2] && cc[1][0]!=EMPTY) || (cc[2][0]==cc[2][1] && cc[2][0]==cc[2][2] && cc[2][0]!=EMPTY)))
      {
	std::cout << "Cutting one configuration :" << std::endl;
	std::cout << ((cc[0][0]==EMPTY) ? 9:cc[0][0]) << ((cc[0][1]==EMPTY) ? 9:cc[0][1]) << ((cc[0][2]==EMPTY) ? 9:cc[0][2]) << std::endl;
	std::cout << ((cc[1][0]==EMPTY)  ? 9:cc[1][0]) << ((cc[1][1]==EMPTY) ?   9:cc[1][1]) << (cc[1][2]==EMPTY ?     9:cc[1][2]) << std::endl;
	std::cout << ((cc[2][0]==EMPTY) ? 9:cc[2][0]) << ((cc[2][1]==EMPTY) ? 9:cc[2][1]) << ((cc[2][2]==EMPTY) ? 9:cc[2][2]) << std::endl;
	return DDD::null; // Way forbidden
      }
      if((cc[0][2]==cc[1][2] && cc[0][2]==cc[2][2] && cc[0][2]!=EMPTY) && ((cc[0][0]==cc[0][1] && cc[0][0]==cc[0][2] && cc[0][0]!=EMPTY) || (cc[1][0]==cc[1][1] && cc[1][0]==cc[1][2] && cc[1][0]!=EMPTY) || (cc[2][0]==cc[2][1] && cc[2][0]==cc[2][2] && cc[2][0]!=EMPTY)))
      {
	std::cout << "Cutting one configuration :" << std::endl;
	std::cout << ((cc[0][0]==EMPTY) ? 9:cc[0][0]) << ((cc[0][1]==EMPTY) ? 9:cc[0][1]) << ((cc[0][2]==EMPTY) ? 9:cc[0][2]) << std::endl;
	std::cout << ((cc[1][0]==EMPTY)  ? 9:cc[1][0]) << ((cc[1][1]==EMPTY) ?   9:cc[1][1]) << (cc[1][2]==EMPTY ?     9:cc[1][2]) << std::endl;
	std::cout << ((cc[2][0]==EMPTY) ? 9:cc[2][0]) << ((cc[2][1]==EMPTY) ? 9:cc[2][1]) << ((cc[2][2]==EMPTY) ? 9:cc[2][2]) << std::endl;
	return DDD::null; // Way forbidden
      }
      
      
      
      //std::cout<< "POSSIBLE WAY" << " Configuration Game : [" << cc1 << "," << cc2 << "," << cc[0][2] << "," << cc[1][0] << "," << ccc[1][1] << "," << cc6 << "," << cc7 << "," << cc8 << "," << cc9 << "]" << std::endl;
      return DDD::one;
    }

    /**
     * PHI [vr,vl] : représente le noeud courant auquel on applique l'homomorphisme
     *  vr ==> Variable ou Noeud courant
     *  vl ==> Valeur de l'arc du successeur pour le Noeud courant
     */
    GHom phi(int vr, int vl) const {
        /* Create new Homo with the current configuration */
      int i=vr/LINE; // Conversion sur la ligne
      int j=vr%COLUMN; // Conversion sur la colonne
      
      if(cc[i][j]!=vl)
      {
	array_type tab(cc);
        tab[i][j] = static_cast<content_type>(vl);
	return GHom (vr,vl,_checkWinner(tab));
      }
      else
      {
	return GHom (vr,vl,GHom(this));
      }
      
    }
     
    /**
     * Fonction de hash utilisé pour identifier l'unicité de l'homomorphisme. Trés utile pour le calcul de la table de cache
     */
    size_t hash() const {
      std::size_t seed = 0;
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
     * Surcharge de l'opérateur << de std::cout pour imprimer le type de la classe en cours selon un formatage personalisé
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
     * Surcharge de l'opérateur == utilisé pour comparer 2 homomorphismes du même type, utilisé par la table de cache
     */
    bool operator==(const StrongHom &s) const {
      const _checkWinner& ps = dynamic_cast<const _checkWinner&>(s);
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
     * Clonage de l'objet en cours utilisée par la table de cache
     */
    _GHom * clone () const {  return new _checkWinner(*this); }
};


/**
 * Fonction publique qui permet de créer des instances de l'homomorphisme décrit ci-dessus
 */
GHom checkWinner (const array_type& t)
{
  return _checkWinner(t);
}




