#include "Stats.h"

Stats::Stats() {
  cycles = PIPESTAGES - 1; // pipeline startup cost
  flushes = 0;
  bubbles = 0;

  memops = 0;
  branches = 0;
  taken = 0;

  for(int i = IF1; i < PIPESTAGES; i++) {
    resultReg[i] = -1;
  }
}

void Stats::clock(PIPESTAGE stage) {
  cycles++;

  // pipeline the register tracking from provided start stage
  // (ops in 'stage' thru end of pipe advance, ops in stages before 'stage'
  // are frozen)
  for(int i = WB; i > stage; i--) {
    resultReg[i] = resultReg[i-1];
  }
  // inject no-op into 'stage'
  resultReg[stage] = -1;
}

void Stats::registerSrc(int r) {
   int bubbleNum;
   for(int i = EXE1; i < WB; ++i)
   {
     if(resultReg[i] == r)
     {
       bubbleNum = WB - i;
       while(bubbleNum > 0)
       {
        bubble();
        --bubbleNum;
       }
       break;
     }  
   }
}

void Stats::registerDest(int r) {
    resultReg[ID] = r;
}

void Stats::flush(int count) { // count == how many ops to flush
    for(int i = 0; i < count; ++i)
    {
        clock(IF1);
        ++flushes;
    }
}

void Stats::bubble() {
    bubbles++;
    clock(EXE1);
}
