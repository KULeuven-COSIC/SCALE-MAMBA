/*
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Mod2Maurer.h"

/* Each share here holds 64 shares */

/* Functions have a few static's to avoid reinitializing memory */

void check_triples(vector<vector<Share2>> &triples, Player &P)
{
  word a, b, c;
  for (unsigned int j= 0; j < triples.size(); j++)
    {
      P.OP2->Open_Bit(a, triples[j][0], P, 0);
      P.OP2->Open_Bit(b, triples[j][1], P, 0);
      P.OP2->Open_Bit(c, triples[j][2], P, 0);
      word te2= ((a & b) - c);
      if (te2 != 0)
        {
          cout << "Triple Error " << endl;
          cout << j << "-th triple out of " << triples.size() << endl;
          cout << " a ";
          triples[j][0].output(cout, true);
          cout << " : " << a << endl;
          cout << " b ";
          triples[j][1].output(cout, true);
          cout << " : " << b << endl;
          cout << " c ";
          triples[j][2].output(cout, true);
          cout << " : " << c << endl;
          abort();
        }
    }
}

/* This produces the sum of Schur products needed by
 * each party in Maurer and Reduced protocols
 */
word schur_sum_prod(const Share2 &aa, const Share2 &bb, const Player &P)
{
  word prod= 0;
  if (Share2::SD.M.shares_per_player(P.whoami()) == 1)
    {
      if (Share2::SD.Schur.get_schur(P.whoami(), 0, 0).is_one())
        {
          prod= aa.get_share(0) & bb.get_share(0);
        }
    }
  else
    {
      prod= 0;
      for (unsigned int i= 0; i < Share2::SD.M.shares_per_player(P.whoami());
           i++)
        {
          for (unsigned int j= 0; j < Share2::SD.M.shares_per_player(P.whoami());
               j++)
            {
              if (Share2::SD.Schur.is_non_zero(P.whoami(), i, j))
                {
                  prod^= aa.get_share(i) & bb.get_share(j);
                }
            }
        }
    }

  return prod;
}

void mult_inner_subroutine_one(const Share2 &aa, const Share2 &bb,
                               vector<Share2> &cc, vector<string> &ss,
                               Player &P)
{

  static vector<string> ee(P.nplayers());
  word prod= schur_sum_prod(aa, bb, P);
  make_shares(cc, prod, P.G);
  for (unsigned int k= 0; k < P.nplayers(); k++)
    {
      if (k != P.whoami())
        {
          ee[k].resize(1 + sizeof(word) * Share2::SD.M.shares_per_player(k));
          cc[k].output(ee[k], 0);
          ss[k]+= ee[k];
        }
    }
}

void mult_inner_subroutine_two(vector<vector<Share2>> &cc,
                               const vector<string> &ss, Player &P)
{
  for (unsigned int k= 0; k < P.nplayers(); k++)
    {
      if (k != P.whoami())
        {
          unsigned int pos= 0;
          for (unsigned int j= 0; j < cc.size(); j++)
            {
              pos+= cc[j][k].input(ss[k], pos);
            }
        }
    }
}

void offline_Maurer_triples(Player &P, PRSS2 &prss,
                            vector<vector<Share2>> &triples,
                            unsigned int batch_size)
{
  triples.resize(batch_size, vector<Share2>(3));

  static vector<vector<Share2>> cc;
  cc.resize(batch_size, vector<Share2>(P.nplayers()));
  vector<string> ss(P.nplayers());

  for (unsigned int j= 0; j < batch_size; j++)
    {
      triples[j][0]= prss.next_share();
      triples[j][1]= prss.next_share();

      mult_inner_subroutine_one(triples[j][0], triples[j][1], cc[j], ss, P);
    }

  P.Send_Distinct_And_Receive(ss, 0);

  mult_inner_subroutine_two(cc, ss, P);

  /* Now have to do the straight linear combination on the cc[j]'s */
  for (unsigned int j= 0; j < batch_size; j++)
    {
      // This also assigns zero, so no need to init with zero
      triples[j][2].set_player(P.whoami());
      for (unsigned int k= 0; k < P.nplayers(); k++)
        {
          triples[j][2].add(cc[j][k]);
        }
    }

  /* TESTING ROUTINE: COMMENT OUT NEVER DELETE !!!  */
  //check_triples(triples, P); // XXXX
}

