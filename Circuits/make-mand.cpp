/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "GC/SimplifyCircuit.h"

#include <fstream>
#include <iostream>
#include <string.h>

using namespace std;

int main(int argc, const char *argv[])
{

  if (argc != 2)
    {
      cout << "Call using\n\tmake-mad.x file\nto convert \n\tBristol/file.txt\nto include MADD gates\n";
      exit(1);
    }

  string input_file_name= argv[1], output_file_name= argv[1];
  input_file_name= "Bristol/" + input_file_name + ".txt";
  output_file_name= "MAND-Circuits/" + output_file_name + ".txt";

  cout << "Input  : " << input_file_name << endl;
  cout << "Output : " << output_file_name << endl;

  Circuit C;

  ifstream inpf(input_file_name.c_str());
  if (inpf.fail())
    {
      throw file_error(input_file_name);
    }
  inpf >> C;
  inpf.close();

  C.merge_AND_gates();

  ofstream outf(output_file_name.c_str());
  outf << C << endl;
  outf.close();

  cout << "All done" << endl;
}
