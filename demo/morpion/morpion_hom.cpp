

#include "morpion_hom.hpp"

const int EMPTY = -1;

/**
* Modélisation d'un homomorphisme
*/
class _TakeCell:public StrongHom {

  /**
  * Liste des variables d'entrées (domaine d'entrée) de l'homomorphisme
  */
  int cell;     // La cellule à choisir
  int player;   // Le joueur qui veut jouer
  public:
    
    /**
    * Le constructeur avec initialisation de l'homomorphisme
    */
    _TakeCell ( int c, int p) : cell(c), player(p) {}

    /**
    * Saturation sur la variable XX
    */
    bool
	skip_variable(int vr) const
    {
      return vr != cell;
    }
  
    /**
    * PHI [1] : Si on rencontre la fin du DDD
    */
    GDDD phiOne() const {
      return DDD::top;
    }

    /**
    * PHI [vr,vl] : représente le noeud courant auquel on applique l'homomorphisme
    *  vr ==> Variable ou Noeud courant
    *  vl ==> Valeur de l'arc du successeur pour le Noeud courant
    */
    GHom phi(int vr, int vl) const {
      if (vl == EMPTY) 
	return GHom (vr, player, GHom::id );// e-(joueur)-> ID
      else
	return GHom(DDD::null); //  Couper le chenmin 0
    }
     
    /**
    * Fonction de hash utilisé pour identifier l'unicité de l'homomorphisme. Trés utile pour le calcul de la table de cache
    */
    size_t hash() const {
      return 10091*(cell+2)^player ;
    }
  
    /**
    * Surcharge de l'opérateur << de std::cout pour imprimer le type de la classe en cours selon un formatage personalisé
    */
    void print (std::ostream & os) const {
      os << "takeCell( cell:" << cell << ", player:" << player << " )";
    }

    /**
    * Surcharge de l'opérateur == utilisé pour comparer 2 homomorphismes du même type, utilisé par la table de cache
    */
    bool operator==(const StrongHom &s) const {
      _TakeCell* ps = (_TakeCell*)&s;
      return cell == ps->cell && player == ps->player ;
    }
    
    /**
    * Clonage de l'objet en cours utilisée par la table de cache
    */
    _GHom * clone () const {  return new _TakeCell(*this); }
};


/**
 * Fonction publique qui permet de créer des instances de l'homomorphisme décrit ci-dessus
 */
GHom take_cell ( int c1, int player)  {
  return _TakeCell(c1,player);
}


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
	    return GHom (vr, vl, SelectWin ( case1, case2, case3, nbCheck-1, player) ); // On se propage sans rien toucher
	  }
	  else{
	    return GHom (vr, vl, GHom::id );     // e-(x)-> ID
	  }
	}
	else 
	{
	  return GHom (vr, vl, GHom(this) ); // On se propage sans rien toucher
	}
      }
      else
      {
	return GHom(DDD::null); //  Couper le chenmin 0
      }
    }
     
    /**
     * Fonction de hash utilisé pour identifier l'unicité de l'homomorphisme. Trés utile pour le calcul de la table de cache
     */
    size_t hash() const {
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

