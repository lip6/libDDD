// author : S. Hong,Y.Thierry-Mieg // date : Jan 2009
#include <sstream>
#include <cstdlib>
#include "morpion_hom_v2.hpp"
#include "morpion_hom.hpp"

// SDD utilities to output stats and dot graphs
#include "util/dotExporter.h"
#include "statistic.hpp"

// option set to true limits verbosity
//static bool beQuiet = false;
// path prefix to output dot files
static string pathdotff = "final";
//static bool dodotexport=false;
// true if user asked for dot export
// static bool dodotexport=false;
// used as first column label of statistics
static std::string modelName = "";

// The number of cells in the game, i.e. 9


// the cell value indicating it is empty
const int EMPTY = -1;
const int NBCASE = 9;
const int LINE = 3;
const int COLUMN = 3;


void usage() {
  cerr << "Morpion V2; package " << PACKAGE_STRING <<endl;
  cerr << "This tool performs state-space analysis of the tic-tac-toe a.k.a. morpion game.\n"
      << " The reachability set is computed using SDD/DDD, the Hierarchical Set Decision Diagram library, \n"
      << " Please see README file enclosed \n"
      << "in the distribution for more details. \n \nOptions :" << endl;
  cerr<<  "    -d path : specifies the path prefix to construct dot state-space graph" <<endl;
  cerr<<  "    --texhead : print tex header"  << endl;
  cerr<<  "    --texline : print tex line " <<endl ;
  cerr<<  "    --textail : print tex tail"  << endl;
  cerr<<  "    --tex : print full tex description" <<endl;
  cerr<<  "    --quiet : limit output verbosity useful in conjunction with tex output --texline for batch performance runs" <<endl;
  cerr<<  "    --help,-h : display this (very helpful) helping help text"<<endl;
  cerr<<  "Problems ? Comments ? contact " << PACKAGE_BUGREPORT <<endl;
}

void bugreport () {
  cerr << "Timed Petri Net SDD/DDD Analyzer; package " << PACKAGE_STRING <<endl;
  cerr << "If you think your model is valid (i.e. it's a bug from us), please send your model files, specify the version you are running on, and we'll try to fix it." <<endl;
  cerr << "Bugreport contact : " << PACKAGE_BUGREPORT <<endl;
  cerr << "Sorry." << endl;
  exit(1);
}

/**
* Create the default configuration struture of DDD
*/
DDD createInitial () {
  
  GDDD M0 = GDDD::one;

  for (int i = 0; i < NBCASE; ++i) {
    M0 = GDDD(i, EMPTY) ^  M0 ;
    std::stringstream cas;
    cas << "cell_" << i;
    DDD::varName(i,cas.str());
  }
  
  M0 = GDDD(NBCASE, EMPTY) ^  M0 ;
  DDD::varName(NBCASE,"State");
  return M0;
}


