#include "morpion_hom.hpp"

// SDD utilities to output stats and dot graphs
#include "util/dotExporter.h"
#include "statistic.hpp"


static bool beQuiet = false;
static string pathdotff = "final";
static bool dodotexport=false;
static std::string modelName = "";


static const int NBCASE = 9;




void usage() {
  cerr << "Morpion; package " << PACKAGE_STRING <<endl;
  cerr << "This tool performs state-space generation of extended timed Petri Nets allowing \n"
      << "inhibitor,pre,post (hyper)arcs. " <<endl
      << " The reachability set is computed using SDD/DDD, the Hierarchical Set Decision Diagram library, \n"
      << " Please see README file enclosed \n"
      << "in the distribution for more details. Input is a ROMEO xml model file, or one of the hard coded demo examples\n"
      << "(see Samples dir for documentation and examples). \n \nOptions :" << endl;
  cerr<<  "    -i path : specifies the path to input Romeo model " <<endl;
  cerr<<  "    -e exampleid : use a hard coded example. exampleid is an int, see HARDEXAMPLES.txt for details on this option.\n" ;
  cerr << "    --order : in conjunction with -e, use a custom order for some examples. \n" ;
  cerr << "    --dump-order : dump the currently used variable order to stdout and exit. \n" ;
  cerr<<  "    -d path : specifies the path prefix to construct dot state-space graph" <<endl;
  cerr<<  "    --sdd : privilege SDD storage." <<endl;
  cerr<<  "    --ddd : privilege DDD (no hierarchy) encoding." <<endl;
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



DDD createInitial () {
  
  GDDD M0 = GDDD::one;
  for (int i = 0; i < NBCASE; ++i) {
    // on ajoute une variable = case 
    // sa valeur = vide
    M0 = GDDD(i, EMPTY) ^  M0 ;
  }
  return M0;
}

int main (int argc, char **argv) {
  
  // Creation de l'état initial
  DDD initial = createInitial ();
  
  // Insertion des homomorphismes pour tirer le joueur 1
  std::set<GHom> nextAset;
  for (int i=0; i< NBCASE ; ++i) {
    nextAset.insert( take_cell (i, 0) );
  }
  // Creation d'un ensemble d'homomorphisme pour le joueur 1
  Hom nextA = GHom::add(nextAset);
  
 /* // moins efficace mais ok conceptuellement
   Hom nextA = GDDD::null;
  for (int i=0; i< NBCASE ; ++i) {
    nextA = nextA +  take_cell (i, 1) ;
  }
  */
  
  // Insertion des homomorphismes pour tirer le joueur 2
  std::set<GHom> nextBset;
  for (int i=0; i< NBCASE ; ++i) {
    nextBset.insert( take_cell (i, 1) );
  }
  // Creation d'un ensemble d'homomorphisme pour le joueur 2
  Hom nextB = GHom::add(nextBset);
      
  std::cout << "Initial state : " << initial << std::endl ;
  exportDot(GSDD(0,initial),"init");
  
  std::cout << "Player A move : " << nextA << std::endl ;
  
  DDD reachable = nextA (initial) ;
  std::cout << "Player A moves once : " << reachable << std::endl ;
  
   
  // Print some stats : memory size, number of states ....
  Statistic S = Statistic(reachable, "onemoveA" , CSV); // can also use LATEX instead of CSV
  S.print_table(std::cout);

  GHom fullT = fixpoint(  (nextB & nextA) + GHom::id );
  reachable = fullT (initial) ;
   // Export the SDD of final states to dot : generates files final.dot and d3final.dot
  exportDot(GSDD(0,reachable),"reach");
  
  
  Statistic S2 = Statistic(reachable, "reach" , CSV); // can also use LATEX instead of CSV
  S2.print_table(std::cout);
  
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