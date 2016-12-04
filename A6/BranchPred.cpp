#include <iostream>
#include <iomanip>
#include <cstdint>
#include "BranchPred.h"

using namespace std;

BranchPred::BranchPred() {
  cout << "Branch Predictor Entries: " << BPRED_SIZE << endl;

  /* You should have some code that goes here */

  predictions = 0;
  pred_takens = 0;
  mispredictions = 0;
  mispred_direction = 0;
  mispred_target = 0;

  for (int i = 0; i < BPRED_SIZE; i++) {
    predictionTable[i].stateNum = 0;
    predictionTable[i].targetPC = 0;
  }
}

/* You should add functions here */
bool BranchPred::predict(uint32_t pc) {
  predictions++;
  int index = (pc >> 2) % BPRED_SIZE;

  if (predictionTable[index].stateNum >= 2) {
    pred_takens++;
    return true; //  Predict Taken
  }
  else {
    return false; // Predict Not Taken
  }
}

bool BranchPred::update(uint32_t pc, uint32_t targetPC, bool prediction, bool actualTaken) {
   
  int index = (pc >> 2) % BPRED_SIZE;  // index for predictionTable

  if (prediction == true && actualTaken == true) { // if predicted taken, and actually taken
   if(predictionTable[index].stateNum < 3) { // increment saturation counter if not already at 3
      predictionTable[index].stateNum++; // increment state if not at max
    }
   if(predictionTable[index].targetPC != targetPC) {
      mispredictions++;
      mispred_target++; // mispredicted branch target address 
      predictionTable[index].targetPC = targetPC; // taken, so overwrite tPC
      return true; // send true to notify for a flush
    }
  }
   
  if(prediction == false && actualTaken == false) { // predicted not-taken correctly, decrement stateNum
     if(predictionTable[index].stateNum > 0) { // if not at mininmum, decrease
       predictionTable[index].stateNum--;
     }
  }

  //Direction mispredictions 
  if(prediction == true && actualTaken == false) { // predicted taken, branch was not taken
    if(predictionTable[index].stateNum > 0) {
      predictionTable[index].stateNum--;
    }
      mispred_direction++; // predicted wrong (taken vs not-taken)
      mispredictions++;
  }
  
  if(prediction == false && actualTaken == true) { // predicted not taken, branch was taken
    if(predictionTable[index].stateNum < 3 ) { // increment saturation counter if not already at 1111
      predictionTable[index].stateNum++;
    }
    mispred_direction++;
    mispredictions++;
    predictionTable[index].targetPC = targetPC;  // update targetPC
  }

  return false; // if reached, we dont need to flush
}

void BranchPred::printFinalStats() {
  int correct = predictions - mispredictions;
  int not_takens = predictions - pred_takens;

  cout << setprecision(1);
  cout << "Branches predicted: " << predictions << endl;
  cout << "  Pred T: " << pred_takens << " ("
       << (100.0 * pred_takens/predictions) << "%)" << endl;
  cout << "  Pred NT: " << not_takens << endl;
  cout << "Mispredictions: " << mispredictions << " ("
       << (100.0 * mispredictions/predictions) << "%)" << endl;
  cout << "  Mispredicted direction: " << mispred_direction << endl;
  cout << "  Mispredicted target: " << mispred_target << endl;
  cout << "Predictor accuracy: " << (100.0 * correct/predictions) << "%" << endl;
}
