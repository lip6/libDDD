#include "IntExpression.hpp"

#include <iostream>

using namespace std;

int main () {

  {
    IntExpression three = IntExpressionFactory::createConstant(3);
    
    cout << "constant 3: " << three << endl;
    IntExpressionFactory::printStats(cout);
    
    Variable *a = new Variable("a");
    IntExpression aexpr = IntExpressionFactory::createVariable(*a);
    
    IntExpression aplus3 = aexpr + three;
    
    cout << "a + 3 : " << aplus3 << endl;
    IntExpressionFactory::printStats(cout);
    
    aplus3 = aplus3 + three;
    cout << "a + 3 + 3: " << aplus3 << endl;
    IntExpressionFactory::printStats(cout);

    IntExpression aplus6 = aplus3.eval();
    cout << "(eval) a + 3 + 3: " << aplus6 << endl;

    cout << " In scope :" << endl;
    IntExpressionFactory::printStats(cout);
  }
  cout << " Out of scope :" << endl;
  IntExpressionFactory::printStats(cout);        

  return 0;
}
