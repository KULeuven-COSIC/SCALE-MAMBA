/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

/* The fake offline phases all run in the same way
 *   - Actual data is produced in the clear by player zero
 *   - Mac values computed by player zero by reading in the Mac values
 *   - Sharing is done by player zero using the generator matrix
 *   - Share values created and sent to other players
 *   - To aid testing the same random numbers are used always for fake offline
 */

#include "Offline/FakePrep.h"
#include "config.h"
#include "offline_subroutines.h"

FakePrep::FakePrep(Player &P) : P(P)
{
  triples.resize(3);
  squares.resize(2);
  rewind_triples= false;
  rewind_squares= false;
  rewind_bits= false;
}

void FakePrep::produce_triples(list<Share> &a, list<Share> &b, list<Share> &c)
{
  if (rewind_triples)
    {
      a= triples[0];
      b= triples[1];
      c= triples[2];
      return;
    }
  if (P.whoami() == 0)
    {
      PRNG PRG;
      unsigned char Seed[SEED_SIZE];
      for (int i= 0; i < SEED_SIZE; i++)
        {
          Seed[i]= 0;
        }
      PRG.SetSeedFromRandom(Seed);

      init_fake();
      gfp aa, bb, cc;
      int n= P.nplayers();
      vector<gfp> amacs(Share::SD.nmacs);
      vector<gfp> bmacs(Share::SD.nmacs);
      vector<gfp> cmacs(Share::SD.nmacs);
      vector<Share> sa(n), sb(n), sc(n);
      for (int i= 0; i < sz_offline_batch; i++)
        {
          aa.randomize(PRG);
          bb.randomize(PRG);
          cc.mul(aa, bb);

          make_fake_macs(amacs, aa);
          make_fake_macs(bmacs, bb);
          make_fake_macs(cmacs, cc);

          make_shares(sa, aa, amacs, PRG);
          make_shares(sb, bb, bmacs, PRG);
          make_shares(sc, cc, cmacs, PRG);

          /* Now send the data to all parties and fix own shares */
          a.push_back(sa[0]);
          b.push_back(sb[0]);
          c.push_back(sc[0]);

          for (unsigned int j= 1; j < P.nplayers(); j++)
            {
              stringstream ss;
              sa[j].output(ss, false);
              sb[j].output(ss, false);
              sc[j].output(ss, false);
              P.send_to_player(j, ss.str());
            }
        }
    }
  else
    {
      for (int i= 0; i < sz_offline_batch; i++)
        {
          string ss;
          P.receive_from_player(0, ss);
          istringstream is(ss);
          Share s(P.whoami());
          s.input(is, false);
          a.push_back(s);
          s.input(is, false);
          b.push_back(s);
          s.input(is, false);
          c.push_back(s);
        }
    }
  triples[0]= a;
  triples[1]= b;
  triples[2]= c;
  rewind_triples= true;
}
void FakePrep::produce_squares(list<Share> &a, list<Share> &b, unsigned int rep)
{
  if (rewind_squares)
    {
      a= squares[0];
      b= squares[1];
      return;
    }

  if (P.whoami() == 0)
    {
      PRNG PRG;
      unsigned char Seed[SEED_SIZE];
      for (int i= 0; i < SEED_SIZE; i++)
        {
          Seed[i]= 0;
        }
      PRG.SetSeedFromRandom(Seed);

      init_fake();
      gfp aa, bb;
      int n= P.nplayers();
      vector<gfp> amacs(Share::SD.nmacs);
      vector<gfp> bmacs(Share::SD.nmacs);
      vector<Share> sa(n), sb(n);
      while (a.size() < sz_offline_batch * rep)
        {
          aa.randomize(PRG);
          bb.mul(aa, aa);

          make_fake_macs(amacs, aa);
          make_fake_macs(bmacs, bb);

          make_shares(sa, aa, amacs, PRG);
          make_shares(sb, bb, bmacs, PRG);

          /* Now send the data to all parties and fix own shares */
          a.push_back(sa[0]);
          b.push_back(sb[0]);
          for (unsigned int j= 1; j < P.nplayers(); j++)
            {
              stringstream ss;
              sa[j].output(ss, false);
              sb[j].output(ss, false);
              P.send_to_player(j, ss.str());
            }
        }
    }
  else
    {
      while (a.size() < sz_offline_batch * rep)
        {
          string ss;
          P.receive_from_player(0, ss);
          istringstream is(ss);
          Share s(P.whoami());
          s.input(is, false);
          a.push_back(s);
          s.input(is, false);
          b.push_back(s);
        }
    }
  squares[0]= a;
  squares[1]= b;
  rewind_squares= true;
}

void FakePrep::produce_bits(list<Share> &b)
{
  if (rewind_bits)
    {
      b= bits;
      return;
    }

  if (P.whoami() == 0)
    {
      PRNG PRG;
      unsigned char Seed[SEED_SIZE];
      for (int i= 0; i < SEED_SIZE; i++)
        {
          Seed[i]= 0;
        }
      PRG.SetSeedFromRandom(Seed);

      init_fake();
      int n= P.nplayers();
      gfp bb;
      vector<gfp> bmacs(Share::SD.nmacs);
      vector<Share> sb(n);
      for (int i= 0; i < sz_offline_batch / 8; i++)
        {
          unsigned char u= PRG.get_uchar();
          for (int j= 0; j < 8; j++)
            {
              if (u & 1)
                {
                  bb.assign_one();
                }
              else
                {
                  bb.assign_zero();
                }
              u= u >> 1;

              make_fake_macs(bmacs, bb);
              make_shares(sb, bb, bmacs, PRG);

              /* Now send the data to all parties and fix own shares */
              b.push_back(sb[0]);
              for (unsigned int k= 1; k < P.nplayers(); k++)
                {
                  stringstream ss;
                  sb[k].output(ss, false);
                  P.send_to_player(k, ss.str());
                }
            }
        }
    }
  else
    {
      for (int i= 0; i < sz_offline_batch / 8; i++)
        {
          for (int j= 0; j < 8; j++)
            {
              string ss;
              P.receive_from_player(0, ss);
              istringstream is(ss);
              Share s(P.whoami());
              s.input(is, false);
              b.push_back(s);
            }
        }
    }
  bits= b;
  rewind_bits= true;
}
