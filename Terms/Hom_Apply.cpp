#include <cassert>
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
  assert(false);
  return GSDD::top;
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

