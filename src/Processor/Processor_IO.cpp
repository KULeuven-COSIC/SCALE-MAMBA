/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Processor_IO.h"
#include "Processor.h"

#include <unistd.h>

extern vector<sacrificed_data> SacrificeD;

template<class SRegint, class SBit>
void Processor_IO<SRegint, SBit>::Common_Subroutine_I(unsigned int player, unsigned int channel,
                                                      unsigned int number,
                                                      Processor<SRegint, SBit> &Proc, Player &P,
                                                      Machine<SRegint, SBit> &machine,
                                                      offline_control_data &OCD)
{
  int thread= Proc.get_thread_num();

  Wait_For_Preproc(DATA_INPUT_MASK, number, thread, OCD, player);

  epsilon.resize(number);
  rshares.resize(number);

  if (player == P.whoami())
    {
      for (unsigned int i= 0; i < number; i++)
        {
          epsilon[i]= machine.get_IO().private_input_gfp(channel);
        }
    }

  stringstream ss;

  OCD.OCD_mutex[thread].lock();

  for (unsigned int i= 0; i < number; i++)
    {
      rshares[i]= SacrificeD[thread].ID.ios[player].front();
      SacrificeD[thread].ID.ios[player].pop_front();
    }

  if (player == P.whoami())
    {
      for (unsigned int i= 0; i < number; i++)
        {
          epsilon[i].sub(SacrificeD[thread].ID.opened_ios.front());
          SacrificeD[thread].ID.opened_ios.pop_front();
          epsilon[i].output(ss, false);
        }
    }
  OCD.OCD_mutex[thread].unlock();
#ifdef BENCH_OFFLINE
  P.inputs+= number;
#endif
  Proc.increment_counters(Share::SD.M.shares_per_player(P.whoami()));

  if (player == P.whoami())
    {
      // Send via Channel 1 to ensure does not conflict with START/STOP OPENs
      P.send_all(ss.str(), 1, false);
    }
}

template<class SRegint, class SBit>
void Processor_IO<SRegint, SBit>::private_input(
    unsigned int player, int target, unsigned int channel,
    Processor<SRegint, SBit> &Proc, Player &P,
    Machine<SRegint, SBit> &machine,
    offline_control_data &OCD,
    unsigned int vectorize)
{

  Common_Subroutine_I(player, channel, vectorize, Proc, P, machine, OCD);

  if (player != P.whoami())
    {
      // Receive via Channel 1
      string ss;
      P.receive_from_player(player, ss, 1, false);
      stringstream is(ss);
      gfp te;
      for (unsigned int i= 0; i < vectorize; i++)
        {
          te.input(is, false);
          Proc.get_Sp_ref(target + i).add(rshares[i], te, P.get_mac_keys());
        }
    }
  else
    {
      for (unsigned int i= 0; i < vectorize; i++)
        {
          Proc.get_Sp_ref(target + i).add(rshares[i], epsilon[i], P.get_mac_keys());
        }
    }
}

template<class SRegint, class SBit>
void Processor_IO<SRegint, SBit>::mprivate_input(
    unsigned int player, int target, unsigned int channel,
    unsigned int n,
    Processor<SRegint, SBit> &Proc, Player &P,
    Machine<SRegint, SBit> &machine,
    offline_control_data &OCD,
    unsigned int vectorize)
{

  Common_Subroutine_I(player, channel, n * vectorize, Proc, P, machine, OCD);

  Share S;
  if (player != P.whoami())
    {
      // Receive via Channel 1
      string ss;
      P.receive_from_player(player, ss, 1, false);
      stringstream is(ss);
      gfp te;
      unsigned int cnt= 0;
      for (unsigned int i= 0; i < vectorize; i++)
        {
          long reg= Proc.read_Ri(target + i);
          for (unsigned int j= 0; j < n; j++)
            {
              te.input(is, false);
              S.add(rshares[cnt], te, P.get_mac_keys());
              machine.Ms.write(reg + j, S, machine.verbose, Proc.get_PC());
              cnt++;
            }
        }
    }
  else
    {
      unsigned int cnt= 0;
      for (unsigned int i= 0; i < vectorize; i++)
        {
          long reg= Proc.read_Ri(target + i);
          for (unsigned int j= 0; j < n; j++)
            {
              S.add(rshares[cnt], epsilon[cnt], P.get_mac_keys());
              machine.Ms.write(reg + j, S, machine.verbose, Proc.get_PC());
              cnt++;
            }
        }
    }
}

