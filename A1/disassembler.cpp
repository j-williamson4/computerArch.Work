/****************************
 * John Williamson
 * CS 3339 Spring 2016
 * Project 1
 * Due: 02/03/2016
 ****************************/
#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include <iomanip>
using namespace std;

const int NREGS = 32;
const string regNames[NREGS] = {"$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
                                "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
                                "$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
                                "$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"};

///////////////////////////////////////////////////////////////////////////
// Disassemble the 32-bit MIPS instruction (assumed to be at the given PC)
// and print the assembly instruction to screen
// ALL YOUR CHANGES GO IN THIS FUNCTION
///////////////////////////////////////////////////////////////////////////
void disassembleInstr(uint32_t pc, uint32_t instr) {
  uint16_t opcode, rs, rt, rd, shamt, funct, uimm;
  uint32_t addr;
  int16_t simm;

  /*    (6)         (5)             (5)               (5)               (5)           (6)
     | opcode | source reg. | 2nd source reg. | destination reg. |  shift amount | function | (R-Format) *R and I can vary are arranged*
         
     | opcode | source reg. | 2nd source reg. |               Word/Byte offset              | (I-Format)
  
     | opcode |                             psuedodirect address                            | (J-Format) 
  */
  
  opcode = instr >> 26; 
  // bit shifting right in order to get most significant 6 bits from instruction
  rs = ( instr >> 21 ) & 0x1f;
  // rShift 21 bits then bitwise AND with ...0001 1111 to get source field
  rt = ( instr >> 16 ) & 0x1f; 
  // rShift 16 bits then AND with ...0001 1111 to get 2nd source field
  rd = ( instr >> 11 ) & 0x1f;
  // rshift 11 bits then AND with ...0001 1111 to get destination field
  shamt = ( instr >> 6 ) & 0x1f;
  // rshift 6 bits then and with 0001 1111 to get shift amount
  funct = instr & 0x3f;
  // simply AND with 0011 1111 to get least significant six bits
  // used along with opcode to determine the instruction   
  uimm = ( instr & 0x0000ffff );
  // using AND to get least sig. 16 bits for unsigned immediate
  simm = ( instr & 0x0000ffff );
  // using AND to get least sig. 16 bits for signed immediate
  addr = ( instr & 0x03ffffff );
  // using AND to get least 26 bits for address field
  cout << hex << setw(8) << pc << ": ";
  switch(opcode) {
    case 0x00:
      switch(funct) {
        case 0x00: cout << "sll " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt; break; // shift left logical ( zeros shifted in )
        case 0x03: cout << "sra " << regNames[rd] << ", " << regNames[rt] << ", " << dec << shamt; break; // shift right arithmetic ( sign bit shifted in )
        case 0x08: cout << "jr " << regNames[rs]; break; // jump register
        case 0x12: cout << "mflo " << regNames[rd]; break; // move from lo ( where quotient is stored after division )
        case 0x10: cout << "mfhi " << regNames[rd]; break; // move from hi ( where remainder is stored )
        case 0x18: cout << "mult " << regNames[rs] << ", " << regNames[rt]; break; // multiply
        case 0x1a: cout << "div " << regNames[rs] << ", " << regNames[rt]; break; // divide
        case 0x21: cout << "addu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]; break; // add unsigned ( no overflow trap )
        case 0x23: cout << "subu " << regNames[rd] << ", " << regNames[rs]; break; // subtract ( no overflow trap )
        case 0x2a: cout << "slt " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]; break; // set on less than
        default: cout << "unimplemented";
      }
      break;
    case 0x02: cout << "j " << hex << ( ( pc + 4 ) & 0xf0000000 ) + addr * 4; break; // jump
    case 0x03: cout << "jal " << hex << ( ( pc + 4 ) & 0xf0000000 ) + addr * 4; break; // jump and link
    case 0x04: cout << "beq " << regNames[rs] << ", " << regNames[rt] << ", " << hex << ( pc + 4 ) + simm * 4; break; // branch if( equal )
    case 0x05: cout << "bne " << regNames[rs] << ", " << regNames[rt] << ", " << hex << ( pc + 4 ) + simm * 4; break; // branch if( !equal )
    case 0x09: cout << "addiu " << regNames[rt] << ", " << regNames[rs] << ", " << dec << simm; break; // add immediate unsigned
    case 0x0c: cout << "andi " << regNames[rt] << ", " << regNames[rs] << ", " << dec << uimm; break; // and immediate
    case 0x0f: cout << "lui " << regNames[rt] << ", " << dec << simm; break; // load upper immediate
    case 0x1a: cout << "trap " << hex << addr; break; // exception handling
    case 0x23: cout << "lw " << regNames[rt] << ", " << dec << simm << '(' << regNames[rs] << ')'; break; // load word ( from memory )
    case 0x2b: cout << "sw " << regNames[rt] << ", " << dec << simm << '(' << regNames[rs] << ')'; break; // store word ( to memory )
    default: cout << "unimplemented";
  }
  cout << endl;
}

////////////////////////////////////////////
// Swizzle all the bytes of the input word
////////////////////////////////////////////
uint32_t swizzle(uint8_t *bytes) {
  return (bytes[0] << 24) | (bytes[1] << 16) | bytes[2] << 8 | bytes[3];
}

//////////
// MAIN
//////////
int main(int argc, char *argv[]) {
  int count, start;
  ifstream exeFile;
  uint8_t bytes[4];

  uint32_t *instructions;

  cout << "CS 3339 MIPS Disassembler" << endl;
  if(argc != 2) {
    cerr << "usage: " << argv[0] << " mips_executable" << endl;
    return -1;
  }

  // open the executable
  exeFile.open(argv[1], ios::binary | ios::in);
  if(!exeFile) {
    cerr << "error: could not open executable file " << argv[1] << endl;
    return -1;
  }

  // BE->LE swap: Executable files are stored 0A0B0C0D => addr 00,01,02,03
  //              Read into bytes[] as b[0],b[1],b[2],b[3] = 0A,0B,0C,0D
  //              Need to swizzle bytes back into little-endian
  // read # of words in file
  if(!exeFile.read((char *)&bytes, 4)) {
    cerr << "error: could not read count from file " << argv[1] << endl;
    return -1;
  }
  count = swizzle(bytes);
  // read start address from file
  if(!exeFile.read((char *)&bytes, 4)) {
    cerr << "error: could not read start addr from file " << argv[1] << endl;
    return -1;
  }
  start = swizzle(bytes);

  // allocate space and read instructions
  instructions = new uint32_t[count];
  if(!instructions) {
    cerr << "error: out of memory" << endl;
    return -1;
  }
  for(int i = 0; i < count; i++) {
    if(!exeFile.read((char *)&bytes, 4)) {
      cerr << "error: could not read instructions from file " << argv[1] << endl;
      return -1;
    }
    instructions[i] = swizzle(bytes);
  }

  exeFile.close();
  
  // Disassemble
  for(int i = 0; i < count; i++) {
    disassembleInstr(start + i * 4, instructions[i]);
  }
  return 0;
}
