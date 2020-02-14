/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Processor_IO.h"
#include "Processor.h"

#include <unistd.h>

extern vector<sacrificed_data> SacrificeD;

// Check to see if we have to wait
void IO_Data_Wait(unsigned int player, unsigned int size, int thread,
                  offline_control_data &OCD)
{
  Wait_For_Preproc(DATA_INPUT_MASK, size, thread, OCD, player);
}

void Processor_IO::private_input(unsigned int player, int target, unsigned int channel,
                                 Processor &Proc, Player &P, Machine &machine,
                                 offline_control_data &OCD)
{
  gfp i_epsilon;
  int thread= Proc.get_thread_num();

  Wait_For_Preproc(DATA_INPUT_MASK, 1, thread, OCD, player);

  if (player == P.whoami())
    {
      i_epsilon= machine.get_IO().private_input_gfp(channel);
    }

  stringstream ss;
  OCD.OCD_mutex[thread].lock();
  rshares[player]= SacrificeD[thread].ID.ios[player].front();
  SacrificeD[thread].ID.ios[player].pop_front();
  if (player == P.whoami())
    {
      i_epsilon.sub(SacrificeD[thread].ID.opened_ios.front());
      SacrificeD[thread].ID.opened_ios.pop_front();
      i_epsilon.output(ss, false);
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
      te.input(is, false);
      Proc.get_Sp_ref(target).add(rshares[player], te, P.get_mac_keys());
    }

  if (player == P.whoami())
    {
      Proc.get_Sp_ref(target).add(rshares[player], i_epsilon, P.get_mac_keys());
    }
}

void Processor_IO::private_output(unsigned int player, int source, unsigned int channel,
                                  Processor &Proc, Player &P,
                                  Machine &machine,
                                  offline_control_data &OCD)
{
  int thread= Proc.get_thread_num();
  Wait_For_Preproc(DATA_INPUT_MASK, 1, thread, OCD, player);

  OCD.OCD_mutex[thread].lock();

  gfp o_epsilon;

  if (player == P.whoami())
    {
      o_epsilon= SacrificeD[thread].ID.opened_ios.front();
      SacrificeD[thread].ID.opened_ios.pop_front();
    }

  vector<Share> shares(1);
  vector<gfp> values(1);
  shares[0]= SacrificeD[thread].ID.ios[player].front();
  SacrificeD[thread].ID.ios[player].pop_front();
  OCD.OCD_mutex[thread].unlock();

  shares[0].add(Proc.get_Sp_ref(source));

  // Via Channel one to avoid conflicts with START/STOP Opens
  Proc.Open_To_All_Begin(values, shares, P, 1);
  Proc.Open_To_All_End(values, shares, P, 1);

  // Guaranteed to be in online thread zero
  Proc.RunOpenCheck(P, machine.get_IO().Get_Check(), 1);
  Proc.RunOpenCheck(P, machine.get_IO().Get_Check(), 2);
  if (player == P.whoami())
    {
      values[0].sub(o_epsilon);
      machine.get_IO().private_output_gfp(values[0], channel);
    }
}
