/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#include "Garbled.h"
#include "Tools/MMO.h"
#include "Tools/Timer.h"

// Apply the PRF with keys k1 and k2 to the message (g||j) where
// j runs through 1 to n. To get an n-vector of gf2n elements out
void apply_PRF(vector<gf2n> &ans, const gf2n &k1, const gf2n &k2, unsigned int g)
{

  gf2n temp1, temp2, x;
  x.assign_x();
  temp1.mul(k1, x);
  temp2.mul(k2, x);
  temp2.mul(temp2, x);
  temp1.add(temp2);

  uint8_t buffer[16], kbuffer[16], ibuffer[4], obuffer[16]; // 16 as gf2n is 128 bits long
  temp1.store_into_buffer(kbuffer);

  memset(buffer, 0, 16 * sizeof(uint8_t));
  INT_TO_BYTES(ibuffer, g);

  // We fix buffer[5...] here, the lower bytes are set in the main loop
  for (unsigned int k= 4; k < 16; k++)
    {
      if (k < 12)
        {
          buffer[k]= kbuffer[k];
        }
      else
        {
          buffer[k]= kbuffer[k] ^ ibuffer[k - 12];
        }
    }

  MMO mmo;
  for (unsigned int j= 0; j < ans.size(); j++)
    {
      INT_TO_BYTES(ibuffer, j);
      for (unsigned int k= 0; k < 4; k++)
        {
          buffer[k]= kbuffer[k] ^ ibuffer[k];
        }
      mmo.hashOneBlock<gf2n>(obuffer, buffer);
      ans[j].load_from_buffer(obuffer);
    }
}

