/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include <unistd.h>

#include "Online.h"
#include "Processor/Processor.h"

void online_phase(int online_num, Player &P, offline_control_data &OCD,
                  Machine &machine)
{
  printf("Doing online for player %d in online thread %d\n", P.whoami(),
         online_num);
  fflush(stdout);

  // Wait until enough triples have been produced
  bool wait= true;
  while (wait)
    {
      OCD.sacrifice_mutex[online_num].lock();
      if (OCD.totm[online_num] > OCD.minm && OCD.tots[online_num] > OCD.mins &&
          OCD.totb[online_num] > OCD.minb)
        {
          wait= false;
        }
      OCD.sacrifice_mutex[online_num].unlock();
      if (wait)
        {
          sleep(1);
        }
    }
  printf("Starting online phase\n");

  // Initialise the program
  Processor Proc(online_num, P.nplayers());

  bool flag= true;

  // synchronize
  fprintf(stderr, "Signal online thread ready %d\n", online_num);
  machine.Signal_Ready(online_num, true);

  while (flag)
    { // Pause online thread until it has a program to run
      int program= machine.Pause_While_Nothing_To_Do(online_num);

      if (program == -1)
        {
          flag= false;
          fprintf(stderr, "\tThread %d terminating\n", online_num);
        }
      else
        { // Execute the program
          Proc.execute(machine.progs[program], machine.get_OTI_arg(online_num), P,
                       machine, OCD);

          // MAC/Hash Check
          if (online_num == 0)
            {
              Proc.RunOpenCheck(P, machine.get_IO().Get_Check());
            }
          else
            {
              Proc.RunOpenCheck(P, "");
            }

          machine.Signal_Finished_Tape(online_num);
        }
    }

  // Run checks again
  if (online_num == 0)
    {
      Proc.RunOpenCheck(P, machine.get_IO().Get_Check());
    }
  else
    {
      Proc.RunOpenCheck(P, "");
    }

  machine.Lock_Until_Ready(online_num);

  // Signal offline threads I am dying now
  OCD.sacrifice_mutex[online_num].lock();
  OCD.finish_offline[online_num]= 1;
  OCD.sacrifice_mutex[online_num].unlock();
  printf("Exiting online phase : %d\n", online_num);
}
