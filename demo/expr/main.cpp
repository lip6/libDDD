#include "IntExpression.hpp"

#include <iostream>

using namespace std;

int main () {
  cout << "coucou" << endl;

  IntExpression three = IntExpressionFactory::createConstant(3);
  
  cout << "constant 3: " << three << endl;
  
  Variable *a = new Variable("a");
  IntExpression aexpr = IntExpressionFactory::createVariable(*a);
  
  IntExpression aplus3 = aexpr + three;
  
  cout << "a + 3 : " << aplus3 << endl;

  IntExpression aplus33 = aplus3 + three;
  cout << "a + 3 + 3: " << aplus33 << endl;

  IntExpression aplus6 = aplus33.eval();
  cout << "(eval) a + 3 + 3: " << aplus6 << endl;

  return 0;
}
