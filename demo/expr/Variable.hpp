#ifndef __VARIABLE_HH__
#define __VARIABLE_HH__

#include <string>

#define GCC_VERSION (__GNUC__ * 10000 \
                + __GNUC_MINOR__ * 100 \
                   + __GNUC_PATCHLEVEL__)


class Variable {
  std::string name;
public:
  Variable(const std::string & nname):name(nname){};
  const std::string & getName () const { return name; }
  bool operator== (const Variable & v) const {
    return v.name == name;
  }
  size_t hash () const { 
#if GCC_VERSION < 40300
    return __gnu_cxx::hash<const char*>()(name.c_str());
#else
    return std::tr1::hash<std::string>() (name);
#endif
  }


};



#endif
