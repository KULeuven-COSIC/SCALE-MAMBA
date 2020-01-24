/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#pragma once

/* A simple IO class which just uses standard
 * input/output to communicate values
 *
 * Whereas share values are input/output using
 * a stream, with either human or non-human form
 */

#include <fstream>
#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>

#include "Input_Output_Base.h"

class Input_Output_File : public Input_Output_Base {
 public:
  Input_Output_File(
      std::istream* ifs,
      std::ostream* ofs,
      bool human_type,
      std::string& datadir,
      int player_number) : Input_Output_Base(),
                           _inpf{ifs},
                           _outf{ofs},
                           _human{human_type},
                           _datadir{datadir},
                           _player_number{player_number} {}

  virtual long open_channel(unsigned int channel);
  virtual void close_channel(unsigned int channel);

  virtual gfp private_input_gfp(unsigned int channel);
  virtual void private_output_gfp(const gfp &output, unsigned int channel);

  virtual void public_output_gfp(const gfp &output, unsigned int channel);
  virtual gfp public_input_gfp(unsigned int channel);

  virtual void public_output_int(const long output, unsigned int channel);
  virtual long public_input_int(unsigned int channel);

  virtual void output_share(const Share &S, unsigned int channel);
  virtual Share input_share(unsigned int channel);

  virtual void trigger(Schedule &schedule);

  virtual void debug_output(const stringstream &ss);

  virtual void crash(unsigned int PC, unsigned int thread_num);

 private:
  std::istream* _inpf;
  std::ostream* _outf;
  // Only affects share output
  bool _human;
  std::string& _datadir;
  int _player_number;

  std::unordered_map<int, std::unique_ptr<ifstream>> _private_gfp_ifiles;
  std::unordered_map<int, std::unique_ptr<ofstream>> _private_gfp_ofiles;
  std::unordered_map<int, std::unique_ptr<ifstream>> _public_gfp_ifiles;
  std::unordered_map<int, std::unique_ptr<ofstream>> _public_gfp_ofiles;
  std::unordered_map<int, std::unique_ptr<ifstream>> _public_int_ifiles;
  std::unordered_map<int, std::unique_ptr<ofstream>> _public_int_ofiles;
  std::unordered_map<int, std::unique_ptr<ifstream>> _share_ifiles;
  std::unordered_map<int, std::unique_ptr<ofstream>> _share_ofiles;

  std::string open_channel_filename_helper(
      unsigned int channel, const std::string& fn);
};
