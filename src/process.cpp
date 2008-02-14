#include "process.hpp"

// for times
#include <sys/times.h>
// for sysconf
#include <unistd.h>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

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
    size_t m;

    string s;
    string tmpfile = string("HaDDoCK_restmp");
    int i=0;
    stringstream spid; spid<<getpid();
    string pid=spid.str();
    stringstream command;
    command<<"ps v "<<pid<<" >"<<tmpfile;
    system (command.str().c_str());
    ifstream f(tmpfile.c_str());
    do {
        f>>s;
        i++;
    } while (s!=string("RSS"));
    do {
        f>>s;
    } while (s!=pid);
    for (int j=2; j<i; j++) f>>s;
    f>>m;

    unlink(tmpfile.c_str());

    return m;
}


} // namespace process
