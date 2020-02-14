/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#include "offline_Maurer.h"
#include "config.h"
#include "offline_subroutines.h"

void clear_vector_sstream(vector<stringstream> &ss)
{
  for (unsigned int i= 0; i < ss.size(); i++)
    {
      ss[i]= stringstream();
    }
}

void mult_inner_subroutine_one(const Share &aa, const Share &bb,
                               vector<Share> &cc, vector<stringstream> &ss,
                               Player &P)
{
  gfp prod= schur_sum_prod(aa, bb, P);
  make_shares(cc, prod, P.G);
  for (unsigned int k= 0; k < P.nplayers(); k++)
    {
      if (k != P.whoami())
        {
          cc[k].output(ss[k], false);
        }
    }
}

void mult_inner_subroutine_two(vector<vector<Share>> &cc,
                               const vector<stringstream> &ss, Player &P)
{
  vector<string> sstr(P.nplayers());
  for (unsigned int k= 0; k < P.nplayers(); k++)
    {
      sstr[k]= ss[k].str();
    }
  P.Send_Distinct_And_Receive(sstr);
  for (unsigned int k= 0; k < P.nplayers(); k++)
    {
      if (k != P.whoami())
        {
          istringstream is(sstr[k]);
          for (int j= 0; j < amortize; j++)
            {
              cc[j][k].input(is, false);
            }
        }
    }
}

void offline_Maurer_triples(Player &P, PRSS &prss, list<Share> &a,
                            list<Share> &b, list<Share> &c)
{
  Share aa, bb;
  vector<vector<Share>> cc(amortize, vector<Share>(P.nplayers()));
  vector<stringstream> ss(P.nplayers());
  for (int i= 0; i < sz_offline_batch / amortize; i++)
    {
      clear_vector_sstream(ss);

      for (int j= 0; j < amortize; j++)
        {
          aa= prss.next_share(P);
          bb= prss.next_share(P);
          a.push_back(aa);
          b.push_back(bb);

          mult_inner_subroutine_one(aa, bb, cc[j], ss, P);
        }

      mult_inner_subroutine_two(cc, ss, P);

      /* Now have to do the straight linear combination on the cc[j]'s */
      Share cr, te;
      cr.set_player(P.whoami());
      for (int j= 0; j < amortize; j++)
        {
          cr.assign_zero();
          for (unsigned int k= 0; k < P.nplayers(); k++)
            {
              cr.add(cc[j][k]);
            }
          c.push_back(cr);
        }
    }
}

void offline_Maurer_squares(Player &P, PRSS &prss,
                            list<Share> &a, list<Share> &b,
                            unsigned int rep)
{
  Share aa;
  vector<vector<Share>> bb(amortize, vector<Share>(P.nplayers()));
  vector<string> sstr(P.nplayers());
  vector<stringstream> ss(P.nplayers());
  while (a.size() < sz_offline_batch * rep)
    {
      clear_vector_sstream(ss);

      for (int j= 0; j < amortize; j++)
        {
          aa= prss.next_share(P);
          a.push_back(aa);

          mult_inner_subroutine_one(aa, aa, bb[j], ss, P);
        }

      mult_inner_subroutine_two(bb, ss, P);

      /* Now have to do the linear combination on the cc[j]'s */
      Share br, te;
      br.set_player(P.whoami());
      for (int j= 0; j < amortize; j++)
        {
          br.assign_zero();
          for (unsigned int k= 0; k < P.nplayers(); k++)
            {
              br.add(bb[j][k]);
            }
          b.push_back(br);
        }
    }
}

void offline_Maurer_bits(Player &P, PRSS &prss, list<Share> &b,
                         Open_Protocol &OP)
{
  vector<Share> aa(amortize), bb(amortize);
  vector<gfp> a2(amortize), macs(0);
  vector<vector<Share>> br(amortize, vector<Share>(P.nplayers()));
  gfp prod, one(1), twoi(2);
  twoi.invert();
  vector<string> sstr(P.nplayers());
  vector<stringstream> ss(P.nplayers());
  for (int i= 0; i < sz_offline_batch / amortize; i++)
    {
      /* Essentially run the square protocol to get amortize
       * number of sharing of a and sharing of b=a^2
       */

      clear_vector_sstream(ss);

      for (int j= 0; j < amortize; j++)
        {
          aa[j]= prss.next_share(P);

          mult_inner_subroutine_one(aa[j], aa[j], br[j], ss, P);
        }

      mult_inner_subroutine_two(br, ss, P);

      /* Now have to do the linear combination on the cc[j]'s */
      Share te;
      for (int j= 0; j < amortize; j++)
        {
          bb[j].set_player(P.whoami());
          for (unsigned int k= 0; k < P.nplayers(); k++)
            {
              bb[j].add(br[j][k]);
            }
        }

      /* Now open the values bb to get the values a2 */
      OP.Open_To_All_Begin(a2, bb, P);
      OP.Open_To_All_End(a2, bb, P);

      /* Now compute v=a/sqrt{a2} assuming a2<>0
       * and then    (v+1)/2
       */
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
