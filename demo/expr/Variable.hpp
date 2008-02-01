#ifndef __VARIABLE_HH__
#define __VARIABLE_HH__

#include <string>


class Variable {
  std::string name;
public:
  Variable(const std::string & nname):name(nname){};
  const std::string & getName () const { return name; }
  bool operator== (const Variable & v) const {
    return v.name == name;
  }
  size_t hash () const { 
    return __gnu_cxx::hash<const char*>()(name.c_str());
  }


};



#endif
