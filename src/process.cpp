#include "process.hpp"

// for times
#include <sys/times.h>
// for sysconf
#include <unistd.h>

#include <string>
#include <cstdio>
#include <cstdlib>

#if defined(LINUX) || defined(linux) || defined(__CYGWIN__) || defined(cygwin) || defined(WIN32)
#define USE_PROC_MEM
#endif

using namespace std;

namespace process {

#ifdef USE_PROC_MEM 
  static size_t page_mult_ = 0;
  static size_t page_mult () {
    if (! page_mult_) {
      size_t page_size = sysconf(_SC_PAGESIZE);
      page_mult_ = page_size / 1024;
    }
    return page_mult_;
  }
#endif

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
#ifdef USE_PROC_MEM
  {
    size_t total_size, rss_size;

    FILE* file = fopen("/proc/self/statm", "r");
    if (!file)
      return -1;
    int res = fscanf(file, "%zu %zu", &total_size, &rss_size);
    (void) fclose(file);
    if (res != 2)
      return -1;
    return rss_size * page_mult();
  }
#else
  /// i.e. mostly MacOS target
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
