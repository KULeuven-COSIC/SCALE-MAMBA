/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2019, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "GC/SimplifyCircuit.h"

#include <boost/tokenizer.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
using namespace boost;

void tokenize(vector<string> &vtokens, const string text)
{
  vtokens.resize(0);
  char_separator<char> sep(", :;()");
  tokenizer<char_separator<char>> tokens(text, sep);
  for (const auto &t : tokens)
    {
      vtokens.push_back(t);
    }
}

unsigned int relabel(const string &ss, const vector<string> &vs)
{
  for (unsigned int i= 0; i < vs.size(); i++)
    {
      if (vs[i] == ss)
        {
          return i;
        }
    }
  abort();
}

int main(int argc, const char *argv[])
{

  if (argc != 2)
    {
      cout << "Call using\n\tconvert.x file\nto convert\n\tVHDL/file_netlist.vhd\nto Bristol format" << endl;
      exit(1);
    }

  string input_file_name= argv[1], output_file_name= argv[1];
  input_file_name= "VHDL/" + input_file_name + ".net";
  output_file_name= "Bristol/" + output_file_name + ".txt";

  cout << "Input  : " << input_file_name << endl;
  cout << "Output : " << output_file_name << endl;

  ifstream inpf(input_file_name.c_str());
  string line;
  vector<string> tokens;
  vector<string> wirenames;
  vector<string> owirenames;

  // Get to inputs
  do
    {
      getline(inpf, line);
      tokenize(tokens, line);
    }
  while (tokens[0] != "input");

  vector<unsigned int> inputs;
  vector<unsigned int> outputs;

  // Now parse inputs
  do
    {
      unsigned int e= stoi(tokens[1].substr(1));
      unsigned int s= stoi(tokens[2]);
      string var= tokens[3];
      inputs.push_back(e - s + 1);
      for (unsigned int i= s; i <= e; i++)
        {
          stringstream ss;
          ss << var << "[" << i << "]";
          wirenames.push_back(ss.str());
        }
      getline(inpf, line);
      tokenize(tokens, line);
    }
  while (tokens[0] != "output");

  // Now parse outputs
  do
    {
      if (tokens[1].c_str()[0] == '[')
        { // We have numbered wires here
          unsigned int e= stoi(tokens[1].substr(1));
          unsigned int s= stoi(tokens[2]);
          string var= tokens[3];
          outputs.push_back(e - s + 1);
          for (unsigned int i= s; i <= e; i++)
            {
              stringstream ss;
              ss << var << "[" << i << "]";
              owirenames.push_back(ss.str());
            }
        }
      else
        { // Just a single wire
          owirenames.push_back(tokens[1]);
          outputs.push_back(1);
        }
      getline(inpf, line);
      tokenize(tokens, line);
    }
  while (tokens[0] != "wire");

  // Now the internal wires
  unsigned int st= 1;
  do
    {
      for (unsigned int i= st; i < tokens.size(); i++)
        {
          wirenames.push_back(tokens[i]);
        }
      getline(inpf, line);
      tokenize(tokens, line);
      st= 0;
    }
  while (tokens[0] != "XOR2_X1" && tokens[0] != "INV_X1" && tokens[0] != "NOR2_X1" && tokens[0] != "AND2_X1" && tokens[0] != "assign");

  // Now load in the gates
  vector<vector<string>> gates;
  do
    {
      if (tokens.size() != 0)
        {
          gates.push_back(tokens);
        }
      getline(inpf, line);
      tokenize(tokens, line);
    }
  while (tokens[0] != "endmodule");

  // Count the number of NOR gates
  unsigned int nor_cnt= 0;
  for (unsigned int i= 0; i < gates.size(); i++)
    {
      if (gates[i][0] == "NOR2_X1")
        {
          nor_cnt++;
        }
    }

  // Add in dummy wires for the NOR gates
  for (unsigned int i= 0; i < nor_cnt; i++)
    {
      stringstream ss1, ss2;
      ss1 << "dummya" << i;
      ss2 << "dummyb" << i;
      wirenames.push_back(ss1.str());
      wirenames.push_back(ss2.str());
    }

  // Now add on the output wires
  for (unsigned int i= 0; i < owirenames.size(); i++)
    {
      wirenames.push_back(owirenames[i]);
    }

  // Now rewrite each gate
  vector<string> ngates;
  unsigned int cnt= 0;
  for (unsigned int i= 0; i < gates.size(); i++)
    {
      if (gates[i][0] == "INV_X1")
        {
          stringstream ss;
          ss << "1 1 ";
          ss << relabel(gates[i][3], wirenames) << " ";
          ss << relabel(gates[i][5], wirenames) << " ";
          ss << "INV";
          ngates.push_back(ss.str());
        }
      else if (gates[i][0] == "XOR2_X1")
        {
          stringstream ss;
          ss << "2 1 ";
          ss << relabel(gates[i][3], wirenames) << " ";
          ss << relabel(gates[i][5], wirenames) << " ";
          ss << relabel(gates[i][7], wirenames) << " ";
          ss << "XOR";
          ngates.push_back(ss.str());
        }
      else if (gates[i][0] == "AND2_X1")
        {
          stringstream ss;
          ss << "2 1 ";
          ss << relabel(gates[i][3], wirenames) << " ";
          ss << relabel(gates[i][5], wirenames) << " ";
          ss << relabel(gates[i][7], wirenames) << " ";
          ss << "AND";
          ngates.push_back(ss.str());
        }
      else if (gates[i][0] == "NOR2_X1")
        {
          stringstream sv1, sv2;
          sv1 << "dummya" << cnt;
          sv2 << "dummyb" << cnt;
          stringstream ss1, ss2, ss3;
          ss1 << "1 1 ";
          ss1 << relabel(gates[i][3], wirenames) << " ";
          ss1 << relabel(sv1.str(), wirenames) << " ";
          ss1 << "INV";
          ss2 << "1 1 ";
          ss2 << relabel(gates[i][5], wirenames) << " ";
          ss2 << relabel(sv2.str(), wirenames) << " ";
          ss2 << "INV";
          ss3 << "2 1 ";
          ss3 << relabel(sv1.str(), wirenames) << " ";
          ss3 << relabel(sv2.str(), wirenames) << " ";
          ss3 << relabel(gates[i][7], wirenames) << " ";
          ss3 << "AND";
          ngates.push_back(ss1.str());
          ngates.push_back(ss2.str());
          ngates.push_back(ss3.str());
          cnt++;
        }
      else if (gates[i][0] == "assign")
        {
          stringstream ss;
          ss << "1 1 ";
          ss << relabel(gates[i][3], wirenames) << " ";
          ss << relabel(gates[i][1], wirenames) << " ";
          ss << "EQW";
          ngates.push_back(ss.str());
        }
      else
        {
          cout << "Something wrong" << endl;
          for (unsigned j= 0; j < gates[i].size(); j++)
            {
              cout << j << " " << gates[i][j] << endl;
            }
          exit(1);
        }
    }

  // Output circuit to stringstream
  stringstream Cout;
  Cout << ngates.size() << " " << wirenames.size() << endl;
  Cout << inputs.size() << " ";
  for (unsigned int i= 0; i < inputs.size(); i++)
    {
      Cout << inputs[i] << " ";
    }
  Cout << endl;
  Cout << outputs.size() << " ";
  for (unsigned int i= 0; i < outputs.size(); i++)
    {
      Cout << outputs[i] << " ";
    }
  Cout << endl;
  for (unsigned int i= 0; i < ngates.size(); i++)
    {
      Cout << ngates[i] << endl;
    }

  Circuit C;
  istringstream is(Cout.str());
  is >> C;
  C.sort();

  // Only do basic simplify, as we assume VHDL compiler is good
  cout << "Converted circuit. I am now seeing if we can simplify..." << endl;
  SimplifyCircuit SC(C);
  SC.Simplify();
  C= SC.Get_Circuit();

  ofstream outf(output_file_name.c_str());
  outf << C << endl;
  outf.close();
}
