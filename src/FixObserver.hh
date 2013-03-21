#ifndef FIXOBSERVER_HH_
#define FIXOBSERVER_HH_

/// forward declarations
class GSDD;
class GDDD;

namespace fobs {

class FixObserver {
public:
  FixObserver (){}
  virtual ~FixObserver () {}
  
  /// \warning: if should_interrult() returns true, then it MUST return true
  /// at least until the next call to update().
  /// The attribute is_interrupted is here for this purpose: it can be set to
  /// true only by should_interrupt and to false only by update, and
  /// is_interrupted => should_interrupt returns true
  virtual bool should_interrupt () = 0;
  virtual void update (const GSDD & after, const GSDD & before) = 0;
  virtual void update (const GDDD & after, const GDDD & before) = 0;
};

FixObserver *
get_fixobserver ();

void
set_fixobserver (FixObserver *);

}

#endif /// ! FIXOBSERVER_HH_
