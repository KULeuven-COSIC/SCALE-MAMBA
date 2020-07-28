/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "COPE.h"
#include "LSSS/PRSS.h"

void Sender_COPE::init(Player &P, int i, unsigned int fsize, CryptoPP::RandomPool &RNG, unsigned int threadNb)
{
  cout << "[COPE - Sender::init] INIT SENDER COPE whoami = " << P.whoami() << " other = " << i << endl;
  connection= threadNb;
  bit_size_field= fsize;
  nbReceiver= i;

  ROT_S.resize(bit_size_field);
  string input, output;

  //Only generate one public key
  ROT_S[0].init(output, RNG);
  P.send_to_player(nbReceiver, output, threadNb);

  for (unsigned int j= 0; j < bit_size_field; j++)
    {
      if (j != 0)
        {
          ROT_S[j]= ROT_S[0];
          ROT_S[j].reset();
        }
      P.receive_from_player(nbReceiver, input, connection);
      ROT_S[j].message(input);
    }

  u.resize(bit_size_field);
  g.resize(bit_size_field);
  for (unsigned int i= 0; i < bit_size_field; i++)
    {
      g[i].assign(2);
      g[i].power(i);
    }
}

void Receiver_COPE::init(Player &P, int i, unsigned int fsize, gfp &delta, CryptoPP::RandomPool &RNG, unsigned int threadNb)
{
  cout << "[COPE - Receiver::init] INIT RECEIVER COPE whoami = " << P.whoami() << " other = " << i << endl;

  connection= threadNb;
  nbSender= i;
  bit_size_field= fsize;
  Delta= delta;

  ROT_R.resize(bit_size_field);
  string input, output;

  //We only have one public key coming to us
  P.receive_from_player(nbSender, input, connection);

  //Bit decompose delta as choicebits
  choicebits.resize(bit_size_field);
  bigint Delta_bigint;

  to_bigint(Delta_bigint, Delta, false);
  bigint_to_bits(choicebits, Delta_bigint);

  for (unsigned int j= 0; j < bit_size_field; j++)
    {
      ROT_R[j].init(input, choicebits[j]);
      ROT_R[j].message(output, RNG);
      P.send_to_player(nbSender, output, connection);
    }

  u.resize(bit_size_field);
  g.resize(bit_size_field);
  for (unsigned int i= 0; i < bit_size_field; i++)
    {
      g[i].assign(2);
      g[i].power(i);
    }
}

void Sender_COPE::extend_vec(Player &P, vector<gfp> &x, vector<gfp> &out, bool resize)
{
  if (t.empty() || t.size() != out.size())
    {
      t.resize(out.size());
      for (unsigned int i= 0; i < out.size(); i++)
        {
          t[i].resize(bit_size_field);
          for (unsigned int j= 0; j < bit_size_field; j++)
            {
              t[i][j].resize(2);
            }
        }
    }
  unsigned int batch_size= 100000;

  unsigned int nb_batches= ceil((float) out.size() / batch_size);
  for (unsigned int l= 0; l < nb_batches; l++)
    {

      ostringstream os;
      for (unsigned int k= 0; k < batch_size; k++)
        {
          if (l * batch_size + k >= out.size())
            {
              break;
            }
          for (unsigned int i= 0; i < bit_size_field; i++)
            {
              ROT_S[i].get_random_bits(0, t[l * batch_size + k][i][0]);
              ROT_S[i].get_random_bits(1, t[l * batch_size + k][i][1]);
              u[i]= t[l * batch_size + k][i][0] - t[l * batch_size + k][i][1] + x[l * batch_size + k];

              //Send u[i] to other
              u[i].output(os, false);
            }
        }

      P.send_to_player(nbReceiver, os.str(), connection);
    }

  for (unsigned int k= 0; k < out.size(); k++)
    {
      out[k].assign(0);
      for (unsigned int i= 0; i < bit_size_field; i++)
        {
          out[k]+= g[i] * t[k][i][0];
        }
      out[k].negate();
    }

  if (resize)
    {
      t.resize(batch_size);
    }
}

