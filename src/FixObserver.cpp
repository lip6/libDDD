#include "FixObserver.hh"

#include <cstddef>

namespace fobs {

class DefaultObserver : public FixObserver {
public:
  DefaultObserver (): FixObserver () {}
  
  virtual bool should_interrupt () { return false; }
  virtual void update (const GSDD & after, const GSDD & before) {}
  virtual void update (const GDDD & after, const GDDD & before) {}
};

static FixObserver * obs = NULL;

void
set_fixobserver (FixObserver * o)
{
  delete obs;
  obs = o;
}

FixObserver *
get_fixobserver ()
{
  if (obs == NULL)
    obs = new DefaultObserver ();
  return obs;
}

}