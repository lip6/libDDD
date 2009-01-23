// author : S. Hong,Y.Thierry-Mieg
// date : Jan 2009
#include <sstream>
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
  cerr << "Morpion; package " << PACKAGE_STRING <<endl;
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


DDD createInitial2 () {
  
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

int main (int /*argc*/, char ** /*argv*/) {
  
  // Creation de l'état initial
  DDD initial2 = createInitial2 ();

  //std::cout<<"here"<< std::endl;
  
  // Insertion des homomorphismes pour tirer le joueur 1
  std::set<GHom> nextAAset;
  std::set<GHom> nextBBset;
  for (int i=1; i< NBCASE+1; ++i)
  {
    nextAAset.insert( TakeCellWithCheckWinner (i, 0) );
    nextBBset.insert( TakeCellWithCheckWinner (i, 1) );
  }
  // Creation d'un ensemble d'homomorphisme pour le joueur 1
  Hom nextAA = GHom::add(nextAAset);
  Hom nextBB = GHom::add(nextBBset);
  
  // Insertion des homomorphismes pour couper les chemins que l'on ne veut pas
  array_type tab(boost::extents[LINE][COLUMN]);
  for(int i = 0; i< LINE ; ++i)
  {
    for(int j=0; j<COLUMN ; ++j)
    {
      tab[i][j]=Vide;
    }
  }
  
  // Insertion des homomorphismes pour couper les chemins que l'on ne veut pas
  std::set<GHom> noteWinnerBset;
  noteWinnerBset.insert( NoteWinner (1,2,3,3,1) & NoteWinner (4,5,6,3,1) & NoteWinner (7,8,9,3,1) & NoteWinner (1,4,7,3,1) & NoteWinner (2,5,8,3,1) & NoteWinner (3,6,9,3,1) & NoteWinner (1,5,9,3,1) & NoteWinner (3,5,7,3,1));
  Hom noteWinnerB = GHom::add(noteWinnerBset);
  
  
  std::set<GHom> noteWinnerAset;
  noteWinnerAset.insert( NoteWinner (1,2,3,3,0) & NoteWinner (4,5,6,3,0) & NoteWinner (7,8,9,3,0) & NoteWinner (1,4,7,3,0) & NoteWinner (2,5,8,3,0) & NoteWinner (3,6,9,3,0) & NoteWinner (1,5,9,3,0) & NoteWinner (3,5,7,3,0));
  Hom noteWinnerA = GHom::add(noteWinnerAset);
  
  std::cout << "Make the fix point : \n\n\n" << std::endl ;
  /* ALGO :
   * 1) First we play BB, only if there is no winner in the current configuration
   * 2) Next we try to play AA (only if there is no winner in the current configuration), there is two possibility in union
   * 2.1) BB can play so it play (no winner and no full game)
   * 2.2) BB can't play because the game is full (it is possible), so nextAA cut this configuration but Full keep it
   * 3) Next we check if there is a winner for player A or B, if yes we note it at the end of the configuration game
   */
  GHom fullT2 = checkImpossible(tab) & fixpoint ( ( noteWinnerB & ( noteWinnerA & ( (Full() + nextAA)  & nextBB ) ) ) + GHom::id ) ;

  DDD reachable2 = fullT2 (initial2);
  exportDot(GSDD(0,reachable2),"reach2");
  Statistic S2 = Statistic(reachable2, "reach2" , CSV); // can also use LATEX instead of CSV
  S2.print_table(std::cout);
  
  
 Hom::id * DDD::one;  
  return 0;
  
 
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