void Receiver_COPE::extend_vec(Player &P, vector<gfp> &out, bool resize)
{
  unsigned int batch_size= 100000;
  if (q.empty() || q.size() != out.size())
    {
      q.resize(out.size());
      t.resize(out.size());
      for (unsigned int i= 0; i < out.size(); i++)
        {
          q[i].resize(bit_size_field);
          t[i].resize(bit_size_field);
        }
    }

  for (unsigned int k= 0; k < out.size(); k++)
    {
      for (unsigned int i= 0; i < bit_size_field; i++)
        {
          ROT_R[i].get_random_bits(t[k][i]);
        }
    }
  unsigned int nb_batches= ceil((float) out.size() / batch_size);
  for (unsigned int l= 0; l < nb_batches; l++)
    {
      string ss;
      P.receive_from_player(nbSender, ss, connection);
      istringstream is(ss);

      for (unsigned int k= 0; k < batch_size; k++)
        {
          if (l * batch_size + k >= out.size())
            {
              break;
            }
          for (unsigned int i= 0; i < bit_size_field; i++)
            {
              u[i].input(is, false);
              if (choicebits[i] == 1)
                {
                  q[l * batch_size + k][i]= u[i] + t[l * batch_size + k][i];
                }
              else
                {
                  q[l * batch_size + k][i]= t[l * batch_size + k][i];
                }
            }
        }
    }

  for (unsigned int k= 0; k < out.size(); k++)
    {
      out[k].assign(0);
      for (unsigned int i= 0; i < bit_size_field; i++)
        {
          out[k]+= g[i] * q[k][i];
        }
    }

  if (resize)
    {
      q.resize(batch_size);
      t.resize(batch_size);
    }
}

void Sender_COPE::extend(Player &P, gfp &x, gfp &out)
{
  vector<vector<gfp>> t;
  vector<gfp> u;

  t.resize(bit_size_field);
  u.resize(bit_size_field);

  ostringstream os;
  for (unsigned int i= 0; i < bit_size_field; i++)
    {
      t[i].resize(2);
      ROT_S[i].get_random_bits(0, t[i][0]);
      ROT_S[i].get_random_bits(1, t[i][1]);
      u[i]= t[i][0] - t[i][1] + x;

      //Send u[i] to other
      u[i].output(os, false);
    }

  P.send_to_player(nbReceiver, os.str(), connection);

  //Compute out
  vector<gfp> g;
  g.resize(bit_size_field);
  for (unsigned int i= 0; i < bit_size_field; i++)
    {
      g[i].assign(2);
      g[i].power(i);
    }

  out.assign(0);
  for (unsigned int i= 0; i < bit_size_field; i++)
    {
      out+= g[i] * t[i][0];
    }
  out.negate();
}

void Receiver_COPE::extend(Player &P, gfp &out)
{
  vector<gfp> t;
  vector<gfp> u;
  vector<gfp> q;

  t.resize(bit_size_field);
  u.resize(bit_size_field);
  q.resize(bit_size_field);
  //Get u[i] from other
  string ss;
  P.receive_from_player(nbSender, ss, connection);
  istringstream is(ss);

  for (unsigned int i= 0; i < bit_size_field; i++)
    {
      ROT_R[i].get_random_bits(t[i]);
      u[i].input(is, false);
      if (choicebits[i] == 1)
        {
          q[i]= u[i] + t[i];
        }
      else
        {
          q[i]= t[i];
        }
    }

  //Compute out
  vector<gfp> g;
  g.resize(bit_size_field);
  for (unsigned int i= 0; i < bit_size_field; i++)
    {
      g[i].assign(2);
      g[i].power(i);
    }

  out.assign(0);
  for (unsigned int i= 0; i < bit_size_field; i++)
    {
      out+= g[i] * q[i];
    }
}
