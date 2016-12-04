#include "Stats.h"

Stats::Stats() {
  cycles = PIPESTAGES - 1; // pipeline startup cost
  flushes = 0;
  bubbles = 0;
  memops = 0;
  branches = 0;
  taken = 0;
  stalls = 0;
  rawHazards = 0;
  exe1Hazards = 0;
  exe2Hazards = 0;
  mem1Hazards = 0;
  mem2Hazards = 0;
  
  for(int i = IF1; i < PIPESTAGES; i++) {
    resultReg[i] = -1;
    availability[i] = -1;
  }
}

void Stats::clock(PIPESTAGE stage) {
  cycles++;

  // pipeline the register tracking from provided start stage
  // (ops in 'stage' thru end of pipe advance, ops in stages before 'stage'
  // are frozen)
  for(int i = WB; i > stage; i--) {
    resultReg[i] = resultReg[i-1];
    availability[i] = availability[i - 1];
  }
  // inject no-op into 'stage'
  resultReg[stage] = -1;
}

void Stats::registerSrc(int r, int needed) {
  if (r == 0)
    return;

  for(int i = EXE1; i < WB; ++i)
   {
     if(resultReg[i] == r)
     {
       if(EXE1 == i)
        exe1Hazards++;
       if(EXE2 == i)
        exe2Hazards++;
       if(MEM1 == i)
        mem1Hazards++;
       if(MEM2 == i)
        mem2Hazards++;

       rawHazards++;
       int bubbleNum = (availability[i] - i) - (needed - ID);

       while(bubbleNum > 0)
       {
         bubble();
         --bubbleNum;
       }
       break;
     }  
   }
}

void Stats::registerDest(int r, int available) {
    resultReg[ID] = r;
    availability[ID] = available;
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

void Stats::stall(int latency) {
  for(int i = 0; i < latency; ++i)
  {
    clock(WB);
    stalls++;
  }
}
