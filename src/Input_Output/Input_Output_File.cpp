/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "Input_Output_File.h"
#include "Exceptions/Exceptions.h"

std::string Input_Output_File::open_channel_filename_helper(
    unsigned int channel,
    const std::string& fn) {
  std::stringstream s;
  s << _datadir << "/" << fn << "_" << channel << ".txt";
  return std::move(s.str());
}

long Input_Output_File::open_channel(unsigned int channel) {
  *_outf << "Player " << _player_number << " Opening Channel "
        << channel << std::endl;

  // Private GFP IO
  auto private_infile_gfp_name =
      open_channel_filename_helper(channel, "private_input_gfp");
  *_outf << "Private GFP Infile=" << private_infile_gfp_name << std::endl;

  auto private_outfile_gfp_name =
      open_channel_filename_helper(channel, "private_output_gfp");
  *_outf << "Private GFP Outfile=" << private_outfile_gfp_name << std::endl;

  _private_gfp_ifiles[channel] =
    std::make_unique<std::ifstream>(private_infile_gfp_name);
  _private_gfp_ofiles[channel] =
    std::make_unique<std::ofstream>(private_outfile_gfp_name);

  //this snippet always sets the fail bit to zero on file open
  // so we don't fail immediately if a file doesn't exist
  // get state
  auto private_gfp_state = _private_gfp_ifiles[channel]->rdstate();
  // remove failbit from it
  private_gfp_state &= ~std::ifstream::failbit;
  // clear old state and set new state
  _private_gfp_ifiles[channel]->clear(private_gfp_state);

  // these force exceptions if there is a problem with the file stream
  _private_gfp_ifiles[channel]->exceptions(std::ifstream::failbit);
  _private_gfp_ofiles[channel]->exceptions(std::ofstream::failbit);


  // Public GFP IO
  auto public_infile_gfp_name =
      open_channel_filename_helper(channel, "public_input_gfp");
  *_outf << "Public GFP Infile=" << public_infile_gfp_name << std::endl;

  auto public_outfile_gfp_name =
      open_channel_filename_helper(channel, "public_output_gfp");
  *_outf << "Public GFP Outfile=" << public_outfile_gfp_name << std::endl;

  _public_gfp_ifiles[channel] =
    std::make_unique<std::ifstream>(public_infile_gfp_name);
  _public_gfp_ofiles[channel] =
    std::make_unique<std::ofstream>(public_outfile_gfp_name);

  // get state
  auto public_gfp_state = _public_gfp_ifiles[channel]->rdstate();
  // remove failbit from it
  public_gfp_state &= ~std::ifstream::failbit;
  // clear old state and set new state
  _public_gfp_ifiles[channel]->clear(public_gfp_state);

  _public_gfp_ifiles[channel]->exceptions(std::ifstream::failbit);
  _public_gfp_ofiles[channel]->exceptions(std::ofstream::failbit);


  // Public Int IO
  auto public_infile_int_name =
      open_channel_filename_helper(channel, "public_input_int");
  *_outf << "Public Int Infile=" << public_infile_int_name << std::endl;

  auto public_outfile_int_name =
      open_channel_filename_helper(channel, "public_output_int");
  *_outf << "Public Int Outfile=" << public_outfile_int_name << std::endl;

  _public_int_ifiles[channel] =
      std::make_unique<std::ifstream>(public_infile_int_name);
  _public_int_ofiles[channel] =
      std::make_unique<std::ofstream>(public_outfile_int_name);

  // get state
  auto public_int_state = _public_int_ifiles[channel]->rdstate();
  // remove failbit from it
  public_int_state &= ~std::ifstream::failbit;
  // clear old state and set new state
  _public_int_ifiles[channel]->clear(public_int_state);

  _public_int_ifiles[channel]->exceptions(std::ifstream::failbit);
  _public_int_ofiles[channel]->exceptions(std::ofstream::failbit);

  // Share IO
  auto share_infile_name = open_channel_filename_helper(channel, "share_input");
  *_outf << "Share Infile=" << share_infile_name << std::endl;

  auto share_outfile_name = open_channel_filename_helper(channel, "share_output");
  *_outf << "Share Outfile=" << share_outfile_name << std::endl;

  _share_ifiles[channel] = std::make_unique<std::ifstream>(share_infile_name);
  _share_ofiles[channel] = std::make_unique<std::ofstream>(share_outfile_name);

  // get state
  auto share_state = _share_ifiles[channel]->rdstate();
  // remove failbit from it
  share_state &= ~std::ifstream::failbit;
  // clear old state and set new state
  _share_ifiles[channel]->clear(share_state);

  _share_ifiles[channel]->exceptions(std::ifstream::failbit);
  _share_ofiles[channel]->exceptions(std::ofstream::failbit);

  return 0;
}

void Input_Output_File::close_channel(unsigned int channel) {
  *_outf << "Closing Channel " << channel << std::endl;
  _private_gfp_ifiles.erase(channel);
  _private_gfp_ofiles.erase(channel);

  _public_gfp_ifiles.erase(channel);
  _public_gfp_ofiles.erase(channel);

  _public_int_ifiles.erase(channel);
  _public_int_ofiles.erase(channel);

  _share_ifiles.erase(channel);
  _share_ofiles.erase(channel);
}

gfp Input_Output_File::private_input_gfp(unsigned int channel) {
  word x;
  *_private_gfp_ifiles[channel] >> x;
  gfp y;
  y.assign(x);
  return y;
}

void Input_Output_File::private_output_gfp(
    const gfp &output, unsigned int channel) {
  *_private_gfp_ofiles[channel] << output << std::endl;
}

gfp Input_Output_File::public_input_gfp(unsigned int channel) {
  word x;
  *_public_gfp_ifiles[channel] >> x;
  gfp y;
  y.assign(x);
  return y;

  // Important to have this call in each version of public_input_gfp
  Update_Checker(y, channel);

  return y;
}

void Input_Output_File::public_output_gfp(
    const gfp &output, unsigned int channel) {
  *_public_gfp_ofiles[channel] << output << std::endl;
}

long Input_Output_File::public_input_int(unsigned int channel) {
  long x;
  *_public_int_ifiles[channel] >> x;
  // Important to have this call in each version of public_input_gfp
  Update_Checker(x, channel);

  return x;
}

void Input_Output_File::public_output_int(
    const long output, unsigned int channel) {
  *_public_int_ofiles[channel] << output << std::endl;
}

void Input_Output_File::output_share(const Share &S, unsigned int channel) {
  S.output(*_share_ofiles[channel], _human);
}

Share Input_Output_File::input_share(unsigned int channel) {
  Share S;
  S.input(*_share_ifiles[channel], _human);
  return S;
}

void Input_Output_File::trigger(Schedule &schedule) {
  std::cout << "Restart requested: Enter a number to proceed" << std::endl;
  int i;
  *_inpf >> i;

  // Load new schedule file program streams, using the original
  // program name
  //
  // Here you could define programatically what the new
  // programs you want to run are, by directly editing the
  // public variables in the schedule object.
  unsigned int nthreads = schedule.Load_Programs();
  if (schedule.max_n_threads() < nthreads) {
    throw Processor_Error("Restart requires more threads, cannot do this");
  }
}

void Input_Output_File::debug_output(const stringstream &ss) {
  std::cout << ss.str() << std::flush;
}

void Input_Output_File::crash(unsigned int PC, unsigned int thread_num) {
  std::cout <<
    "Crashing in thread " << thread_num << "at PC value " << PC << std::endl;
  throw crash_requested();
}