void Garbled_Circuit::Garble(const Circuit &C,
                             const vector<int> &i_a, const vector<int> &o_a,
                             aAND &aA, aBitFactory &aBF, Player &P)
{
  //Timer T; T.reset(); T.start();
  // This follows the method on page 27 of ePrint 2017/214
  unsigned int n= P.nplayers();
  gf2n Delta= aBF.get_Delta();

  unsigned int nW= C.get_nWires();
  lambda.resize(nW);
  k.resize(nW, vector<gf2n>(2));
  unsigned int nAG= C.num_AND_gates();
  vector<aBit> lambda_uv(nAG), lambda_u(nAG), lambda_v(nAG);
  gab.resize(nAG);

  //T.stop(); cout << "1) " << T.elapsed() << endl; T.reset(); T.start();

  // Step 2, 3 and 4
  // Fix the one label
  one_label.resize(n);
  one_label[P.whoami()]= aBF.get_random_gf2n();

  i_assign= i_a;
  o_assign= o_a;
  if (C.num_inputs() != i_assign.size())
    {
      throw bad_value();
    }

  if (C.num_outputs() != o_assign.size())
    {
      throw bad_value();
    }

  unsigned int nI= 0;
  for (unsigned int i= 0; i < C.num_inputs(); i++)
    {
      for (unsigned int j= 0; j < C.num_iWires(i); j++)
        {
          lambda[nI]= aBF.get_aShare(P);
          k[nI][0]= aBF.get_random_gf2n();
          k[nI][1].add(k[nI][0], Delta);
          nI++;
        }
    }
  for (unsigned int i= 0; i < C.get_nGates(); i++)
    {
      unsigned int iout= C.Gate_Wire_Out(i);
      if (C.get_GateType(i) == XOR)
        {
          lambda[iout].add(lambda[C.Gate_Wire_In(i, 0)],
                           lambda[C.Gate_Wire_In(i, 1)]);
          k[iout][0].add(k[C.Gate_Wire_In(i, 0)][0], k[C.Gate_Wire_In(i, 1)][0]);
        }
      else if (C.get_GateType(i) == AND)
        {
          lambda[iout]= aBF.get_aShare(P);
          k[iout][0]= aBF.get_random_gf2n();
        }
      else
        {
          lambda[iout]= lambda[C.Gate_Wire_In(i, 0)];
          k[iout][0].add(k[C.Gate_Wire_In(i, 0)][0], one_label[P.whoami()]);
        }
      k[iout][1].add(k[iout][0], Delta);
    }
  //T.stop(); cout << "2) " << T.elapsed() << endl; T.reset(); T.start();

  // Step 5
  for (unsigned int g= 0; g < nAG; g++)
    {
      unsigned int i= C.get_nth_AND_Gate(g);
      lambda_u[g]= lambda[C.Gate_Wire_In(i, 0)];
      lambda_v[g]= lambda[C.Gate_Wire_In(i, 1)];
    }
  Mult_aBits(lambda_uv, lambda_u, lambda_v, aA, aBF, P);
  //T.stop(); cout << "3) " << T.elapsed() << endl; T.reset(); T.start();

  // Step 6
  vector<vector<vector<gf2n>>> rho(2, vector<vector<gf2n>>(2, vector<gf2n>(n)));
  vector<gf2n> s_l_u, s_l_v;
  for (unsigned int g= 0; g < nAG; g++)
    {
      unsigned int i= C.get_nth_AND_Gate(g);
      lambda_u[g].get_Share_x_Delta_j(s_l_u);
      lambda_v[g].get_Share_x_Delta_j(s_l_v);
      lambda_uv[g].add(lambda[C.Gate_Wire_Out(i)]);
      lambda_uv[g].get_Share_x_Delta_j(rho[0][0]);
      for (unsigned int j= 0; j < n; j++)
        {
          rho[1][0][j].add(rho[0][0][j], s_l_v[j]);
          rho[0][1][j].add(rho[0][0][j], s_l_u[j]);
          rho[1][1][j].add(rho[1][0][j], s_l_u[j]);
          if (j == P.whoami())
            {
              rho[1][1][j].add(Delta);
            }
        }

      // Now do Step 4 of Figure 6
      gab[g].resize(2);
      for (unsigned int a= 0; a < 2; a++)
        {
          gab[g][a].resize(2);
          for (unsigned int b= 0; b < 2; b++)
            {
              gab[g][a][b].resize(n);
              apply_PRF(gab[g][a][b], k[C.Gate_Wire_In(i, 0)][a], k[C.Gate_Wire_In(i, 1)][b], g);
              for (unsigned int j= 0; j < n; j++)
                {
                  gab[g][a][b][j].add(rho[a][b][j]);
                  if (j == P.whoami())
                    {
                      gab[g][a][b][j].add(k[C.Gate_Wire_Out(i)][0]);
                    }
                }
            }
        }
    }
  //T.stop(); cout << "4) " << T.elapsed() << endl; T.reset(); T.start();

  // Step 7
  // Open output wires to player i
  vector<aBit> data;
  vector<int> dummy;
  unsigned int cnt= C.get_nWires();
  for (unsigned int i= 0; i < C.num_outputs(); i++)
    {
      cnt-= C.num_oWires(i);
    }

  outputs.resize(C.num_outputs());
  for (unsigned int i= 0; i < C.num_outputs(); i++)
    {
      data.resize(C.num_oWires(i));
      for (unsigned int j= 0; j < C.num_oWires(i); j++)
        {
          data[j]= lambda[cnt];
          cnt++;
        }
      // XXXX May change this cast when we decide what to do about other inputs/outputs
      if (o_assign[i] == (int) P.whoami())
        {
          Open_aBits_To(outputs[i], o_assign[i], data, P, Delta);
        }
      else
        {
          Open_aBits_To(dummy, o_assign[i], data, P, Delta);
        }
    }
  //T.stop(); cout << "5) " << T.elapsed() << endl; T.reset(); T.start();

  // Step 8
  // Do the same with input wires
  inputs.resize(C.num_inputs());
  cnt= 0;
  for (unsigned int i= 0; i < C.num_inputs(); i++)
    {
      data.resize(C.num_iWires(i));
      for (unsigned int j= 0; j < C.num_iWires(i); j++)
        {
          data[j]= lambda[cnt];
          cnt++;
        }
      // XXXX Ditto (re the cast)
      if (i_assign[i] == (int) P.whoami())
        {
          Open_aBits_To(inputs[i], i_assign[i], data, P, Delta);
        }
      else
        {
          Open_aBits_To(dummy, i_assign[i], data, P, Delta);
        }
    }
  //T.stop(); cout << "6) " << T.elapsed() << endl; T.reset(); T.start();

  // Now opening garbling, altering the one_label
  one_label[P.whoami()].add(Delta);
  Open_Garbling(P);
  //T.stop(); cout << "7) " << T.elapsed() << endl; T.reset(); T.start();
}

void Garbled_Circuit::Open_Garbling(Player &P)
{
  vector<string> oo(P.nplayers());

  stringstream ss;
  for (unsigned int g= 0; g < gab.size(); g++)
    {
      for (unsigned int a= 0; a < 2; a++)
        {
          for (unsigned int b= 0; b < 2; b++)
            {
              for (unsigned int i= 0; i < P.nplayers(); i++)
                {
                  gab[g][a][b][i].output(ss);
                }
            }
        }
    }
  one_label[P.whoami()].output(ss);
  oo[P.whoami()]= ss.str();

  P.Broadcast_Receive(oo);

  gf2n temp;
  for (unsigned int k= 0; k < P.nplayers(); k++)
    {
      if (k != P.whoami())
        {
          istringstream ss(oo[k]);
          for (unsigned int g= 0; g < gab.size(); g++)
            {
              for (unsigned int a= 0; a < 2; a++)
                {
                  for (unsigned int b= 0; b < 2; b++)
                    {
                      for (unsigned int i= 0; i < P.nplayers(); i++)
                        {
                          temp.input(ss);
                          gab[g][a][b][i].add(temp);
                        }
                    }
                }
            }
          temp.input(ss);
          one_label[k]= temp;
        }
    }
}

