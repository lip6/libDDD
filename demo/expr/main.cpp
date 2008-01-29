#include "IntExpression.hpp"

#include <iostream>

using namespace std;

int main () {
  cout << "coucou" << endl;

  const IntExpression & three = *IntExpressionFactory::createConstant(3);
  
  cout << "a constant : 3 " << three << endl;
  
  Variable *a = new Variable("a");
  const IntExpression & aexpr = *IntExpressionFactory::createVariable(*a);
  
  const IntExpression & aplus3 = aexpr + three;
  
  cout << "a + 3 : " << aplus3 << endl;

  return 0;
}
