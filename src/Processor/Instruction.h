/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Instruction
#define _Instruction

/* Class to read and decode an instruction
 */

#include <fstream>
#include <iostream>
#include <vector>
using namespace std;

#include "LSSS/Share.h"
#include "Offline/offline_data.h"
#include "System/Player.h"

class Processor;
class Machine;

/*
 * Opcode constants
 *
 * Whenever these are changed the corresponding dict in
 * Compiler/instructions_base.py
 * MUST also be changed. (+ the documentation)
 */
enum {
  // Load/store
  LDI= 0x1,
  LDSI= 0x2,
  LDMC= 0x3,
  LDMS= 0x4,
  STMC= 0x5,
  STMS= 0x6,
  LDMCI= 0x7,
  LDMSI= 0x8,
  STMCI= 0x9,
  STMSI= 0xA,
  MOVC= 0xB,
  MOVS= 0xC,
  MOVINT= 0xD,
  LDMINT= 0xCA,
  STMINT= 0xCB,
  LDMINTI= 0xCC,
  STMINTI= 0xCD,

  // Machine
  LDTN= 0x10,
  LDARG= 0x11,
  REQBL= 0x12,
  STARG= 0x13,
  CALL= 0x14,
  RETURN= 0x15,
  RUN_TAPE= 0x19,
  JOIN_TAPE= 0x1A,
  CRASH= 0x1B,
  RESTART= 0x1C,
  CLEAR_MEMORY= 0x1D,
  CLEAR_REGISTERS= 0x1E,

  // Addition
  ADDC= 0x20,
  ADDS= 0x21,
  ADDM= 0x22,
  ADDCI= 0x23,
  ADDSI= 0x24,
  SUBC= 0x25,
  SUBS= 0x26,
  SUBML= 0x27,
  SUBMR= 0x28,
  SUBCI= 0x29,
  SUBSI= 0x2A,
  SUBCFI= 0x2B,
  SUBSFI= 0x2C,

  // Multiplication/division/other arithmetic
  MULC= 0x30,
  MULM= 0x31,
  MULCI= 0x32,
  MULSI= 0x33,
  DIVC= 0x34,
  DIVCI= 0x35,
  MODC= 0x36,
  MODCI= 0x37,
  LEGENDREC= 0x38,
  DIGESTC= 0x39,

  // IO
  OUTPUT_CLEAR= 0x40,
  INPUT_CLEAR= 0x41,
  OUTPUT_SHARES= 0x42,
  INPUT_SHARES= 0x43,
  PRIVATE_INPUT= 0x44,
  PRIVATE_OUTPUT= 0x46,
  OUTPUT_INT= 0x48,
  INPUT_INT= 0x49,
  OPEN_CHANNEL= 0x4A,
  CLOSE_CHANNEL= 0x4B,

  // Open
  STARTOPEN= 0xA0,
  STOPOPEN= 0xA1,
  OPENSINT= 0xA2,
  OPENSBIT= 0xA3,

  // Data access
  TRIPLE= 0x50,
  BIT= 0x51,
  SQUARE= 0x52,
  DABIT= 0x53,

  // sregint/sbit instructions
  LDMSINT= 0x60,
  LDMSINTI= 0x61,
  STMSINT= 0x62,
  STMSINTI= 0x63,
  MOVSINT= 0x64,
  LDSINT= 0x65,
  ADDSINT= 0x66,
  ADDSINTC= 0x67,
  SUBSINT= 0x68,
  SUBSINTC= 0x69,
  SUBCINTS= 0x6A,
  MULSINT= 0x6B,
  MULSINTC= 0x6C,
  DIVSINT= 0x6D,
  SHLSINT= 0x6E,
  SHRSINT= 0x6F,

  // Bitwise logic
  ANDC= 0x70,
  XORC= 0x71,
  ORC= 0x72,
  ANDCI= 0x73,
  XORCI= 0x74,
  ORCI= 0x75,
  NOTC= 0x76,
  NEG= 0x77,
  SAND= 0x78,
  XORSB= 0x79,
  ANDSB= 0x7A,
  ORSB= 0x7B,
  NEGB= 0x7C,
  LDSBIT= 0x7D,

  // Bitwise shifts
  SHLC= 0x80,
  SHRC= 0x81,
  SHLCI= 0x82,
  SHRCI= 0x83,

