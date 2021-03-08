/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _InputOutputStore
#define _InputOutputStore

/* A simple IO class which just forwards all operations to
   C or Rust code via FFI.
 */

#include "Input_Output_Base.h"

struct ScaleIoHandle;

extern "C" int scale_io_open_channel(ScaleIoHandle *handle,
                                     unsigned int channel);

extern "C" int scale_io_close_channel(ScaleIoHandle *handle,
                                      unsigned int channel);

extern "C" unsigned int scale_io_close(ScaleIoHandle *handle);

extern "C" unsigned int scale_io_private_input_gfp(char *result, ScaleIoHandle *handle,
                                                   unsigned int channel);

extern "C" unsigned int scale_io_private_output_gfp(ScaleIoHandle *handle, unsigned int channel,
                                                    const char *value);

extern "C" unsigned int scale_io_public_input_gfp(char *result, ScaleIoHandle *handle,
                                                  unsigned int channel);

extern "C" unsigned int scale_io_public_output_gfp(ScaleIoHandle *handle,
                                                   unsigned int channel,
                                                   const char *value);

extern "C" unsigned int scale_io_public_input_int(long *result, ScaleIoHandle *handle,
                                                  unsigned int channel);

extern "C" unsigned int scale_io_public_output_int(ScaleIoHandle *handle, unsigned int channel,
                                                   long value);

extern "C" unsigned int scale_io_input_share(char *result,
                                             ScaleIoHandle *handle,
                                             unsigned int channel);

extern "C" unsigned int scale_io_trigger(char *result,
                                         ScaleIoHandle *handle);

extern "C" unsigned int scale_io_debug_output(const char *data,
                                              ScaleIoHandle *handle);

extern "C" unsigned int scale_io_output_share(ScaleIoHandle *handle,
                                              unsigned int channel,
                                              const char *value);

class Input_Output_C : public Input_Output_Base
{

private:
  ScaleIoHandle *handle;

public:
  Input_Output_C();

  virtual ~Input_Output_C();

  void init(ScaleIoHandle *handle);

  long open_channel(unsigned int channel) override;

  void close_channel(unsigned int channel) override;

  gfp private_input_gfp(unsigned int channel) override;

  void private_output_gfp(const gfp &output, unsigned int channel) override;

  void public_output_gfp(const gfp &output, unsigned int channel) override;

  gfp public_input_gfp(unsigned int channel) override;

  void public_output_int(long output, unsigned int channel) override;

  long public_input_int(unsigned int channel) override;

  void output_share(const Share &S, unsigned int channel) override;

  Share input_share(unsigned int channel) override;

  void trigger(Schedule &schedule) override;

  void debug_output(const stringstream &ss) override;

  void crash(unsigned int PC, unsigned int thread_num) override;
};

#endif
