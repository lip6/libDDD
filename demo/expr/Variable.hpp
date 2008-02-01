#ifndef __VARIABLE_HH__
#define __VARIABLE_HH__

#include <string>


class Variable {
  std::string name;
public:
  Variable(const std::string & nname):name(nname){};
  const std::string & getName () const { return name; }
};



#endif
