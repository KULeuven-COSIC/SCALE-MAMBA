/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _InputOutputBase
#define _InputOutputBase

/* This class gives an abstract base class which defines
 * how input and output are processed by the MPC engine.
 *
 * It is a virtual base class as anyone using the engine
 * need to re-write EVERY function in this class so as to
 * be able to catch all the read/write messages sent
 * by the MPC engine
 *
 * The derived classes could connect to stdin/stdout,
 * or could connect to files/streams, or could even
 * connect to sockets connecting to other services
 * on the internet.
 *
 * When compiling the system you may want/need to
 * change the precise IO class which is bound into the
 * system in Machine.h/Machine.cpp
 *
 */

#include "LSSS/Share.h"
#include "Online/Schedule.h"
#include <openssl/sha.h>

class Input_Output_Base
{
  // This is a hash object to check public data is enterred
  // consistently between the different players
  //   - This does not give a form of active security
  //     it simply protects against stupid users
  SHA256_CTX sha256;

  void Update_Checker(const stringstream &ss);

protected:
  void Update_Checker(const gfp &input, unsigned int channel);
  void Update_Checker(const long input, unsigned int channel);

public:
  Input_Output_Base();
  virtual ~Input_Output_Base()
  {
  }
  string Get_Check();

  // Open and close channels
  // Channels are assumed to be bidirectional, i.e. we can read and write
  // to them. These calls are provided in case some IO derived classes
  // require explicit opening and closing of channels
  virtual long open_channel(unsigned int channel)= 0;
  virtual void close_channel(unsigned int channel)= 0;

  // Allow a player to enter a gfp element
  virtual gfp private_input_gfp(unsigned int channel)= 0;

  // Allow a player to obtain a private gfp element
  //   - Only known to this player
  virtual void private_output_gfp(const gfp &output, unsigned int channel)= 0;

  // Allow a player to obtain a public gfp element output
  //   - This is a value known to all players
  virtual void public_output_gfp(const gfp &output, unsigned int channel)= 0;

  // Gets a public input gfp element
  //   - System must ensure this element is the same for each player
  virtual gfp public_input_gfp(unsigned int channel)= 0;

  // Allow a player to obtain a public regint element output
  //   - This is a value known to all players
  virtual void public_output_int(const long output, unsigned int channel)= 0;

  // Gets a public input regint element
  //   - System must ensure this element is the same for each player
  virtual long public_input_int(unsigned int channel)= 0;

  // Next two functions need to be used with care, as ill use
  // could break the MPC security model

  // Receive a share value from the engine. This is perhaps
  // for external persistant storage by the player
  virtual void output_share(const Share &S, unsigned int channel)= 0;

  // And allow external system to enter a share value
  virtual Share input_share(unsigned int channel)= 0;

  // Trigger function for restarts
  virtual void trigger(Schedule &schedule)= 0;

  // Debug output data sent here
  virtual void debug_output(const stringstream &ss)= 0;

  // Crash call sent here
  virtual void crash(unsigned int PC, unsigned int thread_num)= 0;
};

#endif
