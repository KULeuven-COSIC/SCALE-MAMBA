/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Processor_IO.h"
#include "Processor.h"

#include <unistd.h>

extern vector<sacrificed_data> SacrificeD;

void Processor_IO::private_input(unsigned int player, int target, unsigned int channel,
                                 Processor &Proc, Player &P, Machine &machine,
                                 offline_control_data &OCD,
                                 unsigned int vectorize)
{
  int thread= Proc.get_thread_num();

  Wait_For_Preproc(DATA_INPUT_MASK, vectorize, thread, OCD, player);

  vector<gfp> i_epsilon(vectorize);

  if (player == P.whoami())
    {
      for (unsigned int i= 0; i < vectorize; i++)
        {
          i_epsilon[i]= machine.get_IO().private_input_gfp(channel);
        }
    }

  stringstream ss;
  rshares[player].resize(vectorize);

  OCD.OCD_mutex[thread].lock();

  for (unsigned int i= 0; i < vectorize; i++)
    {
      rshares[player][i]= SacrificeD[thread].ID.ios[player].front();
      SacrificeD[thread].ID.ios[player].pop_front();
    }

  if (player == P.whoami())
    {
      for (unsigned int i= 0; i < vectorize; i++)
        {
          i_epsilon[i].sub(SacrificeD[thread].ID.opened_ios.front());
          SacrificeD[thread].ID.opened_ios.pop_front();
          i_epsilon[i].output(ss, false);
        }
    }
  OCD.OCD_mutex[thread].unlock();
  Proc.increment_counters(Share::SD.M.shares_per_player(P.whoami()));

  if (player == P.whoami())
    {
      // Send via Channel 1 to ensure does not conflict with START/STOP OPENs
      P.send_all(ss.str(), 1, false);
    }
  else
    {
      // Receive via Channel 1
      string ss;
      P.receive_from_player(player, ss, 1, false);
      stringstream is(ss);
      gfp te;
      for (unsigned int i= 0; i < vectorize; i++)
        {
          te.input(is, false);
          Proc.get_Sp_ref(target + i).add(rshares[player][i], te, P.get_mac_keys());
        }
    }

  if (player == P.whoami())
    {
      for (unsigned int i= 0; i < vectorize; i++)
        {
          Proc.get_Sp_ref(target + i).add(rshares[player][i], i_epsilon[i], P.get_mac_keys());
        }
    }
}

void Processor_IO::private_output(unsigned int player, int source, unsigned int channel,
                                  Processor &Proc, Player &P,
                                  Machine &machine,
                                  offline_control_data &OCD,
                                  unsigned int vectorize)
{
  int thread= Proc.get_thread_num();
  Wait_For_Preproc(DATA_INPUT_MASK, vectorize, thread, OCD, player);

  OCD.OCD_mutex[thread].lock();

  vector<gfp> o_epsilon(vectorize);

  if (player == P.whoami())
    {
      for (unsigned int i= 0; i < vectorize; i++)
        {
          o_epsilon[i]= SacrificeD[thread].ID.opened_ios.front();
          SacrificeD[thread].ID.opened_ios.pop_front();
        }
    }

  vector<Share> shares(vectorize);
  vector<gfp> values(vectorize);
  for (unsigned int i= 0; i < vectorize; i++)
    {
      shares[i]= SacrificeD[thread].ID.ios[player].front();
      SacrificeD[thread].ID.ios[player].pop_front();
    }

  OCD.OCD_mutex[thread].unlock();

  for (unsigned int i= 0; i < vectorize; i++)
    {
      shares[i].add(Proc.get_Sp_ref(source + i));
    }

  // Via Channel one to avoid conflicts with START/STOP Opens
  Proc.Open_To_All_Begin(values, shares, P, 1);
  Proc.Open_To_All_End(values, shares, P, 1);

  // Guaranteed to be in online thread zero
  Proc.RunOpenCheck(P, machine.get_IO().Get_Check(), 1);
  Proc.RunOpenCheck(P, machine.get_IO().Get_Check(), 2);
  if (player == P.whoami())
    {
      for (unsigned int i= 0; i < vectorize; i++)
        {
          values[i].sub(o_epsilon[i]);
          machine.get_IO().private_output_gfp(values[i], channel);
        }
    }
}
