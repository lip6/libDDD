#include "IntExpression.hpp"

#include <iostream>

using namespace std;

int main () {

  {
    IntExpression three = IntExpressionFactory::createConstant(3);
    
    cout << "constant 3: " << three << endl;
    IntExpressionFactory::printStats(cout);
    
    Variable vara = Variable("a");
    Variable varb = Variable("b");
        
    IntExpression aexpr = IntExpressionFactory::createVariable(vara);
    
    IntExpression aplus3 = aexpr + three;
    
    cout << "a + 3 : " << aplus3 << endl;
    IntExpressionFactory::printStats(cout);

    IntExpression bexpr = IntExpressionFactory::createVariable(varb);
    IntExpression aplus3b = aplus3 + bexpr;
    cout << "a + 3 + b : " << aplus3b << endl;

    aplus3 = aplus3 + three;
    cout << "a + 3 + 3: " << aplus3 << endl;
    IntExpressionFactory::printStats(cout);

    IntExpression aplus6 = aplus3.eval();
    cout << "(eval) a + 3 + 3: " << aplus6 << endl;

    IntExpression four = IntExpressionFactory::createConstant(4);

    Assertion a = IntExpressionFactory::createAssertion (vara,four);

    IntExpression fourplus6 = aplus3 & a ;
    cout << "a + 3 + 3 & a:4 " << fourplus6 << endl;
    cout << "(eval) : " << fourplus6.eval() << endl;

    cout << " In scope :" << endl;
    IntExpressionFactory::printStats(cout);
  }
  cout << " Out of scope :" << endl;
  IntExpressionFactory::printStats(cout);        

  return 0;
}