/* 
 * Opens and checks a bunch of triples (not for debugging)
 */
void check_routine(vector<vector<Share2>> &triples,
                   Player &P, unsigned int num, int verbose= 0)
{
  vector<word> wa, wb, wc;
  vector<Share2> sa(num), sb(num), sc(num);
  for (unsigned int i= 0; i < num; i++)
    {
      sa[i]= triples[i][0];
      sb[i]= triples[i][1];
      sc[i]= triples[i][2];
    }
  P.OP2->Open_To_All(wa, sa, P, 0);
  P.OP2->Open_To_All(wb, sb, P, 0);
  P.OP2->Open_To_All(wc, sc, P, 0);

  for (unsigned int i= 0; i < num; i++)
    {
      word check= wc[i] - (wa[i] & wb[i]);
      if (verbose)
        {
          cout << "\tI = " << i << endl;
          cout << "\t\tShare a: "
               << " : ";
          sa[i].output(cout, true);
          cout << "\t\tShare b: "
               << " : ";
          sb[i].output(cout, true);
          cout << "\t\tShare c: "
               << " : ";
          sc[i].output(cout, true);

          cout << "\t\tValue a: " << wa[i] << endl;
          cout << "\t\tValue b: " << wb[i] << endl;
          cout << "\t\tValue c: " << wc[i] << endl;
          cout << "\t\tCheck  : " << check << endl;
        }
      if (check != 0)
        {
          abort();
        }
    }
}

/* Uses a Fisher-Yates shuffle */
void permute_triples(vector<vector<Share2>> &triples, PRNG &G)
{
  unsigned int perm_length= triples.size() * 64;
  for (int i= perm_length - 1; i >= 0; i--)
    {
      unsigned int j= G.get_uint() % (i + 1);
      // Now convert i and j to share numbers and bit positions
      unsigned int ibit= i & 63, ipos= i >> 6;
      unsigned int jbit= j & 63, jpos= j >> 6;
      swap_shares(triples[ipos][0], triples[jpos][0], ibit, jbit);
      swap_shares(triples[ipos][1], triples[jpos][1], ibit, jbit);
      swap_shares(triples[ipos][2], triples[jpos][2], ibit, jbit);
    }
}

/* Uses a Fisher-Yates shuffle */
void permute_arrays(vector<vector<vector<Share2>>> &arrays, PRNG &G)
{
  unsigned int perm_length= arrays.size();
  for (int i= perm_length - 1; i >= 0; i--)
    {
      unsigned int j= G.get_uint() % (i + 1);
      swap(arrays[i], arrays[j]);
    }
}

