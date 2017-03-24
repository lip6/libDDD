#ifndef __PROCESS_H__
#define __PROCESS_H__

// for size_t
#include <stddef.h>


namespace process {

  // rely on "times" 
  double getTotalTime ();
 
  size_t getResidentMemory () ;

}

#endif
