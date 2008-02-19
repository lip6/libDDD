#include "MLHom.h"
#include "statistic.hpp"

#include "ExprHom.hpp"
#include <iostream>
#include <cassert>
using namespace std;


typedef enum {A, B, C, D,E, F, G} var;
var variables;
const char* vn[]= {"A", "B", "C", "D", "E", "F", "G"};

void initName() {
  for (int i=A; i<=G; i++) {
    DDD::varName(i,vn[i]);
    Context::setVariableIndex(i,Variable(vn[i]));
  }
}



class _AssertionHom;

int main () {
  initName();

  DDD test1 = GDDD(A,1,GDDD(B,2,GDDD(C,3,GDDD(D,4,GDDD(E,5,GDDD(F,6,GDDD(G,7)))))));
  DDD test2 = GDDD(A,2,GDDD(B,2,GDDD(C,3,GDDD(D,5,GDDD(E,5,GDDD(F,6,GDDD(G,7)))))));
  DDD test3 = GDDD(A,1,GDDD(B,2,GDDD(C,12,GDDD(D,4,GDDD(E,5,GDDD(F,6,GDDD(G,9)))))));
  DDD test4 = GDDD(A,1,GDDD(B,2,GDDD(C,12,GDDD(D,4,GDDD(E,5,GDDD(F,6,GDDD(G,7)))))));
  DDD test5 = test1 + test2 + test3 + test4;

  IntExpression Aexpr = IntExpressionFactory::createVariable(Context::getVariable(A));
  IntExpression Bexpr = IntExpressionFactory::createVariable(Context::getVariable(B));
  IntExpression Eexpr = IntExpressionFactory::createVariable(Context::getVariable(E));
  IntExpression Gexpr = IntExpressionFactory::createVariable(Context::getVariable(G));
  

  GHom be = assignExpr(B,Eexpr);
  GHom eb = assignExpr(E,Bexpr);
  GHom bg = assignExpr(B,Gexpr);

  IntExpression eplusg = Eexpr * Gexpr + (2 * Bexpr) - Aexpr;
  GHom beg = assignExpr(B,eplusg);  

  cout << "Input :\n" << test5 << endl;
  cout << "b:=" << Eexpr << " \n" << be(test5) << endl;
  cout << "e:=" << Bexpr << " \n" << eb(test5) << endl;
  cout << "b:=" << Gexpr << " \n" << bg(test5) << endl;
  cout << "b:=" << eplusg << " \n" <<  beg(test5) << endl;

  GHom gmax = predicate (Gexpr < 9);
  GHom incrG = assignExpr(G, Gexpr+1);

  cout <<  "g:=" <<  (Gexpr+1) << "["<<  (Gexpr < 9) << "]" << endl;
  cout << (incrG & gmax) (test5) << endl ;
  cout << "fixpoint :" << endl;
  cout << fixpoint((incrG & gmax) + GHom::id) (test5)<< endl;

  Statistic stat = Statistic(test5, "stats") ;
  stat.print_table (cout);

  return 0;
}
