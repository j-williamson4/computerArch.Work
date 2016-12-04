/****************************
 * John Williamson
 * CS 3339 Spring 2016 Sec:251
 * Project 4
 * Due 03/30
 ****************************/
#include "CPU.h"

const string CPU::regNames[] = {"$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
                                "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
                                "$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
                                "$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"};

CPU::CPU(uint32_t pc, Memory &iMem, Memory &dMem) : pc(pc), iMem(iMem), dMem(dMem) {
  for(int i = 0; i < NREGS; i++) {
    regFile[i] = 0;
  }
  hi = 0;
  lo = 0;
  regFile[28] = 0x10008000; // gp
  regFile[29] = 0x10000000 + dMem.getSize(); // sp

  instructions = 0;
  stop = false;
}

void CPU::run() {
  while(!stop) {
    instructions++;

    fetch();
    decode();
    execute();
    pipeStats.clock(IF1);
    D(printRegFile());
    
  }
}

void CPU::fetch() {
  instr = iMem.loadWord(pc);
  pc = pc + 4;
}
  /*    (6)         (5)             (5)               (5)               (5)           (6)
     | opcode | source reg. | 2nd source reg. | destination reg. |  shift amount | function | (R-Format) *R and I can vary*
         
     | opcode | source reg. | 2nd source reg. |               Word/Byte offset              | (I-Format)
  
     | opcode |                             psuedodirect address                            | (J-Format)
  */

