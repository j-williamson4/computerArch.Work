#ifndef __STATS_H
#define __STATS_H
#include <iostream>
#include "Debug.h"
using namespace std;

enum PIPESTAGE { IF1 = 0, IF2 = 1, ID = 2, EXE1 = 3, EXE2 = 4, MEM1 = 5, 
                 MEM2 = 6, WB = 7, PIPESTAGES = 8 };

class Stats {
  private:
    long long cycles;
    int flushes;
    int bubbles;
    int stalls;
    int memops;
    int branches;
    int taken;
    
    int rawHazards; 
    int exe1Hazards;
    int exe2Hazards;
    int mem1Hazards;
    int mem2Hazards;
   
    int resultReg[PIPESTAGES];
    int availability[PIPESTAGES];

  public:
    Stats();

    void clock(PIPESTAGE stage);

    void flush(int count);
    void stall(int latency);
    void registerSrc(int r, int needed);
    void registerDest(int r, int available);

    void countMemOp() { memops++; }
    void countBranch() { branches++; }
    void countTaken() { taken++; }
    void countRAWHazards() { rawHazards++; }
    void countEXE1() { exe1Hazards++; }
    void countEXE2() { exe2Hazards++; }
    void countMEM1() { mem1Hazards++; }
    void countMEM2() { mem2Hazards++; }

    // getters
    long long getCycles() { return cycles; }
    int getFlushes() { return flushes; }
    int getBubbles() { return bubbles; }
    int getMemOps() { return memops; }
    int getBranches() { return branches; }
    int getTaken() { return taken; }
    int getStalls() { return stalls; }
    int getRAWHazards() { return rawHazards; }
    int getEXE1Hazards() { return exe1Hazards; }
    int getEXE2Hazards() { return exe2Hazards; }
    int getMEM1Hazards() { return mem1Hazards; }
    int getMEM2Hazards() { return mem2Hazards; }

  private:
    void bubble();
};

#endif
