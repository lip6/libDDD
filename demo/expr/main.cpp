#include "IntExpression.hpp"
#include "BoolExpression.hpp"

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
    IntExpression bexpr = IntExpressionFactory::createVariable(varb);
    
    IntExpression aplus3 = aexpr + three;
    cout << "a + 3 : " << aplus3 << endl;
    IntExpressionFactory::printStats(cout);

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
    

    BoolExpression acompb = aexpr == bexpr;
    
    BoolExpression alessb = aexpr < bexpr;
    Assertion b = IntExpressionFactory::createAssertion (varb,four);
    cout << acompb << endl
	 << (acompb & a) << endl
	 << (acompb & a & b) << endl
	 << (acompb & a & b).eval() << endl
	 << endl ;
    

    cout << " In scope :" << endl;
    IntExpressionFactory::printStats(cout);
    BoolExpressionFactory::printStats(cout);
  }
  cout << " Out of scope :" << endl;
  IntExpressionFactory::printStats(cout);   
  BoolExpressionFactory::printStats(cout);     

  return 0;
}
