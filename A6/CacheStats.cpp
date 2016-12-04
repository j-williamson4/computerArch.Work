/****************************
 * John Williamson
 * CS 3339 Spring 2016 Sec: 251
 * Project 5
 * Due 04/13
 ****************************/
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include "CacheStats.h"
using namespace std;

CacheStats::CacheStats() {
  cout << "Cache Config: ";
  if(!CACHE_EN) {
    cout << "cache disabled" << endl;
  } else {
    cout << (SETS * WAYS * BLOCKSIZE) << " B (";
    cout << BLOCKSIZE << " bytes/block, " << SETS << " sets, " << WAYS << " ways)" << endl;
    cout << "  Latencies: Hit = " << HIT_LATENCY << " cycles, ";
    cout << "Read = " << READ_LATENCY << " cycles, ";
    cout << "Write = " << WRITE_LATENCY << " cycles" << endl;
  }

  reads = 0;
  writes = 0;
  read_misses = 0;
  write_misses = 0;
  writebacks = 0;

  /* TODO: your code here */
  for(int i = 0; i < SETS; i++) 
  {
    for (int j = 0; j < WAYS; j++)
    {
       cacheArray[i][j].valid = false;
       cacheArray[i][j].dirty = false;   
       cacheArray[i][j].tag = 0;
    }
  }
  for(int k = 0; k < SETS; k++)
  {
     roundRobin[k] = 0;
  }
}

int CacheStats::access(uint32_t addr, ACCESS_TYPE type) {
  if(!CACHE_EN) { // no cache
    return (type == LOAD) ? READ_LATENCY : WRITE_LATENCY;
  }
  /* TODO: your code here */
  int latency = 0;
  int addrTag = addr / (BLOCKSIZE * SETS);
  int index = (addr / BLOCKSIZE) % SETS;
  int rrIndex = roundRobin[index];
  
  if (type == LOAD) {
    // Lookup
     reads++;
     latency += HIT_LATENCY;

     // Hit?
     for (int i = 0; i < WAYS; ++i)
     {
       if (cacheArray[index][i].valid == true && cacheArray[index][i].tag == addrTag) {
        return latency;
       }
     }
  
     // Miss
     latency += READ_LATENCY;
     read_misses++;
     if(cacheArray[index][rrIndex].dirty == true) 
     {
       latency += WRITE_LATENCY;
       writebacks++;
     }

    // update tag, dirty, and valid bit
    cacheArray[index][rrIndex].tag = addrTag;
    cacheArray[index][rrIndex].dirty = false;
    cacheArray[index][rrIndex].valid = true;
    roundRobin[index] = (roundRobin[index] + 1) % WAYS;

    return latency;
  }
  
  if(type == STORE) {
    // Lookup
    writes++;
    
    // Hit?
    latency += HIT_LATENCY;
    for (int i = 0; i < WAYS; ++i)
    {
      if(cacheArray[index][i].valid == true && cacheArray[index][i].tag == addrTag)
        {
          cacheArray[index][i].dirty = true; // set dirty bit
          return latency;
        }
    }

    // Miss
    write_misses++;
    latency += READ_LATENCY;
    if(cacheArray[index][rrIndex].dirty == true)
    {
      latency += WRITE_LATENCY;
      writebacks++;
    }
    
    // Update tag, dirty, valid bit, and Round Robin index
    cacheArray[index][rrIndex].tag = addrTag;  
    cacheArray[index][rrIndex].dirty = true; 
    cacheArray[index][rrIndex].valid = true;
    roundRobin[index] = (roundRobin[index] + 1) % WAYS; 

    return latency;
   }
}

void CacheStats::printFinalStats() {
  /* TODO: your code here (don't forget to drain the cache of writebacks) */
   
   for (int i = 0; i < SETS; i++)
   {
      for(int j = 0; j < WAYS; j++)
       {
         if(cacheArray[i][j].dirty == true) {
           writebacks++;
		 } 
       }
   }
   
  int accesses = reads + writes;
  int misses = read_misses + write_misses;
  cout << "Accesses: " << accesses << endl;
  cout << "  Reads: " << reads << endl;
  cout << "  Writes: " << writes << endl;
  cout << "Misses: " << misses << endl;
  cout << "  Read misses: " << read_misses << endl;
  cout << "  Write misses: " << write_misses << endl;
  cout << "Writebacks: " << writebacks << endl;
  cout << "Hit Ratio: " << fixed << setprecision(1) << 100.0 * (accesses - misses) / accesses;
  cout << "%" << endl;
}