int main (int /*argc*/, char ** /*argv*/) {
  
  // Creation de l'état initial
  DDD initial = createInitial ();
  
  // Insertion des homomorphismes pour tirer le joueur 1
  std::set<GHom> nextAAset;
  std::set<GHom> nextBBset;
  for (int i=0; i< NBCASE; ++i)
  {
    nextAAset.insert( Play (i, 0) );
    nextBBset.insert( Play (i, 1) );
  }
  // Creation d'un ensemble d'homomorphisme pour le joueur 1
  Hom nextAA = GHom::add(nextAAset);
  Hom nextBB = GHom::add(nextBBset);
  
  // Initialisation des combinaisons gagnantes
  GHom winnerA;
  GHom winnerB;
  
  winnerA = ( CheckCellWinner (0, 0) & CheckCellWinner (0, 1) & CheckCellWinner (0, 2) ) ;
  winnerA = winnerA + ( CheckCellWinner (0, 3) & CheckCellWinner (0, 4) & CheckCellWinner (0, 5) ) ;
  winnerA = winnerA + ( CheckCellWinner (0, 6) & CheckCellWinner (0, 7) & CheckCellWinner (0, 8) ) ;

  winnerA = winnerA + ( CheckCellWinner (0, 0) & CheckCellWinner (0, 3) & CheckCellWinner (0, 6) ) ;
  winnerA = winnerA + ( CheckCellWinner (0, 1) & CheckCellWinner (0, 4) & CheckCellWinner (0, 7) ) ;
  winnerA = winnerA + ( CheckCellWinner (0, 2) & CheckCellWinner (0, 5) & CheckCellWinner (0, 8) ) ;

  winnerA = winnerA + ( CheckCellWinner (0, 0) & CheckCellWinner (0, 4) & CheckCellWinner (0, 8) ) ;
  winnerA = winnerA + ( CheckCellWinner (0, 2) & CheckCellWinner (0, 4) & CheckCellWinner (0, 6) ) ;
  
  winnerB = ( CheckCellWinner (1, 0) & CheckCellWinner (1, 1) & CheckCellWinner (1, 2) ) ;
  winnerB = winnerB + ( CheckCellWinner (1, 3) & CheckCellWinner (1, 4) & CheckCellWinner (1, 5) ) ;
  winnerB = winnerB + ( CheckCellWinner (1, 6) & CheckCellWinner (1, 7) & CheckCellWinner (1, 8) ) ;
  
  winnerB = winnerB + ( CheckCellWinner (1, 0) & CheckCellWinner (1, 3) & CheckCellWinner (1, 6) ) ;
  winnerB = winnerB + ( CheckCellWinner (1, 1) & CheckCellWinner (1, 4) & CheckCellWinner (1, 7) ) ;
  winnerB = winnerB + ( CheckCellWinner (1, 2) & CheckCellWinner (1, 5) & CheckCellWinner (1, 8) ) ;
  
  winnerB = winnerB + ( CheckCellWinner (1, 0) & CheckCellWinner (1, 4) & CheckCellWinner (1, 8) ) ;
  winnerB = winnerB + ( CheckCellWinner (1, 2) & CheckCellWinner (1, 4) & CheckCellWinner (1, 6) ) ;
  
  // Initialisation des combinaisons non gagnantes
  GHom noWinner;
    for(int i=EMPTY;i<2;++i)
    {
      if(i == 0)
      {
        noWinner = ( CheckCellNoWinner (i, 0) + CheckCellNoWinner (i, 1) + CheckCellNoWinner (i, 2) ) ;
      }
      else
      {
        noWinner = noWinner & ( CheckCellNoWinner (i, 0) + CheckCellNoWinner (i, 1) + CheckCellNoWinner (i, 2) ) ;
      }
      noWinner = noWinner & ( CheckCellNoWinner (i, 3) + CheckCellNoWinner (i, 4) + CheckCellNoWinner (i, 5) ) ;
      noWinner = noWinner & ( CheckCellNoWinner (i, 6) + CheckCellNoWinner (i, 7) + CheckCellNoWinner (i, 8) ) ;
      
      noWinner = noWinner & ( CheckCellNoWinner (i, 0) + CheckCellNoWinner (i, 3) + CheckCellNoWinner (i, 6) ) ;
      noWinner = noWinner & ( CheckCellNoWinner (i, 1) + CheckCellNoWinner (i, 4) + CheckCellNoWinner (i, 7) ) ;
      noWinner = noWinner & ( CheckCellNoWinner (i, 2) + CheckCellNoWinner (i, 5) + CheckCellNoWinner (i, 8) ) ;
      
      noWinner = noWinner & ( CheckCellNoWinner (i, 0) + CheckCellNoWinner (i, 4) + CheckCellNoWinner (i, 8) ) ;
      noWinner = noWinner & ( CheckCellNoWinner (i, 2) + CheckCellNoWinner (i, 4) + CheckCellNoWinner (i, 6) ) ;
    }

    // Insertion des homomorphismes pour couper les chemins que l'on ne veut pas
    array_type tab(boost::extents[LINE][COLUMN]);
    for(int i = 0; i< LINE ; ++i)
    {
      for(int j=0; j<COLUMN ; ++j)
      {
        tab[i][j]=Vide;
      }
    }

  std::cout << "Make the fix point : \n\n\n" << std::endl ;
  /* ALGO :
  * 1) First we play BB, only if there is no winner in the current configuration
  * 2) Next we try to play AA (only if there is no winner in the current configuration), there is two possibility in union
  * 2.1) BB can play so it play (no winner and no full game)
  * 2.2) BB can't play because the game is full (it is possible), so nextAA cut this configuration but Full keep it
  * 3) Next we check if there is a winner for player A or B, if yes we note it at the end of the configuration game
  */
  /*
  GHom fullT2 = checkImpossible(tab,0,9) & fixpoint (
                          ( NoteWinner(0) & (winnerA & ( (Full(9) + nextAA)  & nextBB ) ) )
                           +
                          ( NoteWinner(1) & (winnerB & ( (Full(9) + nextAA)  & nextBB ) ) )
                           +
                          ( noWinner & ( (Full(9) + nextAA)  & nextBB ) )
                              
			  + GHom::id ) ;
  */
  
 
  
  GHom fullT2 = checkImpossible(tab,0,9) & fixpoint( ( ( (Full(9) + nextAA) & nextBB) ) + GHom::id);
  
  std::cout << "here" << std::endl;
  DDD reachable = fullT2 (initial);
  exportDot(GSDD(0,reachable),"reach2");
  Statistic S2 = Statistic(reachable, "reach2" , CSV); // can also use LATEX instead of CSV
  S2.print_table(std::cout);
  
  
  return EXIT_SUCCESS;
}




  /*
  for (int i=1;i < argc; i++) {
    if ( ! strcmp(argv[i],"-i") ) {
      if (++i > argc) 
{ cerr << "give argument value for romeo xml file name please after " << argv[i-1]<<endl; usage() ;exit(1);;}
      pathromeoff = argv[i];
      doromeoparse = true;
} else if (! strcmp(argv[i],"-d") ) {
      if (++i > argc) 
{ cerr << "give argument value for .dot file name please after " << argv[i-1]<<endl; usage() ; exit(1);;}
      pathdotff = argv[i];
      dodotexport = true;
} else if (! strcmp(argv[i],"-e") ) {
      if (++i > argc) 
{ cerr << "give integer argument value for example file name please after " << argv[i-1]<<endl; usage() ; exit(1);;}
      numexample = atoi(argv[i]);
      dosimpleexample = true;
} else if (! strcmp(argv[i],"--help") || ! strcmp(argv[i],"-h")  ) {
      usage(); exit(0);
} else if (! strcmp(argv[i],"--texhead")   ) {
      Statistic s = Statistic(SDD::one,"");
      s.print_header(cout);
      exit(0); 
} else if (! strcmp(argv[i],"--textail")   ) {
      Statistic s = Statistic(SDD::one,"");
      s.print_trailer(cout);
      exit(0);
} else if (! strcmp(argv[i],"--tex")   ) {
      doFullTex = true;
} else if (! strcmp(argv[i],"--texline")   ) {
      doTexLine = true;
} else if (! strcmp(argv[i],"--quiet")   ) {
      beQuiet = true;
} else if (! strcmp(argv[i],"--ddd")   ) {
      method = ddd_solution ;
      modelName = "d3:";
      model.setStorage(ITSModel::ddd_storage);
} else if (! strcmp(argv[i],"--order")   ) {
      usecustomorder = true;
} else if (! strcmp(argv[i],"--sdd")   ) {
      method = sdd_solution ;
      model.setStorage(ITSModel::sdd_storage);
}else if (! strcmp(argv[i],"--dump-order")   ) {
      dodumporder = true;
} else {
      cerr << "Error : incorrect Argument : "<<argv[i] <<endl ; usage(); exit(0);
}
}
  */
