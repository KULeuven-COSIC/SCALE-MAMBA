/*
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

//
// Created by Bruno Grieder on 2019-04-17.
//

#include "Input_Output_C.h"

extern "C" Input_Output_Base *create_input_output_base_rust(ScaleIoHandle *handle)
{
  auto ptr= new Input_Output_C();
  ptr->init(handle);
  return ptr;
}

Input_Output_C::Input_Output_C()
    : Input_Output_Base(), handle(0)
{
}

Input_Output_C::~Input_Output_C()
{
  scale_io_close(this->handle);
}

void Input_Output_C::init(ScaleIoHandle *handle)
{
  this->handle= handle;
}

long Input_Output_C::open_channel(unsigned int channel)
{
  auto ret= scale_io_open_channel(this->handle, channel);
  if (ret != 0)
    {
      throw std::invalid_argument("failed opening channel: " +
                                  std::to_string(channel) + ". Check the logs");
    }
  return ret;
}

void Input_Output_C::close_channel(unsigned int channel)
{
  auto ret= scale_io_close_channel(this->handle, channel);
  if (ret != 0)
    {
      throw std::invalid_argument("failed closing channel: " +
                                  std::to_string(channel) + ". Check the logs");
    }
}

gfp Input_Output_C::private_input_gfp(unsigned int channel)
{
  char c[512];
  auto ret= scale_io_private_input_gfp(c, this->handle, channel);
  if (ret != 0)
    {
      throw std::invalid_argument("failed reading private input on channel: " +
                                  std::to_string(channel) + ". Check the logs");
    }
  gfp y;
  auto bi= bigint(c);
  y.assign(bi);
  return y;
}

void Input_Output_C::private_output_gfp(const gfp &output,
                                        unsigned int channel)
{
  std::stringstream ss;
  output.output(ss, true);
  auto s= ss.str();
  if (scale_io_private_output_gfp(handle, channel, s.data()) != 0)
    {
      throw std::invalid_argument("failed writing private output on channel: " +
                                  std::to_string(channel) + ". Check the logs");
    }
}

void Input_Output_C::public_output_gfp(const gfp &output,
                                       unsigned int channel)
{
  std::stringstream ss;
  output.output(ss, true);
  auto s= ss.str();
  if (scale_io_public_output_gfp(handle, channel, s.data()) != 0)
    {
      throw std::invalid_argument("failed writing public output on channel: " +
                                  std::to_string(channel) + ". Check the logs");
    }
}

gfp Input_Output_C::public_input_gfp(unsigned int channel)
{
  char c[512];
  auto ret= scale_io_public_input_gfp(c, this->handle, channel);
  if (ret != 0)
    {
      throw std::invalid_argument("failed reading public input on channel: " +
                                  std::to_string(channel) + ". Check the logs");
    }
  gfp y;
  auto bi= bigint(c);
  y.assign(bi);
  return y;
}

void Input_Output_C::public_output_int(long output, unsigned int channel)
{
  if (scale_io_public_output_int(handle, channel, output) != 0)
    {
      throw std::invalid_argument("failed writing public int on channel: " +
                                  std::to_string(channel) + ". Check the logs");
    }
}

long Input_Output_C::public_input_int(unsigned int channel)
{
  long res;
  auto ret= scale_io_public_input_int(&res, this->handle, channel);
  if (ret != 0)
    {
      throw std::invalid_argument("failed reading public int on channel: " +
                                  std::to_string(channel) + ". Check the logs");
    }
  return res;
}

void Input_Output_C::output_share(const Share &S, unsigned int channel)
{
  std::stringstream ss;
  S.output(ss, true);
  auto str= ss.str();
  if (scale_io_output_share(handle, channel, str.data()) != 0)
    {
      throw std::invalid_argument("failed writing share on channel: " +
                                  std::to_string(channel) + ". Check the logs");
    }
}

Share Input_Output_C::input_share(unsigned int channel)
{
  char c[512];
  auto ret= scale_io_input_share(c, this->handle, channel);
  if (ret != 0)
    {
      throw std::invalid_argument("failed reading share on channel: " +
                                  std::to_string(channel) + ". Check the logs");
    }
  stringstream ss(c);
  Share S;
  S.input(ss, false);
  return S;
}

void Input_Output_C::trigger(Schedule &schedule)
{
  char c[512];
  auto ret= scale_io_trigger(c, this->handle);
  if (ret != 0)
    {
      throw std::invalid_argument("failed reading fetching next program. Check the logs");
    }
  unsigned int nthreads= schedule.Load_Programs(c);
  if (schedule.max_n_threads() < nthreads)
    {
      throw std::invalid_argument("Restart requires more threads, cannot do this");
    }
}

void Input_Output_C::debug_output(const stringstream &ss)
{
  auto s= ss.str();
  scale_io_debug_output(s.data(), this->handle);
}

void Input_Output_C::crash(unsigned int PC, unsigned int thread_num)
{
  std::cout << "Crashing in thread " << thread_num << " at PC value " << PC << std::endl;
  throw crash_requested();
}
