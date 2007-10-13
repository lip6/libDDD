#ifndef __HOM_BUILD_H_
#define __HOM_BUILD_H_

#include <tr1/memory>

#include <SDD.h>
#include <SHom.h>

#include "Hom_Apply.hpp" // for EnvShom

class _hom_build
  : public EnvShom
{
private:
  int                                     variable_;
  ::std::tr1::shared_ptr< const EnvShom > sub_;
  ::std::tr1::shared_ptr< const EnvShom > next_;
public:
  _hom_build (int,
              ::std::tr1::shared_ptr< const EnvShom >,
              ::std::tr1::shared_ptr< const EnvShom >);
  GSDD    phiOne()                      const;
  GShom   phi(int, const DataSet&)      const;
  size_t  hash()                        const;
  bool    operator==(const StrongShom&) const;
  GShom   defineEnvironment(const SDD&) const;
};

EnvironmentShom
build(int, EnvironmentShom, EnvironmentShom);

#endif //__HOM_BUILD_H_

