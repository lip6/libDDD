#include <cassert>
#include "Hom_Build.hpp"

// _hom_build

_hom_build::_hom_build (int                                     variable,
                        ::std::tr1::shared_ptr< const EnvShom > sub,
                        ::std::tr1::shared_ptr< const EnvShom > next)
  : EnvShom(SDD::null),
    variable_(variable),
    sub_(sub),
    next_(next.get())
{}

GSDD
_hom_build::phiOne() const
{
  SDD sub  =  sub_->defineEnvironment(environment_)(SDD::one);
  SDD next = next_->defineEnvironment(environment_)(SDD::one);
  return GSDD(variable_, sub, next);
}

GShom
_hom_build::phi(int /*vr*/, const DataSet& /*vl*/) const
{
  assert(false);
}

size_t
_hom_build::hash() const
{
  return 1 // FIXME
       ^ ::__gnu_cxx::hash< GSDD >()(environment_)
       ^ ::__gnu_cxx::hash< int >()(variable_)
       ^ sub_->hash()
       ^ next_->hash();
}

bool
_hom_build::operator==(const StrongShom& other) const
{
  const _hom_build& o = reinterpret_cast< const _hom_build& >(other);
  return environment_ == o.environment_
      && variable_    == o.variable_
      && sub_         == o.sub_
      && *next_       == *o.next_;
}

GShom
_hom_build::defineEnvironment(const SDD& env) const
{
  _hom_build* result = new _hom_build(variable_, sub_, next_);
  result->environment_ = env;
  return result;
}

// Helper function:

EnvironmentShom
build(int var,
      EnvironmentShom sub,
      EnvironmentShom next)
{
  return EnvironmentShom(new _hom_build(var, sub, next));
}

