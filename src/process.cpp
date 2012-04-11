#include "process.hpp"

// for clock
#include <time.h>

#include <string>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#if defined(LINUX) || defined(linux) || defined(__CYGWIN__) || defined(cygwin)
#define USE_PROC_MEM 1
#elif defined(WIN32) && defined (__GLIBC__)
#define USE_MALLINFO 1 
#else 
#define OS_APPLE 1
#endif
 
#if OS_APPLE // use mach function
#   include <mach/mach_init.h>
#   include <mach/task.h>
#elif USE_PROC_MEM
#   include <unistd.h>
#elif USE_MALLINFO
#   include <malloc.h> 
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
  double val = clock();
  return (val*1000.0 / CLOCKS_PER_SEC) / 1000;

}


/*****************************************************************************
 * Return total amount of bytes allocated.
 *****************************************************************************/
unsigned long
MemoryUsed( void )
{
#ifdef USE_PROC_MEM
  {
    size_t total_size, rss_size;

    FILE* file = fopen("/proc/self/statm", "r");
    if (!file)
      return 0;
    int res = fscanf(file, "%zu %zu", &total_size, &rss_size);
    (void) fclose(file);
    if (res != 2)
      return 0;
    return rss_size * page_mult();
  }
#elif USE_MALLINFO

    // Per delorie.com:
    // Example:
    // struct mallinfo info = mallinfo();
    // printf("Memory in use: %d bytes\n", info.usmblks + info.uordblks);
    // printf("Total heap size: %d bytes\n", info.arena);
    struct mallinfo meminfo;
    meminfo = mallinfo();
    return meminfo.usmblks;

#elif OS_APPLE

    // Use Mach functions.
    task_basic_info        machInfo  = { 0 };
    mach_port_t            machTask  = mach_task_self();
    mach_msg_type_number_t machCount = TASK_BASIC_INFO_COUNT;
    if ( task_info( machTask, TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&machInfo), &machCount ) == KERN_SUCCESS )
        return machInfo.resident_size;
    else {
		std::cerr << "Detected Apple OS to obtain process memory usage, but call to kernel failed. Will report 0." << std::endl;
        return 0;  // error
    }

#else
    std::cerr << "Unsupported OS to obtain process memory usage. Will report 0." << std::endl;
    return 0;  // unsupported

#endif
}



/** in Bytes */
size_t getResidentMemory() {
  static bool memAvailable = true;
  if (memAvailable) {
	unsigned long val = MemoryUsed();
	if (val == 0) 
		memAvailable = false;
	return val ;	
  } 
  return 0;
 }


} // namespace process
