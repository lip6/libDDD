// author : S. Hong,Y.Thierry-Mieg
// date : Jan 2009
#include "morpion_hom.hpp"

// the cell value indicating it is empty
const int EMPTY = -1;

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
      return 10091*(cell+2)^player ;
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
  int nbCheck;        // Nombre de cellule vérifier
  int player;   // Le joueur qui veut jouer
  public:
    
    /**
   * Le constructeur avec initialisation de l'homomorphisme
     */
    _SelectWin ( int c1, int c2, int c3, int nb, int p) : case1(c1),case2(c2),case3(c3),nbCheck(nb), player(p) {}

    /**
     * Saturation sur la variable XX
     
    bool
	skip_variable(int vr) const
    {
      return vr != cell;
    }
    */
  
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
      if(nbCheck>0)
      {
	if(vr == case1 || vr == case2 || vr == case3)
	{
	  if(vl==player){
	    // une case de moins � tester
	    return GHom (vr, vl, SelectWin ( case1, case2, case3, nbCheck-1, player) ); // On se propage sans rien toucher
	  }
	  else{
	    // ID... donc on garde les chemins ou on a perdu ?
	    return GHom (vr, vl, GHom::id );     // e-(x)-> ID
	  }
	}
	else 
	{
	    // c'est le cas � capturer dans le skip_variable : on ignore la cellule
	  return GHom (vr, vl, GHom(this) ); // On se propage sans rien toucher
	}
      }
      else
      {
	// on coupe le chemin si on a gagn� ???
	return GHom(DDD::null); //  Couper le chenmin 0
      }
    }
     
    /**
     * Fonction de hash utilisé pour identifier l'unicité de l'homomorphisme. Trés utile pour le calcul de la table de cache
     */
    size_t hash() const {
      // il manque le nbcheck
      return 7817*(case1+2)^player + (case2+2)^case3;
    }
  
    /**
     * Surcharge de l'opérateur << de std::cout pour imprimer le type de la classe en cours selon un formatage personalisé
     */
    void print (std::ostream & os) const {
      os << "winnerCheckOn( case: [" << case1 << case2 << case3 << "]" << " for player:" << player << " )";
    }

    /**
     * Surcharge de l'opérateur == utilisé pour comparer 2 homomorphismes du même type, utilisé par la table de cache
     */
    bool operator==(const StrongHom &s) const {
      _SelectWin* ps = (_SelectWin*)&s;
      // OOOOPS il manque le nbcheck
      return case1 == ps->case1 && player == ps->player && case2 == ps->case2 && case3 == ps->case3 ;
    }
    
    /**
     * Clonage de l'objet en cours utilisée par la table de cache
     */
    _GHom * clone () const {  return new _SelectWin(*this); }
};


/**
* Fonction publique qui permet de créer des instances de l'homomorphisme décrit ci-dessus
*/
GHom SelectWin ( int c1, int c2, int c3,int nb, int player)  {
  return _SelectWin(c1,c2,c3,nb,player);
}

