#include "process.hpp"

// for times
#include <sys/times.h>
// for sysconf
#include <unistd.h>

#include <string>
#include <cstdio>
#include <cstdlib>

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
#ifdef linux
  {
    int size;

    FILE* file = fopen("/proc/self/statm", "r");
    if (!file)
      return -1;
    int res = fscanf(file, "%d", &size);
    (void) fclose(file);
    if (res != 1)
      return -1;
    return size;
  }
#else
  size_t m;
  char cmd [255];
  const char * tmpff = "ps-run";
  
  sprintf (cmd,"ps o rss %d > %s",getpid(),tmpff);
  int ret = ::system (cmd);
  FILE* fd ;
  if (ret || ((fd = fopen(tmpff,"r")) == NULL)) {
    if (ret) {
      perror("Execution of ps command failed.\n"); 
      fprintf(stderr,"When attempting to system execute : %s \n",cmd);
    } else {
      perror(" Error opening temporary file to sample resident shared size.");
    }
    fprintf(stderr," Will report 0 as resident memory \nThis is a known limitation on cygwin. Please report this to ddd@lip6.fr if you have another os.");
    is_available = false;
    return 0;
  }

  fscanf(fd,"%s\n%zu",cmd,&m);
  unlink(tmpff);    
  return m;
#endif
 }


} // namespace process
