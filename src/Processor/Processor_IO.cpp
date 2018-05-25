/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

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
  bool wait= true;
  while (wait)
    {
      OCD.sacrifice_mutex[thread].lock();
      wait= false;
      if (SacrificeD[thread].ID.ios[player].size() < size)
        {
          wait= true;
        }
      OCD.sacrifice_mutex[thread].unlock();
      if (wait)
        {
          sleep(1);
        }
    }
}

void Processor_IO::start_input(unsigned int player, unsigned int n_inputs, unsigned int channel,
                               Processor &Proc, Player &P, Machine &machine,
                               offline_control_data &OCD)
{
  rshares[player].resize(n_inputs);
  if (P.whoami() == player)
    {
      i_epsilon.resize(n_inputs);
    }

  int thread= Proc.get_thread_num();

  IO_Data_Wait(player, n_inputs, thread, OCD);

  // Do this loop outside of Mutex to stop waiting for input stopping queues
  for (unsigned int i= 0; i < n_inputs; i++)
    {
      if (player == P.whoami())
        {
          i_epsilon[i]= machine.IO.private_input_gfp(channel);
        }
    }

  stringstream ss;
  OCD.sacrifice_mutex[thread].lock();
  for (unsigned int i= 0; i < n_inputs; i++)
    {
      rshares[player][i]= SacrificeD[thread].ID.ios[player].front();
      SacrificeD[thread].ID.ios[player].pop_front();
      if (player == P.whoami())
        {
          i_epsilon[i].sub(SacrificeD[thread].ID.opened_ios.front());
          SacrificeD[thread].ID.opened_ios.pop_front();
          i_epsilon[i].output(ss, false);
        }
    }
  OCD.sacrifice_mutex[thread].unlock();
  Proc.increment_counters(n_inputs * Share::SD.M.shares_per_player(P.whoami()));

  if (player == P.whoami())
    {
      P.send_all(ss.str(), false);
    }
}

void Processor_IO::stop_input(unsigned int player, vector<int> targets,
                              Processor &Proc, Player &P)
{
  if (targets.size() != rshares[player].size())
    {
      throw bad_value();
    }
  if (P.whoami() != player)
    {
      string ss;
      P.receive_from_player(player, ss, false);
      stringstream is(ss);
      gfp te;
      for (unsigned int i= 0; i < targets.size(); i++)
        {
          te.input(is, false);
          Proc.get_Sp_ref(targets[i]).add(rshares[player][i], te, P.get_mac_keys());
        }
    }
  else
    {
      for (unsigned int i= 0; i < targets.size(); i++)
        {
          Proc.get_Sp_ref(targets[i])
              .add(rshares[player][i], i_epsilon[i], P.get_mac_keys());
        }
    }
}

void Processor_IO::start_output(unsigned int player, int target, int source,
                                Processor &Proc, Player &P, offline_control_data &OCD)
{
  int thread= Proc.get_thread_num();

  IO_Data_Wait(player, 1, thread, OCD);

  OCD.sacrifice_mutex[thread].lock();

  if (player == P.whoami())
    {
      gfp mask= SacrificeD[thread].ID.opened_ios.front();
      SacrificeD[thread].ID.opened_ios.pop_front();
      o_epsilon.push_back(mask);
    }

  Share share= SacrificeD[thread].ID.ios[player].front();
  SacrificeD[thread].ID.ios[player].pop_front();
  OCD.sacrifice_mutex[thread].unlock();

  Proc.get_Sp_ref(target).add(share, Proc.get_Sp_ref(source));
}

void Processor_IO::stop_output(unsigned int player, int source, unsigned int channel,
                               Processor &Proc, Player &P, Machine &machine)
{
  Proc.RunOpenCheck(P);
  if (player == P.whoami())
    {
      gfp value;
      value.sub(Proc.get_Cp_ref(source), o_epsilon.front());
      o_epsilon.pop_front();
      machine.IO.private_output_gfp(value, channel);
    }
}
