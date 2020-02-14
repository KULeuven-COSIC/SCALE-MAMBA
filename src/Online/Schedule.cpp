/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include <fstream>

#include "Exceptions/Exceptions.h"
#include "Schedule.h"

void print_hex(const char *s, int n)
{
  for (int i= 0; i < n; i++)
    {
      printf("%02x", (unsigned int) s[i]);
    }
}

string read_file(ifstream &in)
{
  string contents;
  in.seekg(0, std::ios::end);
  contents.resize(in.tellg());
  in.seekg(0, std::ios::beg);
  in.read(&contents[0], contents.size());
  return contents;
}

unsigned int Schedule::Load_Programs()
{
  char filename[2048];
  sprintf(filename, "%s/%s.sch", progname.c_str(), name.c_str());
  fprintf(stderr, "Opening file %s\n", filename);
  ifstream ifs, tfs;
  ifs.open(filename);
  if (ifs.fail())
    {
      throw file_error("Missing '" + string(filename) + "'. Did you compile '" +
                       name + "'?");
    }

  unsigned int nthreads;
  ifs >> nthreads;
  fprintf(stderr, "Number of online threads I will run in parallel =  %d\n",
          nthreads);

  int nprogs;
  ifs >> nprogs;
  fprintf(stderr, "Number of program sequences I need to load =  %d\n", nprogs);

  // Load in the programs
  progs.resize(nprogs);
  tnames.resize(nprogs);
  char threadname[1024];
  for (int i= 0; i < nprogs; i++)
    {
      ifs >> threadname;
      sprintf(filename, "%s/%s.bc", progname.c_str(), threadname);
      tnames[i]= string(threadname);
      fprintf(stderr, "Loading program %d from %s\n", i, filename);
      tfs.open(filename, ifstream::binary);
      if (tfs.fail())
        {
          throw file_error(filename);
        }
      progs[i]= stringstream(read_file(tfs));
      tfs.close();
      /*
      printf("%d\n",progs[i].str().size());
      printf("progs[%d] : \n------\n",i); print_hex(progs[i].str().c_str(),progs[i].str().size()); printf("\n------\n");
      */
    }

  // Load in the schedule
  Sch.resize(0);
  bool done= false;
  unsigned int nt, tape_number;
  while (!done)
    {
      ifs >> nt;
      if (nt == 0)
        {
          done= true;
        }
      else
        {
          vector<vector<int>> line(nt, vector<int>(2));
          for (unsigned int i= 0; i < nt; i++)
            {
              ifs >> tape_number;

              // Cope with passing an integer parameter to a tape
              int arg;
              if (ifs.get() == ':')
                ifs >> arg;
              else
                arg= 0;

              line[i][0]= tape_number;
              line[i][1]= arg;
            }
          Sch.push_back(line);
        }
    }
  ifs.get();
  ifs.getline(compiler_command, 1000);
  line_number= 0;
  ifs.close();

  return nthreads;
}

unsigned int Schedule::Load_Programs(const string &pname)
{
  progname= pname;
  /* First find progname itself, minus the directory path */
  int pos= progname.size() - 1, tot= progname.size() - 1;
  if (progname.c_str()[pos] == '/')
    {
      pos--;
      tot--;
    }
  while (progname.c_str()[pos] != '/' && pos >= 0)
    {
      pos--;
    }
  name= progname.substr(pos + 1, tot - pos);

  unsigned int nthreads= Load_Programs();

  return nthreads;
}
