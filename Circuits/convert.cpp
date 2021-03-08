/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "GC/SimplifyCircuit.h"

#include <boost/tokenizer.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>
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
  cout << "Error with wire : " << ss << endl;
  abort();
}

/* inputs/outputs only needed when parsing output wires */
void parse_wires(const vector<string> &tokens,
                 vector<string> &wirenames,
                 vector<unsigned int> &input_outputs,
                 unsigned int start)
{
  if (start == 1 && tokens[1].c_str()[0] == '[')
    { // We have numbered wires here
      unsigned int e= stoi(tokens[1].substr(1));
      unsigned int s= stoi(tokens[2]);
      string var= tokens[3];
      if (e >= s)
        {
          input_outputs.push_back(e - s + 1);
          for (unsigned int i= s; i <= e; i++)
            {
              stringstream ss;
              ss << var << "[" << i << "]";
              wirenames.push_back(ss.str());
            }
        }
      else
        {
          input_outputs.push_back(s - e + 1);
          for (int i= s; i >= (int) e; i--)
            {
              stringstream ss;
              ss << var << "[" << i << "]";
              wirenames.push_back(ss.str());
            }
        }
    }
  else
    {
      for (unsigned int i= start; i < tokens.size(); i++)
        {
          wirenames.push_back(tokens[i]);
          input_outputs.push_back(1);
        }
    }
}

int main(int argc, const char *argv[])
{

  if ((argc != 2 && argc != 3) || (argc == 3 && strncmp(argv[2], "x", 2) != 0))
    {
      cout << "Call using\n\tconvert.x file\nto convert\n\tVHDL/file.net\nto Bristol format\n";
      cout << "\nTo stop optimizations use\n\tconvert.x file x" << endl;
      exit(1);
    }

  string input_file_name= argv[1], output_file_name= argv[1];
  bool optimize= true;
  if (argc == 3)
    {
      optimize= false;
    }
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
  vector<unsigned int> dummy;

  // Now parse inputs
  do
    {
      if (tokens[0] != "input")
        {
          abort();
        }
      parse_wires(tokens, wirenames, inputs, 1);
      getline(inpf, line);
      tokenize(tokens, line);
    }
  while (tokens[0] != "output");

  // Now parse outputs
  do
    {
      if (tokens[0] != "output")
        {
          abort();
        }
      parse_wires(tokens, owirenames, outputs, 1);
      getline(inpf, line);
      tokenize(tokens, line);
    }
  while (tokens[0] != "wire");

  // Now the internal wires
  do
    {
      if (tokens[0] == "wire")
        {
          parse_wires(tokens, wirenames, dummy, 1);
        }
      else
        {
          parse_wires(tokens, wirenames, dummy, 0);
        }
      getline(inpf, line);
      tokenize(tokens, line);
    }
  while (tokens[0] != "XOR2_X1" && tokens[0] != "INV_X1" && tokens[0] != "NOR2_X1" && tokens[0] != "AND2_X1" && tokens[0] != "XOR2" && tokens[0] != "INV" && tokens[0] != "NOR2" && tokens[0] != "AND2" && tokens[0] != "assign");

  // Now add the contant wires
  stringstream w0, w1;
  w0 << "1'b0";
  w1 << "1'b1";
  wirenames.push_back(w0.str());
  wirenames.push_back(w1.str());

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
  cout << "Read in gates" << endl;

  // Count the number of NOR gates
  unsigned int nor_cnt= 0;
  for (unsigned int i= 0; i < gates.size(); i++)
    {
      if (gates[i][0] == "NOR2_X1" || gates[i][0] == "NOR2")
        {
          nor_cnt++;
        }
    }
  cout << "Counted NOR gates : " << nor_cnt << endl;

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
  cout << "Rewriting gates..." << endl;
  vector<string> ngates;
  unsigned int cnt= 0;
  for (unsigned int i= 0; i < gates.size(); i++)
    {
      if (i % 10000 == 0)
        {
          cout << "\t" << i << " out of " << gates.size() << endl;
        }

      if (gates[i][0] == "INV_X1" || gates[i][0] == "INV")
        {
          stringstream ss;
          ss << "1 1 ";
          ss << relabel(gates[i][3], wirenames) << " ";
          ss << relabel(gates[i][5], wirenames) << " ";
          ss << "INV";
          ngates.push_back(ss.str());
        }
      else if (gates[i][0] == "XOR2_X1" || gates[i][0] == "XOR2")
        {
          stringstream ss;
          ss << "2 1 ";
          ss << relabel(gates[i][3], wirenames) << " ";
          ss << relabel(gates[i][5], wirenames) << " ";
          ss << relabel(gates[i][7], wirenames) << " ";
          ss << "XOR";
          ngates.push_back(ss.str());
        }
      else if (gates[i][0] == "AND2_X1" || gates[i][0] == "AND2")
        {
          stringstream ss;
          ss << "2 1 ";
          ss << relabel(gates[i][3], wirenames) << " ";
          ss << relabel(gates[i][5], wirenames) << " ";
          ss << relabel(gates[i][7], wirenames) << " ";
          ss << "AND";
          ngates.push_back(ss.str());
        }
      else if (gates[i][0] == "NOR2_X1" || gates[i][0] == "NOR2")
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
  cout << "Processed gates" << endl;

  // Now add in the constant wire assignments
  //   If we do not need these then simplify should get rid of them
  stringstream s0, s1;
  s0 << "1 1 0 " << relabel("1'b0", wirenames) << " EQ";
  s1 << "1 1 1 " << relabel("1'b1", wirenames) << " EQ";
  // Push in two goes so we have the correct size
  ngates.push_back(s0.str());
  ngates.push_back(s1.str());

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

  cout << "Processed file, now sorting..." << endl;
  Circuit C;
  istringstream is(Cout.str());
  is >> C;
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
