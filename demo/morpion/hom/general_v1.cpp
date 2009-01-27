// author : S. Hong,Y.Thierry-Mieg
// date : Jan 2009

#include "hom/general_v1.hpp"
#include <boost/functional/hash.hpp>




DDD
  createInitial2 () {
  
  GDDD M0 = GDDD::one;
  
  M0 = GDDD(0, EMPTY) ^  M0 ;
  DDD::varName(0,"gagnant");
  
  for (int i = 1; i < NBCASE+1; ++i) {
    // on ajoute une variable = case 
    // sa valeur = vide
    M0 = GDDD(i, EMPTY) ^  M0 ;
    std::stringstream cas;
    cas << "cell_" << i;
    DDD::varName(i,cas.str());
  }
  return M0;
}


Hom
    createPlayerHit( int player)
{
  // Insertion des homomorphismes pour tirer le joueur 1
  std::set<GHom> nextAAset;
  for (int i=1; i< NBCASE+1; ++i)
  {
    nextAAset.insert( TakeCellWithCheckWinner (i, player) );
  }
  // Creation d'un ensemble d'homomorphisme pour le joueur 1
  return GHom::add(nextAAset);
}

Hom
    createWinnerConfiguration( int player)
{
  std::set<GHom> noteWinnerBset;
  noteWinnerBset.insert( NoteWinner (1,2,3,3,player) & NoteWinner (4,5,6,3,player) & NoteWinner (7,8,9,3,player) & NoteWinner (1,4,7,3,player) & NoteWinner (2,5,8,3,player) & NoteWinner (3,6,9,3,player) & NoteWinner (1,5,9,3,player) & NoteWinner (3,5,7,3,player));

  return GHom::add(noteWinnerBset);
}








