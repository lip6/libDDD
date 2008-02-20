#include "process.hpp"

// for times
#include <sys/times.h>
// for sysconf
#include <unistd.h>

#include <string>
#include <cstdio>
using namespace std;

namespace process {

double getTotalTime() {
    struct tms tbuff;
    double m;
    times(&tbuff);
    m= ((double)tbuff.tms_utime+(double)tbuff.tms_stime ) / ((double) sysconf(_SC_CLK_TCK));
    return m;
}


size_t getResidentMemory() {
  static bool is_available = true;
  if (! is_available) {
    return 0;
  }
  size_t m;
  char cmd [255];
  const char * tmpff = "ps-run";
  
  sprintf (cmd,"ps --no-heading o rss %d > %s",getpid(),tmpff);
  int ret = system (cmd);
  FILE* fd ;
  if (ret || ((fd = fopen(tmpff,"r")) == NULL)) {
    printf ("When attempting to system execute : %s \n",cmd);
    perror(" Error opening temporary file to sample resident shared size (raised in process.cpp).");
    perror(" Will report 0 as resident memory.");
    is_available = false;
    return 0;
  }
  fscanf(fd,"%ld",&m);
  unlink(tmpff);    
  return m;
 }


} // namespace process
