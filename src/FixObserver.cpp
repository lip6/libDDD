#include "FixObserver.hh"

#include <cstddef>

namespace fobs {

class DefaultObserver : public FixObserver {
public:
  DefaultObserver (): FixObserver () {}
  
  bool should_interrupt (const GSDD &, const GSDD &) { return false; }
  bool should_interrupt (const GDDD &, const GDDD &) { return false; }
  bool was_interrupted () const { return false; }
  void update (const GSDD & after, const GSDD & before) {}
  void update (const GDDD & after, const GDDD & before) {}
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