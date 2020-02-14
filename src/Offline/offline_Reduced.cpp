/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#include "offline_Reduced.h"
#include "config.h"
#include "offline_subroutines.h"

/* Configures data to speed things up */
bool init= false;
bool maurer= true;
bool need_prss= false;
unsigned int pos;

void Init_Configure(unsigned int whoami)
{
  init= true;
  // Check whether this player does Maurer or specific send,
  // if not Maurer find which share value am I going to be sending
  // to the other players
  if (find(Share::SD.maurer_chans.begin(), Share::SD.maurer_chans.end(),
           whoami) == Share::SD.maurer_chans.end())
    {
      maurer= false;
    }
  pos= -1;
  if (!maurer)
    {
      for (unsigned int k= 0; k < Share::SD.M.shares_per_player(whoami); k++)
        {
          if (Share::SD.mult_proc[whoami][k] == 1)
            {
              pos= k;
            }
        }
    }
  // Now check whether we need a PRSS in the computation
  for (unsigned int i= 0; i < Share::SD.mult_proc.size(); i++)
    {
      for (unsigned int j= 0; j < Share::SD.mult_proc[i].size(); j++)
        {
          if (Share::SD.mult_proc[i][j] <= 0)
            {
              need_prss= true;
            }
        }
    }
}

void mult_inner_subroutine_one(const Share &aa, const Share &bb, Share &cc,
                               Share &cc_m, vector<stringstream> &ss_m,
                               stringstream &ss_r, Player &P, PRSS &prss,
                               PRZS &przs)
{
  Share dd;
  gfp prod, zero;
  zero.assign_zero();
  int whoami= P.whoami();

  /* Produce the Schur product */
  prod= schur_sum_prod(aa, bb, P);

  /* Apply the PRZS */
  prod+= przs.next_share();

  /* Call the PRSS in case we need it */
  if (need_prss)
    {
      dd= prss.next_share(P);
    }

  /* Adjust product if we are using a PRSS on one of our shares
   * else fix the share to be zero if we are going to receive it
   * from someone else */
  for (unsigned int k= 0; k < Share::SD.M.shares_per_player(whoami); k++)
    {
      if (need_prss)
        {
          if (Share::SD.mult_proc[whoami][k] <= 0)
            {
              cc.set_share(k, dd.get_share(k));
            }
          if (Share::SD.mult_proc[whoami][k] == -1)
            {
              prod-= dd.get_share(k);
            }
        }
      else if (Share::SD.mult_proc[whoami][k] == 2)
        {
          cc.set_share(k, zero);
        }
    }

  /* Assign prod to my share here */
  if (!maurer)
    {
      cc.set_share(pos, prod);
    }

  /* Now decide what to send */
  if (maurer)
    {
      vector<Share> maur(P.nplayers());
      make_shares(maur, prod, P.G);
      for (unsigned int k= 0; k < P.nplayers(); k++)
        {
          if (k != P.whoami())
            {
              maur[k].output(ss_m[k], false);
            }
        }
      cc_m= maur[whoami]; /* Stored for adding in later */
    }
  else
    { /* Specific Send of my value */
      prod.output(ss_r, false);
    }
}

void mult_inner_subroutine_two(vector<Share> &cc, vector<Share> &cc_m,
                               const vector<stringstream> &ss_m,
                               const stringstream &ss_r, Player &P)
{
  unsigned int whoami= P.whoami();
  gfp tmp;

  /* Now do the sending of my share of the product */
  if (maurer)
    {
      for (unsigned int k= 0; k < P.nplayers(); k++)
        {
          if (k != whoami)
            {
              P.send_to_player(k, ss_m[k].str());
            }
        }
    }
  else
    {
      for (unsigned int k= 0; k < Share::SD.mult_chans[whoami][pos].size();
           k++)
        {
          P.send_to_player(Share::SD.mult_chans[whoami][pos][k], ss_r.str());
        }
    }

  /* Now do the receiving */

  /* First do non Maurer receives */
  for (unsigned int k= 0; k < Share::SD.M.shares_per_player(whoami); k++)
    {
      if (Share::SD.mult_proc[whoami][k] == 2)
        {
          int pl= Share::SD.mult_chans[whoami][k][0];
          string sstr;
          P.receive_from_player(pl, sstr);
          istringstream is(sstr);
          for (int j= 0; j < amortize; j++)
            {
              tmp.input(is, false);
              cc[j].set_share(k, tmp);
            }
        }
    }
  /* Then the Maurer receives
   *   At this point cc[j] holds a valid share for all players.
   *   So we just have to receive the Maurer share and add it on
   */
  Share dd;
  for (unsigned int k= 0; k < Share::SD.maurer_chans.size(); k++)
    {
      unsigned int pl= Share::SD.maurer_chans[k];
      if (pl != whoami)
        {
          string sstr;
          P.receive_from_player(pl, sstr);
          istringstream is(sstr);
          for (int j= 0; j < amortize; j++)
            {
              dd.input(is, false);
              cc[j].add(dd);
            }
        }
      else
        {
          for (int j= 0; j < amortize; j++)
            {
              cc[j].add(cc_m[j]);
            }
        }
    }
}

