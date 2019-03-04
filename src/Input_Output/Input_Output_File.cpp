/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Input_Output_File.h"
#include "Exceptions/Exceptions.h"

void Input_Output_File::set_file_names(string a, string b) {
  inf_name = a;
  onf_name = b;
}

long Input_Output_File::open_channel(unsigned int channel)
{
    cout << "Opening channel " << channel << endl;

    if (channel == 0) {
      cout << "Opening file: " << inf_name << endl;
      inpf->open(inf_name);
      cout << "Opening file: " << onf_name << endl;
      outf->open(onf_name);
    }

  if (inpf->fail() || outf->fail()) {
    cout << "Fail opening files" << endl;
  }

  return 0;
}

void Input_Output_File::close_channel(unsigned int channel)
{
  cout << "Closing channel " << channel << endl;
  if (channel == 0) {
    inpf->close();
    outf->close();
  }
}

gfp Input_Output_File::private_input_gfp(unsigned int channel)
{
  cout << "Input channel " << channel << " : ";
  word x;
  (*inpf) >> x;

  cout << "Value from file: " << x << endl;

  gfp y;
  y.assign(x);
  return y;
}

void Input_Output_File::private_output_gfp(const gfp &output, unsigned int channel)
{
  cout << "Output channel " << channel << " : ";
  output.output((*outf), true);
  cout << endl;
}

gfp Input_Output_File::public_input_gfp(unsigned int channel)
{
  cout << "Enter value on channel " << channel << " : ";
  word x;
  (*inpf) >> x;
  cout << "Value from file: " << x << endl;
  gfp y;
  y.assign(x);

  // Important to have this call in each version of public_input_gfp
  Update_Checker(y, channel);

  return y;
}

void Input_Output_File::public_output_gfp(const gfp &output, unsigned int channel)
{
  cout << "Output channel " << channel << " : ";
  output.output(cout, true);
  cout << endl;
}

long Input_Output_File::public_input_int(unsigned int channel)
{
  cout << "Enter value on channel " << channel << " : ";
  long x;
  (*inpf) >> x;
  cout << "Value from file: " << x << endl;

  // Important to have this call in each version of public_input_gfp
  Update_Checker(x, channel);

  return x;
}

void Input_Output_File::public_output_int(const long output, unsigned int channel)
{
  cout << "Output channel " << channel << " : " << output << endl;
}

void Input_Output_File::output_share(const Share &S, unsigned int channel)
{
  S.output(*outf, human);
}

Share Input_Output_File::input_share(unsigned int channel)
{
  cout << "Enter value on channel " << channel << " : ";
  Share S;
  S.input(*inpf, human);
  return S;
}

void Input_Output_File::trigger(Schedule &schedule)
{
  printf("Restart requested: Enter a number to proceed\n");
  int i;
  cin >> i;

  // Load new schedule file program streams, using the original
  // program name
  //
  // Here you could define programatically what the new
  // programs you want to run are, by directly editing the
  // public variables in the schedule object.
  unsigned int nthreads= schedule.Load_Programs();
  if (schedule.max_n_threads() < nthreads)
    {
      throw Processor_Error("Restart requires more threads, cannot do this");
    }
}

void Input_Output_File::debug_output(const stringstream &ss)
{
  printf("%s", ss.str().c_str());
  fflush(stdout);
}

void Input_Output_File::crash(unsigned int PC, unsigned int thread_num)
{
  printf("Crashing in thread %d at PC value %d\n", thread_num, PC);
  throw crash_requested();
}
