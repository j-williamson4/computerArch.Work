#ifndef __BRANCH_PRED_H
#define __BRANCH_PRED_H
#include <cstdint>
#include "Debug.h"
using namespace std;
#ifndef BPRED_SIZE
#define BPRED_SIZE 64
#endif
class BranchPred {
  private:
    /* Your member variables here */
    int predictions;
    int pred_takens;
    int mispredictions;
    int mispred_direction;
    int mispred_target;
    struct prediction {
        int stateNum;
        int targetPC;
    };
    prediction predictionTable[BPRED_SIZE];

  public:
    BranchPred();
    /* Your member functions here */
    bool predict(uint32_t pc);
    bool update(uint32_t pc, uint32_t targetPC, bool prediction, bool actualTaken);
    void printFinalStats();
};
#endif
