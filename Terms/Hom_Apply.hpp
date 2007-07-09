#ifndef __HOM_APPLY_H_
#define __HOM_APPLY_H_

#include <tr1/memory>

#include <SDD.h>
#include <SHom.h>

class EnvShom
  : public StrongShom
{
protected:
  SDD environment_;
public:
  EnvShom(const SDD&);
  virtual GShom defineEnvironment(const SDD&) const = 0;
};

class _hom_apply
  : public EnvShom
{
private:
  GShom                                   extract_;
  ::std::tr1::shared_ptr< const EnvShom > next_;
public:
  _hom_apply (const GShom&,
              ::std::tr1::shared_ptr< const EnvShom >);
  GSDD    phiOne()                      const;
  GShom   phi(int, const DataSet&)      const;
  size_t  hash()                        const;
  bool    operator==(const StrongShom&) const;
  GShom   defineEnvironment(const SDD&) const;
};

#endif //__HOM_APPLY_H_

