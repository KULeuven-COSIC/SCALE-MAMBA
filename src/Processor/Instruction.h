/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

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
  LDMINT= 0xCA,
  STMINT= 0xCB,
  LDMINTI= 0xCC,
  STMINTI= 0xCD,
  PUSHINT= 0xCE,
  POPINT= 0xCF,
  MOVINT= 0xD0,

  // Machine
  LDTN= 0x10,
  LDARG= 0x11,
  REQBL= 0x12,
  STARG= 0x13,
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
  OUTPUT_SHARE= 0x42,
  INPUT_SHARE= 0x43,
  PRIVATE_INPUT= 0x44,
  PRIVATE_OUTPUT= 0x46,
  OUTPUT_INT= 0x48,
  INPUT_INT= 0x49,
  OPEN_CHAN= 0x4A,
  CLOSE_CHAN= 0x4B,

  // Open
  STARTOPEN= 0xA0,
  STOPOPEN= 0xA1,

  // Data access
  TRIPLE= 0x50,
  BIT= 0x51,
  SQUARE= 0x52,

  // Bitwise logic
  ANDC= 0x70,
  XORC= 0x71,
  ORC= 0x72,
  ANDCI= 0x73,
  XORCI= 0x74,
  ORCI= 0x75,
  NOTC= 0x76,

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
  JMPI= 0x98,

  // Integers
  LDINT= 0x9A,
  ADDINT= 0x9B,
  SUBINT= 0x9C,
  MULINT= 0x9D,
  DIVINT= 0x9E,

  // Conversion
  CONVINT= 0xC0,
  CONVMODP= 0xC1,

  // Debug Printing
  PRINTMEM= 0xB0,
  PRINTREG= 0XB1,
  PRINTREGPLAIN= 0xB2,
  PRINTCHR= 0xB3,
  PRINTSTR= 0xB4,
  PRINTCHRINT= 0xB5,
  PRINTSTRINT= 0xB6,
  PRINTFLOATPLAIN= 0xB7,
  PRINTFIXPLAIN= 0xB8,
  PRINTINT= 0xB9,

  // Others
  RAND= 0xE0,
  START_TIMER= 0xE1,
  STOP_TIMER= 0xE2
};

// Register types
enum RegType {
  MODP,
  INT,
  MAX_REG_TYPE,
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

  virtual int get_reg_type() const;

  bool is_direct_memory_access(SecrecyType sec_type) const;

  // Returns the maximal register used
  int get_max_reg(int reg_type) const;
};

class Instruction : public BaseInstruction
{
  void execute_using_sacrifice_data(Processor &Proc,
                                    offline_control_data &OCD) const;

public:
  // Reads a single instruction from the istream
  void parse(istream &s);

  // Returns the memory size used if applicable and known
  int get_mem(RegType reg_type, SecrecyType sec_type) const;

  friend ostream &operator<<(ostream &s, const Instruction &instr);

  // Execute this instruction, updateing the processor and memory
  //  Returns true if we should execute a restart
  bool execute(Processor &Proc, Player &P, Machine &machine,
               offline_control_data &OCD) const;
};

#endif
