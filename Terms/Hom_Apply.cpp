#include <cassert>
#include "DDD.h"
#include "Hom_Apply.hpp"

// EnvShom

EnvShom::EnvShom(const SDD& environment)
  : environment_(environment)
{}

// _hom_apply

_hom_apply::_hom_apply (const GShom& extract,
                        ::std::tr1::shared_ptr< const EnvShom > next)
  : EnvShom(SDD::null),
    extract_(extract),
    next_(next.get())
{}

GSDD
_hom_apply::phiOne() const
{
  return environment_;
}

GShom
_hom_apply::phi(int /*vr*/, const DataSet& vl) const
{
  SDD subenv = extract_(reinterpret_cast< const SDD& >(vl));
  return next_->defineEnvironment(subenv + environment_);
}

size_t
_hom_apply::hash() const
{
  return 1 // FIXME
       ^ ::__gnu_cxx::hash< GSDD >()(environment_)
       ^ ::__gnu_cxx::hash< GShom >()(extract_)
       ^ next_->hash();
}

bool
_hom_apply::operator==(const StrongShom& other) const
{
  const _hom_apply& o = reinterpret_cast< const _hom_apply& >(other);
  return environment_ == o.environment_
      && extract_     == o.extract_
      && *next_       == *o.next_;
}

GShom
_hom_apply::defineEnvironment(const SDD& env) const
{
  _hom_apply* result = new _hom_apply(extract_, next_);
  result->environment_ = env;
  return result;
}

// _hom_variable

_hom_variable::_hom_variable (int variable)
  : variable_(variable)
{}

GSDD
_hom_variable::phiOne() const
{
  return GSDD::one;
}

GShom
_hom_variable::phi(int /*vr*/, const DataSet& vl) const
{
  return GSDD(0, DDD(0, variable_), GSDD(1, vl, GSDD::one));
}

size_t
_hom_variable::hash() const
{
  return 1 // FIXME
       ^ ::__gnu_cxx::hash< int >()(variable_);
}

bool
_hom_variable::operator==(const StrongShom& other) const
{
  const _hom_variable& o = reinterpret_cast< const _hom_variable& >(other);
  return variable_    == o.variable_;
}

// Helper functions:

EnvironmentShom
apply(const GShom&    extract,
      EnvironmentShom next)
{
  return EnvironmentShom(new _hom_apply(extract, next));
}

GShom
variable(int var)
{
  return new _hom_variable(var);
}

