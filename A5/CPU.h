#ifndef __CPU_H
#define __CPU_H

#include <iostream>
#include <cstdint>
#include <string>
#include <iomanip>
#include <cstdlib>
#include "Memory.h"
#include "ALU.h"
#include "Debug.h"
#include "CacheStats.h"
#include "Stats.h"
using namespace std;

class CPU {
  private:
    static const int NREGS = 32;
    static const string regNames[];

    static const int REG_RA = 31;
    static const int REG_HILO = 32;

    uint32_t pc;
    uint32_t instr;
    uint32_t regFile[NREGS];
    uint32_t hi, lo;

    ALU alu;
    Stats pipeStats;
    CacheStats cacheStats;
    
    Memory &iMem;
    Memory &dMem;

    int instructions;
    bool stop;

    bool opIsLoad, opIsStore, opIsMultDiv;
    ALU_OP aluOp;
    bool writeDest;
    int destReg;
    uint32_t aluSrc1, aluSrc2;
    uint32_t storeData;

  public:
    CPU(uint32_t pc, Memory &iMem, Memory &dMem);

    void run();
    void printFinalStats();

  private:
    void fetch();
    void decode();
    void execute();
    void printRegFile();
};

#endif
