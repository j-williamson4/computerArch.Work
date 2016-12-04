#ifndef __CACHE_STATS_H
#define __CACHE_STATS_H

#include <cstdint>
#include "Debug.h"
using namespace std;

#ifndef CACHE_EN
#define CACHE_EN 1
#endif

#ifndef BLOCKSIZE
#define BLOCKSIZE 32
#endif

#ifndef SETS
#define SETS 8
#endif

#ifndef WAYS
#define WAYS 4
#endif

#ifndef HIT_LATENCY
#define HIT_LATENCY 0
#endif

#ifndef READ_LATENCY
#define READ_LATENCY 30
#endif

#ifndef WRITE_LATENCY
#define WRITE_LATENCY 10
#endif

enum ACCESS_TYPE { LOAD, STORE };

class CacheStats {
  private:
    /* TODO: you probably want to add some member variables here to represent
     * the parts of the cache contents you need to model! */

    int reads;
    int writes;
    int read_misses;
    int write_misses;
    int writebacks;

    struct cacheEle {
      bool valid;
      bool dirty;
      int tag;
    };

    cacheEle cacheArray[SETS][WAYS];    
    int roundRobin[SETS];
   
  public:
    CacheStats();
    int access(uint32_t, ACCESS_TYPE);
    void printFinalStats();
};

#endif