/* Figure 11 on page 29 of 2017/214 */
void Garbled_Circuit::Evaluate(vector<vector<int>> &output,
                               const vector<vector<int>> &input,
                               const Circuit &C, Player &P)
{
  if (C.num_inputs() != input.size())
    {
      throw bad_value();
    }

  //Timer T; T.reset(); T.start();
  // These hold the external values and the opened k values
  vector<int> Gamma(C.get_nWires());
  vector<vector<gf2n>> ok(C.get_nWires(), vector<gf2n>(P.nplayers()));

  // Line 1
  unsigned int tot_num_iwires= 0;
  vector<string> o(P.nplayers());
  stringstream ss1;
  for (unsigned int i= 0; i < C.num_inputs(); i++)
    { //XXXX Ditto re cast
      if (i_assign[i] == (int) P.whoami())
        {
          for (unsigned int j= 0; j < input[i].size(); j++)
            {
              Gamma[j + tot_num_iwires]= inputs[i][j] ^ input[i][j];
              ss1 << (char) Gamma[j + tot_num_iwires];
            }
        }
      tot_num_iwires+= C.num_iWires(i);
    }
  o[P.whoami()]= ss1.str();

  P.Broadcast_Receive(o, true);

  char c;
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      if (i != P.whoami())
        {
          istringstream is(o[i]);
          unsigned int cnt= 0;
          for (unsigned int j= 0; j < C.num_inputs(); j++)
            { // XXXX Ditto re cast
              if (i_assign[j] == (int) i)
                {
                  for (unsigned int k= 0; k < C.num_iWires(j); k++)
                    {
                      is >> c;
                      Gamma[k + cnt]= (int) c;
                    }
                }
              cnt+= C.num_iWires(j);
            }
        }
    }
  //T.stop(); cout << "a) " << T.elapsed() << endl; T.reset(); T.start();

  // Vector of opened k values

  // Line 2
  stringstream ss2;
  for (unsigned int i= 0; i < tot_num_iwires; i++)
    {
      ok[i][P.whoami()]= k[i][Gamma[i]];
      ok[i][P.whoami()].output(ss2);
    }
  o[P.whoami()]= ss2.str();

  P.Broadcast_Receive(o, true);

  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      if (i != P.whoami())
        {
          istringstream is(o[i]);
          for (unsigned int j= 0; j < tot_num_iwires; j++)
            {
              ok[j][i].input(is);
            }
        }
    }
  //T.stop(); cout << "b) " << T.elapsed() << endl; T.reset(); T.start();

  // Line 4
  unsigned int i0, i1, out;
  vector<vector<gf2n>> ans(P.nplayers(), vector<gf2n>(P.nplayers()));
  for (unsigned int i= 0; i < C.get_nGates(); i++)
    {
      i0= C.Gate_Wire_In(i, 0);
      if (C.get_GateType(i) != INV)
        {
          i1= C.Gate_Wire_In(i, 1);
        }
      out= C.Gate_Wire_Out(i);
      if (C.get_GateType(i) == XOR)
        {
          Gamma[out]= Gamma[i0] ^ Gamma[i1];
          for (unsigned int j= 0; j < P.nplayers(); j++)
            {
              ok[out][j].add(ok[i0][j], ok[i1][j]);
            }
        }
      else if (C.get_GateType(i) == INV)
        {
          Gamma[out]= 1 - Gamma[i0];
          for (unsigned int j= 0; j < P.nplayers(); j++)
            {
              ok[out][j].add(ok[i0][j], one_label[j]);
            }
        }
      else
        { // AND Gate
          unsigned int g= C.map_to_AND_Gate(i);
          for (unsigned int k= 0; k < P.nplayers(); k++)
            {
              apply_PRF(ans[k], ok[i0][k], ok[i1][k], g);
            }
          for (unsigned int j= 0; j < P.nplayers(); j++)
            {
              ok[out][j]= gab[g][Gamma[i0]][Gamma[i1]][j];
              for (unsigned int k= 0; k < P.nplayers(); k++)
                {
                  ok[out][j].add(ans[k][j]);
                }
            }
          // Line 4.3 and 4.4
          if (ok[out][P.whoami()] == k[out][0])
            {
              Gamma[out]= 0;
            }
          else if (ok[out][P.whoami()] == k[out][1])
            {
              Gamma[out]= 1;
            }
          else
            {
              throw circuit_eval_error();
            }
        }
    }

  P.Check_Broadcast();
  //T.stop(); cout << "c) " << T.elapsed() << endl; T.reset(); T.start();

  // Line 5
  output.resize(C.num_outputs());
  unsigned int cnt= C.get_nWires();
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      cnt-= C.num_oWires(i);
    }
  for (unsigned int i= 0; i < C.num_outputs(); i++)
    { // Ditto re cast
      if (o_assign[i] == (int) P.whoami())
        {
          output[i].resize(C.num_oWires(i));
          for (unsigned int k= 0; k < output[i].size(); k++)
            {
              output[i][k]= outputs[i][k] ^ Gamma[cnt + k];
            }
        }
      else
        {
          output[i].resize(0);
        }
      cnt+= C.num_oWires(i);
    }
  //T.stop(); cout << "d) " << T.elapsed() << endl; T.reset(); T.start();
}