  // Branching and comparison
  JMP= 0x90,
  JMPNZ= 0x91,
  JMPEQZ= 0x92,
  EQZINT= 0x93,
  LTZINT= 0x94,
  LTINT= 0x95,
  GTINT= 0x96,
  EQINT= 0x97,

  // Integers
  LDINT= 0x9A,
  ADDINT= 0x9B,
  SUBINT= 0x9C,
  MULINT= 0x9D,
  DIVINT= 0x9E,

  // Conversion
  CONVINT= 0xC0,
  CONVMODP= 0xC1,
  CONVSINTSREG= 0xC2,
  CONVREGSREG= 0xC3,
  CONVSREGSINT= 0xC4,
  CONVSUREGSINT= 0xC5,

  // Debug Printing
  PRINT_MEM= 0xB0,
  PRINT_REG= 0xB2,
  PRINT_CHAR= 0xB3,
  PRINT_CHAR4= 0xB4,
  PRINT_CHAR_REGINT= 0xB5,
  PRINT_CHAR4_REGINT= 0xB6,
  PRINT_FLOAT= 0xB7,
  PRINT_FIX= 0xB8,
  PRINT_INT= 0xB9,
  PRINT_IEEE_FLOAT= 0xBA,

  // Comparison of sregints
  EQZSINT= 0xD0,
  LTZSINT= 0xD1,

  // Bitwise logical operations on sregints
  ANDSINT= 0xD3,
  ANDSINTC= 0xD4,
  ORSINT= 0xD5,
  ORSINTC= 0xD6,
  XORSINT= 0xD7,
  XORSINTC= 0xD8,
  INVSINT= 0xD9,

  // 64x64 -> 128 multiplier
  MUL2SINT= 0xDA,

  // GC (and allied) commands
  GC= 0xDB,
  BITSINT= 0xDC,
  SINTBIT= 0xDD,
  LF= 0xDE,

  // Others
  RAND= 0xE0,
  START_CLOCK= 0xE1,
  STOP_CLOCK= 0xE2,

  // Stack operations
  PUSHINT= 0x100,
  POPINT= 0x101,
  PEEKINT= 0x102,
  POKEINT= 0x103,
  GETSPINT= 0x104,
  PUSHSINT= 0x105,
  POPSINT= 0x106,
  PEEKSINT= 0x107,
  POKESINT= 0x108,
  GETSPSINT= 0x109,
  PUSHSBIT= 0x10A,
  POPSBIT= 0x10B,
  PEEKSBIT= 0x10C,
  POKESBIT= 0x10D,
  GETSPSBIT= 0x10E,
  PUSHC= 0x110,
  POPC= 0x111,
  PEEKC= 0x112,
  POKEC= 0x113,
  GETSPC= 0x114,
  PUSHS= 0x115,
  POPS= 0x116,
  PEEKS= 0x117,
  POKES= 0x118,
  GETSPS= 0x119,
};

// Register types
enum RegType {
  MODP,
  INT,
  SBIT,
  DUALBIT,
  MAX_REG_TYPE,
  DUAL,
  NONE
};

enum SecrecyType {
  SECRET,
  CLEAR,
  MAX_SECRECY_TYPE
};

class BaseInstruction
{
protected:
  int opcode;        // The code
  unsigned int size; // Vector size
  int r[4];          // Fixed parameter registers
  unsigned int p;    // Player value of argument
  unsigned int n;    // Possible immediate value
  unsigned int m;    // Possible immediate value
  vector<int> start; // Values for a start/stop open

public:
  virtual ~BaseInstruction(){};

  void parse_operands(istream &s, int pos);

  virtual RegType get_reg_type() const;

  // Returns the maximal register used
  int get_max_reg(RegType reg_type) const;
};

class Instruction : public BaseInstruction
{
  void execute_using_sacrifice_data(Processor &Proc,
                                    offline_control_data &OCD) const;

public:
  // Reads a single instruction from the istream
  void parse(istream &s);

  friend ostream &operator<<(ostream &s, const Instruction &instr);

  // Execute this instruction, updateing the processor and memory
  //  Returns true if we should execute a restart
  bool execute(Processor &Proc, Player &P, Machine &machine,
               offline_control_data &OCD) const;
};

#endif
