/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "GC/SimplifyCircuit.h"

#include <fstream>
#include <iostream>
#include <string.h>

using namespace std;

int main(int argc, const char *argv[])
{

  if ((argc != 2 && argc != 3) || (argc == 3 && strncmp(argv[2], "x", 2) != 0))
    {
      cout << "Call using\n\tprocess.x file\nto check if\n\tFloat/file.txt\ncan be optimized\n";
      cout << "\nTo stop optimizations use\n\tconvert.x file x" << endl;
      exit(1);
    }

  string input_file_name= argv[1], output_file_name= argv[1];
  bool optimize= true;
  if (argc == 3)
    {
      optimize= false;
    }
  input_file_name= "Float/" + input_file_name + ".txt";
  output_file_name= "Bristol/FP-" + output_file_name + ".txt";

  cout << "Input  : " << input_file_name << endl;
  cout << "Output : " << output_file_name << endl;

  Circuit C;

  ifstream inpf(input_file_name.c_str());
  inpf >> C;
  inpf.close();
  C.sort();

  if (optimize)
    { // Only do basic simplify, as we assume VHDL compiler is good
      cout << "Converted circuit. I am now seeing if we can simplify..." << endl;
      SimplifyCircuit SC(C);
      SC.Simplify();
      C= SC.Get_Circuit();
    }

  ofstream outf(output_file_name.c_str());
  outf << C << endl;
  outf.close();

  cout << "All done" << endl;
}