void Gen_Checked_Triples(Player &P, PRSS2 &prss,
                         vector<vector<Share2>> &triples,
                         unsigned int B,
                         unsigned int C,
                         unsigned int X,
                         unsigned int L)
{
  string aux;

  // We must have X and C and L divisible by 64, which makes N divisible by 64
  if (L % 64 != 0 || C % 64 != 0 || X % 64 != 0)
    {
      throw invalid_params();
    }
  L= L / 64;
  C= C / 64;
  // This is the actual N divide by 64
  unsigned int N= (X - C) * L;

  // Steps 1 and 2
  // We generate the triples in place for the relevant arrays
  // D1 = triples
  static vector<vector<vector<vector<Share2>>>> Dkj(B - 1);

  offline_Maurer_triples(P, prss, triples, N);

  for (unsigned int k= 0; k < B - 1; k++)
    {
      Dkj[k].resize(L);
      for (unsigned int j= 0; j < L; j++)
        {
          offline_Maurer_triples(P, prss, Dkj[k][j], X);
        }
    }

  // We now generate the PRNG seed for permuting,
  // as the triples are committed in some sense
  uint8_t seed[SEED_SIZE];
  AgreeRandom(P, seed, SEED_SIZE, 0);
  PRNG G;
  G.SetSeedFromRandom(seed);

  // Step 3c
  // Permute the Dkj's
  for (unsigned int k= 0; k < B - 1; k++)
    {
      for (unsigned int j= 0; j < L; j++)
        {
          permute_triples(Dkj[k][j], G);

          /* TESTING ROUTINE: COMMENT OUT NEVER DELETE !!!  */
          //check_triples(Dkj[k][j], P); // XXXX
        }
    }

  // Step 3d
  // Permute the L arrays
  for (unsigned int k= 0; k < B - 1; k++)
    {
      permute_arrays(Dkj[k], G);

      /* TESTING ROUTINE: COMMENT OUT NEVER DELETE !!!  */
      /*
      for (unsigned int j=0; j<L; j++)
        { 
	  check_triples(Dkj[k][j], P);  // XXXX
	}
      */
    }

  // Step 3e
  // Check C of the triples in each set
  for (unsigned int k= 0; k < B - 1; k++)
    {
      for (unsigned int j= 0; j < L; j++)
        {
          check_routine(Dkj[k][j], P, C);
        }
    }
  P.OP2->RunOpenCheck(P, aux, 0);

  // Step 3f
  // Bucket Dk for k=2..B is actually the values
  // in D_{k-1, j, v}  for j=0..L-1, and v=C,...,X-1
  // So the ith element in Dk is
  //    Dkj[k-1][j][v]
  // where j=i%L
  //       v=(i-j)/L+C

  // Step 4
  // Validate the triples in D1, using the triples in D2,..,DB
  // This is algorithm 2.24 from EC 2017 paper by Furukawa et al
  //   eprint 2016/944
  // run B-1 times per triple in D1

  // Stage 1
  // Compute and open rho = [x]+[a] and sigma = [y]+[b]
  static vector<vector<word>> rho_sigma;
  rho_sigma.resize(B - 1, vector<word>(2 * N));
  static vector<Share2> temp, temp2;
  temp.resize(2 * N);
  temp2.resize(N);
  for (unsigned int b= 0; b < B - 1; b++)
    {
      for (unsigned int i= 0; i < N; i++)
        {
          unsigned int j= i % L;
          unsigned int v= (i - j) / L + C;
          temp[i].add(triples[i][0], Dkj[b][j][v][0]);
          temp[i + N].add(triples[i][1], Dkj[b][j][v][1]);
        }
      P.OP2->Open_To_All(rho_sigma[b], temp, P, 0);
    }
  P.OP2->RunOpenCheck(P, aux, 0);
  // Stage 2
  // Compute [z] + [c] + sigma*[a] + rho*[b] + sigma*rho
  static vector<word> ans;
  Share2 te;
  for (unsigned int b= 0; b < B - 1; b++)
    {
      for (unsigned int i= 0; i < N; i++)
        {
          unsigned int j= i % L;
          unsigned int v= (i - j) / L + C;
          // [z]+[c]
          temp2[i].add(triples[i][2], Dkj[b][j][v][2]);
          // +=sigma*[a]
          te.mul(Dkj[b][j][v][0], rho_sigma[b][i + N]);
          temp2[i].add(te);
          // +=rho*[b]
          te.mul(Dkj[b][j][v][1], rho_sigma[b][i]);
          temp2[i].add(te);
          // +=rho*sigma
          temp2[i].add(temp[i], rho_sigma[b][i] & rho_sigma[b][i + N]);
        }
      P.OP2->Open_To_All(ans, temp2, P, 0);
    }
  P.OP2->RunOpenCheck(P, aux, 0);
}

void Mult_Bit(Share2 &z, const Share2 &x, const Share2 &y,
              const vector<Share2> &T, Player &P)
{
  Share2 ed, temp;
  ed.multi_add(T[1], y, T[0], x);

  z= T[2];

  word val;
  P.OP2->Open_Bit(val, ed, P, 2, 1);

  if ((val & 1) == 1)
    {
      z.add(T[1]);
    }
  if ((val & 2) == 2)
    {
      z.add(T[0]);
      if (val == 3)
        {
          z.negate();
        }
    }
}

void Mult_Bits(Share2 &z, const Share2 &x, const Share2 &y,
               const vector<Share2> &T, Player &P)
{
  vector<Share2> ed(2);
  vector<word> vals(2);
  ed[0].add(T[1], y); // delta
  ed[1].add(T[0], x); // epsilon

  z= T[2];

  P.OP2->Open_To_All(vals, ed, P, 2);

  Share2 te;

  // Add epsilon * b
  te.mul(T[1], vals[1]);
  z.add(te);

  // Add delta * a
  te.mul(T[0], vals[0]);
  z.add(te);

  // Add epsilon*delta
  z.add(vals[0] & vals[1]);
}
