#include "IntExpression.hpp"

#include <iostream>

using namespace std;

int main () {
  cout << "coucou" << endl;

  const IntExpression & three = *IntExpressionFactory::createConstant(3);
  
  cout << "constant 3: " << three << endl;
  
  Variable *a = new Variable("a");
  const IntExpression & aexpr = *IntExpressionFactory::createVariable(*a);
  
  const IntExpression & aplus3 = aexpr + three;
  
  cout << "a + 3 : " << aplus3 << endl;

  const IntExpression & aplus33 = aplus3 + three;
  cout << "a + 3 + 3: " << aplus33 << endl;

  const IntExpression & aplus6 = aplus33.eval();
  cout << "(eval) a + 3 + 3: " << aplus6 << endl;

  return 0;
}