void CPU::decode() {
  uint32_t opcode = instr >> 26;
  uint32_t rs = (instr >> 21) & 0x1f;
  uint32_t rt = (instr >> 16) & 0x1f;
  uint32_t rd = (instr >> 11) & 0x1f;
  uint32_t shamt = (instr >> 6) & 0x1f;
  uint32_t funct = instr & 0x3f;
  uint32_t uimm = instr & 0xffff;
  int32_t  simm = ((signed)uimm << 16) >> 16;
  uint32_t addr = instr & 0x3ffffff;

  opIsMultDiv = false;
  opIsLoad = false;
  opIsStore = false;
  aluOp = OUT_S1;
  //default to false to prevent potenial error
  
  D(cout << "  " << hex << setw(8) << pc - 4 << ":");
  switch(opcode) {
    case 0x00:
      switch(funct) {
        // Shift Left Logical (Multiply by 2^shamt);
        // $rd = $rs << shamt
        case 0x00: D(cout << "sll " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                  aluOp = SHF_L;
                  aluSrc1 = regFile[rs];
                  aluSrc2 = shamt;
                  writeDest = true; destReg = rd;
                  pipeStats.registerDest(rd, MEM1);
                  pipeStats.registerSrc(rs, EXE1);
                   break;
        // Shift Right Arithmetic (Divide by 2^shamt)
        // $rd = $rs >> shamt
        case 0x03: D(cout << "sra " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                  aluOp = SHF_R;
                  aluSrc1 = regFile[rs];
                  aluSrc2 = shamt;
                  writeDest = true; destReg = rd;
                  pipeStats.registerDest(rd, MEM1);
                  pipeStats.registerSrc(rs, EXE1);
                   break;
        // Jump Register
        // j $rs 
        case 0x08: D(cout << "jr " << regNames[rs]); 
                  aluOp = OUT_S1;
                  writeDest = false;
                  aluSrc1 = pc;
                  pc = regFile[rs];
                  pipeStats.registerSrc(rs,ID);
                  pipeStats.flush(2);
                   break;
        // Move From hi (hi contains remainder after div)
        // $rd = hi;
        case 0x10: D(cout << "mfhi " << regNames[rd]);
                  aluOp = ADD;
                  aluSrc1 = hi;
                  aluSrc2 = 0;
                  writeDest = true; destReg = rd;
                  pipeStats.registerDest(rd, MEM1);
                  pipeStats.registerSrc(REG_HILO, EXE1);
                   break;
        case 0x12: D(cout << "mflo " << regNames[rd]);
        // Move From lo (lo contains MUL/DIV result)
        // $rd = lo;
                  aluOp = ADD;
                  aluSrc1 = lo;
                  aluSrc2 = 0;
                  writeDest = true; destReg = rd;
                  pipeStats.registerDest(rd, MEM1);
                  pipeStats.registerSrc(REG_HILO, EXE1);
                   break;
        case 0x18: D(cout << "mult " << regNames[rs] << ", " << regNames[rt]);
        // Multiply $rs * $rt
                  aluOp = MUL;
                  aluSrc1 = regFile[rs];
                  aluSrc2 = regFile[rt];
                  pipeStats.registerSrc(rs, EXE1);
                  pipeStats.registerSrc(rt, EXE1);
                  pipeStats.registerDest(REG_HILO, WB);
                  writeDest = false; opIsMultDiv = true;
                   break;
        case 0x1a: D(cout << "div " << regNames[rs] << ", " << regNames[rt]);
        // Division $rs / $rt
        // Quotient in LO, Remainder in HI
                  aluOp = DIV;
                  aluSrc1 = regFile[rs];
                  aluSrc2 = regFile[rt];
                  pipeStats.registerSrc(rs, EXE1);
                  pipeStats.registerSrc(rt, EXE1);
                  pipeStats.registerDest(REG_HILO, WB);
                  writeDest = false; opIsMultDiv = true;
                   break;
        case 0x21: D(cout << "addu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
        // Add (ignore overflow)
        // $rd = $rs + $rt;
                  aluOp = ADD;
                  aluSrc1 = regFile[rs];
                  aluSrc2 = regFile[rt];
                  writeDest = true; destReg = rd;
                  pipeStats.registerDest(rd, MEM1);
                  pipeStats.registerSrc(rs, EXE1);
                  pipeStats.registerSrc(rt, EXE1);  
                   break;
        case 0x23: D(cout << "subu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
        // Subtract (ignore overflow)
        // $rd = $rs - $rt
                  aluOp = ADD;
                  aluSrc1 = regFile[rs];
                  aluSrc2 = -regFile[rt];
                  writeDest = true; destReg = rd;
                  pipeStats.registerDest(rd, MEM1);
                  pipeStats.registerSrc(rs, EXE1);
                  pipeStats.registerSrc(rt, EXE1);
                   break;
        // Set Less Than
        // (rs < rt) ? 1 : 0       
        case 0x2a: D(cout << "slt " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                  aluOp = CMP_LT;
                  aluSrc1 = regFile[rs];
                  aluSrc2 = regFile[rt];
                  writeDest = true; destReg = rd;
                  pipeStats.registerDest(rd, MEM1);
                  pipeStats.registerSrc(rs, EXE1);
                  pipeStats.registerSrc(rt, EXE1);
                   break;
        default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
      }
      break;
    // Jump instruction
    // Sets Program Counter to calculated address
    case 0x02: D(cout << "j " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4        
              aluOp = OUT_S1;
              writeDest = false;
              aluSrc1 = pc;
              pc = ((pc & 0xf0000000) | (addr << 2));
              pipeStats.flush(2);
               break;
    // Jump and Link
    // Sets Program Counter to an address, and stores return address in $ra
    case 0x03: D(cout << "jal " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 41
              aluOp = OUT_S1; // pass src1 straight through ALU
              writeDest = true; destReg = REG_RA; // writes PC+4 to $ra
              aluSrc1 = pc;
              pc = ((pc & 0xf0000000) | (addr << 2));
              pipeStats.registerDest(REG_RA, EXE1);
              pipeStats.flush(2);
               break;
    // Branch on Equal
    case 0x04: D(cout << "beq " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2)); 
              aluOp = OUT_S1;
              aluSrc1 = pc;
              writeDest = false;
              pipeStats.registerSrc(rt, ID);
              pipeStats.registerSrc(rs, ID);
              pipeStats.countBranch();
              if( regFile[rs] == regFile[rt] ) 
              {
                pc += (simm << 2);
                pipeStats.countTaken();
                pipeStats.flush(2);
                
              }
               break;
    // Branch on not Equal
    case 0x05: D(cout << "bne " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2)); // TO DO
              aluOp = OUT_S1;
              aluSrc1 = pc;
              writeDest = false;
              pipeStats.registerSrc(rt, ID);
              pipeStats.registerSrc(rs, ID);
              pipeStats.countBranch();
              if (regFile[rs] != regFile[rt]) 
              {
                pc += (simm << 2);
                pipeStats.countTaken();
                pipeStats.flush(2);
              }
               break;
    // Add Immediate 
    // $rt = $rs + (signed)imm
    case 0x08: D(cout << "addi " << regNames[rt] << ", " << regNames[rs] << ", " << dec << simm); 
              aluOp = ADD;
              aluSrc1 = regFile[rs];
              aluSrc2 = simm;
              writeDest = true; destReg = rt;
              pipeStats.registerDest(rt, MEM1);
              pipeStats.registerSrc(rs, EXE1);
               break;
    // Add Immediate (ignore overflow)
    // $rt = $rs + (signed)imm
    case 0x09: D(cout << "addiu " << regNames[rt] << ", " << regNames[rs] << ", " << dec << simm);
              aluOp = ADD;
              aluSrc1 = regFile[rs];
              aluSrc2 = simm;
              writeDest = true; destReg = rt;
              pipeStats.registerDest(rt, MEM1);
              pipeStats.registerSrc(rs, EXE1);
               break;
    // And Immediate
    // $rt = $rs & imm
    case 0x0c: D(cout << "andi " << regNames[rt] << ", " << regNames[rs] << ", " << dec << uimm);
              aluOp = AND;
              aluSrc1 = regFile[rs];
              aluSrc2 = uimm;
              writeDest = true; destReg = rt;
              pipeStats.registerDest(rt, MEM1);
              pipeStats.registerSrc(rs, EXE1);
               break;
    // Load Upper Immediate
    // *store upper 16 bits of immediate into $rt*
    case 0x0f: D(cout << "lui " << regNames[rt] << ", " << dec << simm);
              aluOp = SHF_L;
              aluSrc1 = simm;
              aluSrc2 = 16;
              writeDest = true; destReg = rt;
              pipeStats.registerDest(rt, MEM1);
               break;
    // Exception Handling
    case 0x1a: D(cout << "trap " << hex << addr);
              aluOp = OUT_S1; // don't need the ALU
              writeDest = false;
               switch(addr & 0xf) {
                 case 0x0: cout << endl; break;
                 case 0x1: cout << " " << (signed)regFile[rs]; pipeStats.registerSrc(rs, EXE1); break;
                 case 0x5: cout << endl << "? "; cin >> regFile[rt]; pipeStats.registerDest(rt, MEM1); break;
                 case 0xa: stop = true; break;
                 default: cerr << "unimplemented trap: pc = 0x" << hex << pc - 4 << endl;
                          stop = true;
               }
               break;
    // Load Word 
    // $rt <- offSet($rs)
    case 0x23: D(cout << "lw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
              opIsLoad = true;
              writeDest = true;
              aluOp = ADD;
              aluSrc1 = regFile[rs]; 
              aluSrc2 = simm;
              destReg = rt;
              pipeStats.registerDest(rt, WB);
              pipeStats.registerSrc(rs, EXE1);
               break;
    // $rt -> offSet($rs)
    case 0x2b: D(cout << "sw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
              opIsStore = true;
              writeDest = false;
              aluOp = ADD;
              aluSrc1 = regFile[rs];
              aluSrc2 = simm;
              storeData = regFile[rt];
              pipeStats.registerSrc(rs, EXE1);
              pipeStats.registerSrc(rt, MEM1);
               break;
    default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
  }
  D(cout << endl);
}

void CPU::execute() {
  uint32_t aluOut = alu.op(aluOp, aluSrc1, aluSrc2);
  if(opIsLoad)
  {
    aluOut = dMem.loadWord(aluOut);
    pipeStats.countMemOp();
  }
  else if(opIsStore)
  {
    dMem.storeWord(storeData, aluOut);
    pipeStats.countMemOp();
  }
  else if(opIsMultDiv) {
    hi = alu.getUpper();
    lo = alu.getLower();
  }

  // Regfile update (but never write to register 0)
  if(writeDest && destReg > 0)
    regFile[destReg] = aluOut;
}

void CPU::printRegFile() {
  cout << hex;
  for(int i = 0; i < NREGS; i++) {
    cout << "    " << regNames[i];
    if(i > 0) cout << "  ";
    cout << ": " << setfill('0') << setw(8) << regFile[i];
    if( i == (NREGS - 1) || (i + 1) % 4 == 0 )
      cout << endl;
  }
  cout << "    hi   : " << setfill('0') << setw(8) << hi;
  cout << "    lo   : " << setfill('0') << setw(8) << lo;
  cout << dec << endl;
}

void CPU::printFinalStats() {
  cout << fixed << setprecision(1);
  cout << "Program finished at pc = 0x" << hex << pc << "  ("
       << dec << instructions << " instructions executed)" << endl << endl;
  
  cout << "Cycles: " << pipeStats.getCycles() << endl;
  cout << "CPI: " << pipeStats.getCycles() / static_cast<double>(instructions) << endl << endl;
  cout << "Bubbles: " << pipeStats.getBubbles() << endl;
  cout << "Flushes: " << pipeStats.getFlushes() << endl << endl;
  
  cout << "RAW hazards: "  << pipeStats.getRAWHazards() << " (1 per every "<< fixed << setprecision(2) << (static_cast<double>(instructions) / pipeStats.getRAWHazards())<< " instructions) " << endl;   
  cout << "   On EXE1 op: " << pipeStats.getEXE1Hazards() << " (" << fixed << setprecision(0) << 100.0 * (pipeStats.getEXE1Hazards() / static_cast<double>(pipeStats.getRAWHazards())) << "%)" << endl;   
  cout << "   On EXE2 op: " << pipeStats.getEXE2Hazards() << " (" << fixed << setprecision(0) << 100.0 * (pipeStats.getEXE2Hazards() / static_cast<double>(pipeStats.getRAWHazards())) << "%)" << endl; 
  cout << "   On MEM1 op: " << pipeStats.getMEM1Hazards() << " (" << fixed << setprecision(0) << 100.0 * (pipeStats.getMEM1Hazards() / static_cast<double>(pipeStats.getRAWHazards())) << "%)" << endl;
  cout << "   On MEM2 op: " << pipeStats.getMEM2Hazards() << " (" << fixed << setprecision(0) << 100.0 * (pipeStats.getMEM2Hazards() / static_cast<double>(pipeStats.getRAWHazards())) << "%)" << endl; 
}
