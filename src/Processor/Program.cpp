/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Processor/Program.h"

template<class SRegint, class SBit>
void Program<SRegint, SBit>::compute_constants()
{
  for (int reg_type= 0; reg_type < MAX_REG_TYPE; reg_type++)
    {
      max_reg[reg_type]= 0;
    }
  for (unsigned int i= 0; i < p.size(); i++)
    {
      for (int reg_type= 0; reg_type < MAX_REG_TYPE; reg_type++)
        {
          max_reg[reg_type]= max(max_reg[reg_type], p[i].get_max_reg(RegType(reg_type)));
        }
    }
}

template<class SRegint, class SBit>
void Program<SRegint, SBit>::parse(stringstream &s)
{
  p.resize(0);
  Instruction<SRegint, SBit> instr;
  if (!s.fail())
    {
      s.peek();
      while (!s.eof())
        {
          instr.parse(s);
          p.push_back(instr);
          s.peek();
        }
    }
  compute_constants();
}

template<class SRegint, class SBit>
ostream &operator<<(ostream &s, const Program<SRegint, SBit> &P)
{
  for (unsigned int i= 0; i < P.p.size(); i++)
    {
      s << i << " :: " << P.p[i] << endl;
    }
  return s;
}

#include "Mod2Engine/aBitVector2.h"
#include "OT/aBitVector.h"

template class Program<aBitVector, aBit>;
template class Program<aBitVector2, Share2>;