template<class SRegint, class SBit>
void Processor_IO<SRegint, SBit>::Common_Subroutine_O(unsigned int player, unsigned int number,
                                                      Processor<SRegint, SBit> &Proc, Player &P,
                                                      offline_control_data &OCD)
{
  int thread= Proc.get_thread_num();
  Wait_For_Preproc(DATA_INPUT_MASK, number, thread, OCD, player);

  OCD.OCD_mutex[thread].lock();

  epsilon.resize(number);
  rshares.resize(number);

  if (player == P.whoami())
    {
      for (unsigned int i= 0; i < number; i++)
        {
          epsilon[i]= SacrificeD[thread].ID.opened_ios.front();
          SacrificeD[thread].ID.opened_ios.pop_front();
        }
    }

  for (unsigned int i= 0; i < number; i++)
    {
      rshares[i]= SacrificeD[thread].ID.ios[player].front();
      SacrificeD[thread].ID.ios[player].pop_front();
    }

  OCD.OCD_mutex[thread].unlock();
#ifdef BENCH_OFFLINE
  P.inputs+= number;
#endif
}

template<class SRegint, class SBit>
void Processor_IO<SRegint, SBit>::private_output(
    unsigned int player, int source, unsigned int channel,
    Processor<SRegint, SBit> &Proc, Player &P,
    Machine<SRegint, SBit> &machine,
    offline_control_data &OCD,
    unsigned int vectorize)
{
  Common_Subroutine_O(player, vectorize, Proc, P, OCD);

  for (unsigned int i= 0; i < vectorize; i++)
    {
      rshares[i].add(Proc.get_Sp_ref(source + i));
    }

  // Via Channel one to avoid conflicts with START/STOP Opens
  vector<gfp> values(vectorize);
  Proc.Open_To_All_Begin(values, rshares, P, 1);
  Proc.Open_To_All_End(values, rshares, P, 1);

  // Guaranteed to be in online thread zero
  Proc.RunOpenCheck(P, machine.get_IO().Get_Check(), 1);
  Proc.RunOpenCheck(P, machine.get_IO().Get_Check(), 2);
  if (player == P.whoami())
    {
      for (unsigned int i= 0; i < vectorize; i++)
        {
          values[i].sub(epsilon[i]);
          machine.get_IO().private_output_gfp(values[i], channel);
        }
    }
}

template<class SRegint, class SBit>
void Processor_IO<SRegint, SBit>::mprivate_output(
    unsigned int player, int source, unsigned int channel,
    unsigned int n,
    Processor<SRegint, SBit> &Proc, Player &P,
    Machine<SRegint, SBit> &machine,
    offline_control_data &OCD,
    unsigned int vectorize)
{
  Common_Subroutine_O(player, n * vectorize, Proc, P, OCD);

  unsigned int cnt= 0;
  for (unsigned int i= 0; i < vectorize; i++)
    {
      long reg= Proc.read_Ri(source + i);
      for (unsigned int j= 0; j < n; j++)
        {
          rshares[cnt].add(machine.Ms.read(reg + j, machine.verbose));
          cnt++;
        }
    }

  // Via Channel one to avoid conflicts with START/STOP Opens
  vector<gfp> values(n * vectorize);
  Proc.Open_To_All_Begin(values, rshares, P, 1);
  Proc.Open_To_All_End(values, rshares, P, 1);

  // Guaranteed to be in online thread zero
  Proc.RunOpenCheck(P, machine.get_IO().Get_Check(), 1);
  Proc.RunOpenCheck(P, machine.get_IO().Get_Check(), 2);
  if (player == P.whoami())
    {
      for (unsigned int i= 0; i < n * vectorize; i++)
        {
          values[i].sub(epsilon[i]);
          machine.get_IO().private_output_gfp(values[i], channel);
        }
    }
}

template class Processor_IO<aBitVector, aBit>;
template class Processor_IO<aBitVector2, Share2>;
