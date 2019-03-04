
/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _IOStream
#define _IOStream

/* A simple IO class which just uses standard
 * input/output to communicate values
 *
 * Whereas share values are input/output using
 * a steam, with either human or non-human form
 */

#include "Input_Output_Base.h"

#include <fstream>
using namespace std;

class IO_Stream : public Input_Output_Base
{

public:
  ifstream *inpf;
  ofstream *outf;

  bool human; // Only affects share output

  IO_Stream()
      : Input_Output_Base()
  {
    ;
  }

  void init_streams(ifstream &ifs, ofstream &ofs, bool human_type);

  virtual long open_channel(unsigned int channel) = 0;
  virtual void close_channel(unsigned int channel) = 0;

  virtual gfp private_input_gfp(unsigned int channel) = 0;
  virtual void private_output_gfp(const gfp &output, unsigned int channel) = 0;

  virtual void public_output_gfp(const gfp &output, unsigned int channel) = 0;
  virtual gfp public_input_gfp(unsigned int channel) = 0;

  virtual void public_output_int(const long output, unsigned int channel) = 0;
  virtual long public_input_int(unsigned int channel) = 0;

  virtual void output_share(const Share &S, unsigned int channel) = 0;
  virtual Share input_share(unsigned int channel) = 0;

  virtual void trigger(Schedule &schedule) = 0;

  virtual void debug_output(const stringstream &ss) = 0;

  virtual void crash(unsigned int PC, unsigned int thread_num) = 0;
};

#endif