void offline_Reduced_triples(Player &P, PRSS &prss, PRZS &przs, list<Share> &a,
                             list<Share> &b, list<Share> &c)
{
  Share aa, bb;
  vector<Share> cc(amortize), cc_m(amortize);
  if (!init)
    {
      Init_Configure(P.whoami());
    }

  for (int i= 0; i < sz_offline_batch / amortize; i++)
    {
      vector<stringstream> ss_m(P.nplayers()); // This line is here to make sure
                                               // ss is reinitialized every loop
      stringstream
          ss_r; // This line is here to make sure ss is reinitialized every loop
      for (int j= 0; j < amortize; j++)
        {
          aa= prss.next_share(P);
          bb= prss.next_share(P);
          a.push_back(aa);
          b.push_back(bb);
          cc[j].set_player(P.whoami());

          mult_inner_subroutine_one(aa, bb, cc[j], cc_m[j], ss_m, ss_r, P, prss,
                                    przs);
        }

      mult_inner_subroutine_two(cc, cc_m, ss_m, ss_r, P);

      /* Now add cc[j] onto the output list */
      for (int j= 0; j < amortize; j++)
        {
          c.push_back(cc[j]);
        }
    }
}

void offline_Reduced_squares(Player &P, PRSS &prss, PRZS &przs,
                             list<Share> &a, list<Share> &b,
                             unsigned int rep)
{
  Share aa;
  vector<Share> cc(amortize), cc_m(amortize);
  if (!init)
    {
      Init_Configure(P.whoami());
    }

  while (a.size() < sz_offline_batch * rep)
    {
      vector<stringstream> ss_m(P.nplayers()); // This line is here to make sure
                                               // ss is reinitialized every loop
      stringstream
          ss_r; // This line is here to make sure ss is reinitialized every loop
      for (int j= 0; j < amortize; j++)
        {
          aa= prss.next_share(P);
          a.push_back(aa);
          cc[j].set_player(P.whoami());

          mult_inner_subroutine_one(aa, aa, cc[j], cc_m[j], ss_m, ss_r, P, prss,
                                    przs);
        }

      mult_inner_subroutine_two(cc, cc_m, ss_m, ss_r, P);

      /* Now add cc[j] onto the output list */
      for (int j= 0; j < amortize; j++)
        {
          b.push_back(cc[j]);
        }
    }
}

void offline_Reduced_bits(Player &P, PRSS &prss, PRZS &przs, list<Share> &b,
                          Open_Protocol &OP)
{
  vector<Share> aa(amortize);
  vector<Share> bb(amortize), bb_m(amortize);
  gfp prod, one(1), twoi(2);
  vector<gfp> a2(amortize), macs(0);
  twoi.invert();

  if (!init)
    {
      Init_Configure(P.whoami());
    }

  for (int i= 0; i < sz_offline_batch / amortize;
       i++)
    {                                          /* Essentially run the square protocol to get amortize
number of sharing of a and sharing of b=a^2
*/
      vector<stringstream> ss_m(P.nplayers()); // This line is here to make sure
                                               // ss is reinitialized every loop
      stringstream
          ss_r; // This line is here to make sure ss is reinitialized every loop
      for (int j= 0; j < amortize; j++)
        {
          aa[j]= prss.next_share(P);
          bb[j].set_player(P.whoami());

          mult_inner_subroutine_one(aa[j], aa[j], bb[j], bb_m[j], ss_m, ss_r, P,
                                    prss, przs);
        }

      mult_inner_subroutine_two(bb, bb_m, ss_m, ss_r, P);

      /* Now open the values bb to get the values a2 */
      OP.Open_To_All_Begin(a2, bb, P);
      OP.Open_To_All_End(a2, bb, P);

      /* Now compute v=a/sqrt{a2} assuming a2<>0
 * and then    (v+1)/2
 */
      Share te;
      for (int j= 0; j < amortize; j++)
        {
          if (!a2[j].is_zero())
            {
              gfp rt= a2[j].sqrRoot();
              rt.invert();
              te.mul(aa[j], rt);
              te.add(te, one, macs);
              te.mul(te, twoi);
              b.push_back(te);
            }
        }
    }
}
