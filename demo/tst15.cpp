#include "SwapMLHom.hh"

int main() {
  DDD a(0,0,GDDD(1,1,GDDD(2,3)));
  DDD b(0,1,GDDD(1,2,GDDD(2,3)));
  DDD c = a + b;
  
  Hom swap1 = Swap(0,1);
  Hom swap2 = Swap(0,2);
  Hom swap3 = Swap(2,0);
  Hom swap4 = Swap(2,1);
  
  std::cout << "c: " << std::endl;
  std::cout << c << std::endl;
  
  std::cout << "swap vars 0 and 1 in c: " << std::endl;
  std::cout << swap1(c) << std::endl;
  
  std::cout << "swap vars 0 and 2 in c: " << std::endl;
  std::cout << swap2(c) << std::endl;
  
  std::cout << "swap vars 2 and 0 in c (should be the same as the previous one): " << std::endl;
  std::cout << swap3(c) << std::endl;
  
  std::cout << "swap vars 2 and 1 in c: " << std::endl;
  std::cout << swap4(c) << std::endl;
  
  return 0;
}
