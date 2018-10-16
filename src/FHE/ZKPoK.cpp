/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#include "FHE/ZKPoK.h"

/* Kills things off when no longer needed so as to keep memory footprint
 * small
 */

void ZKPoK::Step1(condition type, const FHE_PK &pk, const FFT_Data &PTD,
                  PRNG &G, const vector<gfp> &alpha)
{
  PoKType= type;

  // Make plaintexts first
  m.resize(ZK_stat_sec, Plaintext(PTD));
  vector<bigint> bi_alpha(alpha.size());
  for (unsigned int i= 0; i < alpha.size(); i++)
    {
      to_bigint(bi_alpha[i], alpha[i]);
    }

  for (unsigned int i= 0; i < ZK_stat_sec; i++)
    {
      if (PoKType == Diagonal)
        {
          m[i].assign_zero();
          m[i].set_coeff(0, bi_alpha[i % alpha.size()]);
        }
      else
        {
          m[i].randomize(G);
        }
    }

  // Now make the random coins for the encryptions
  r.resize(ZK_stat_sec, Random_Coins(pk.get_params()));
  if (PoKType == Diagonal)
    {
      for (unsigned int i= 0; i < alpha.size(); i++)
        {
          r[i].generate(G);
        }
      for (unsigned int i= alpha.size(); i < ZK_stat_sec; i++)
        {
          r[i]= r[i % alpha.size()];
        }
    }
  else
    {
      for (unsigned int i= 0; i < ZK_stat_sec; i++)
        {
          r[i].generate(G);
        }
    }

  // Now do the encryptions
  E.resize(ZK_stat_sec, pk.get_params());
  for (unsigned int i= 0; i < ZK_stat_sec; i++)
    {
      pk.encrypt(E[i], m[i], r[i]);
    }

  V= 2 * ZK_stat_sec - 1;

  // Now do the fake plaintexts
  bigint Bp= PTD.get_prime() << (ZK_stat_sec - 1);
  vector<bigint> vv(pk.get_params().phi_m());
  if (PoKType == Diagonal)
    {
      for (unsigned int i= 0; i < pk.get_params().phi_m(); i++)
        {
          vv[i]= 0;
        }
    }

  Z.resize(V, Rq_Element(pk.get_params().FFTD(), polynomial, polynomial));
  for (unsigned int i= 0; i < V; i++)
    {
      if (PoKType == Diagonal)
        {
          vv[0]= randomBnd(G, 2 * Bp + 1) - Bp;
          Z[i].from_vec(vv);
        }
      else
        {
          Z[i].from(UniformGenerator(G, Bp));
        }
    }

  // Now do the fake random coins
  // s in the notes, but we add them directly into T[i])
  T.resize(V, pk.get_params());
  bigint B1= bigint(1) << ZK_stat_sec, B2= 20 * B1;
  for (unsigned int i= 0; i < V; i++)
    {
      T[i].generateUniform(G, B1, B2, B2);
    }

  // Now do the fake encryptions
  A.resize(V, pk.get_params());
  for (unsigned int i= 0; i < V; i++)
    {
      pk.quasi_encrypt(A[i], Z[i], T[i]);
    }
}

// Convert pair into matrix element
int M(unsigned int k, unsigned int l, const vector<int> &e)
{
  if (l > k)
    {
      return 0;
    }
  unsigned int t= k - l; // No need to add one, as we index the vector e from zero
  if (t >= ZK_stat_sec)
    {
      return 0;
    }
  return e[t];
}

void ZKPoK::get_vA(ostream &s) const
{
  for (unsigned int i= 0; i < A.size(); i++)
    {
      A[i].output(s);
    }
}

void ZKPoK::get_vE(ostream &s) const
{
  for (unsigned int i= 0; i < E.size(); i++)
    {
      E[i].output(s);
    }
}

void ZKPoK::get_vT(ostream &s) const
{
  for (unsigned int i= 0; i < T.size(); i++)
    {
      T[i].output(s);
    }
}

void ZKPoK::get_vZ(ostream &s) const
{
  for (unsigned int i= 0; i < Z.size(); i++)
    {
      Z[i].output(s);
    }
}

// Player calls this to enter the each other players vector vA and vE
void ZKPoK::Step1_Step(istream &vE, istream &vA, const FHE_PK &pk)
{
  Ciphertext eq(pk.get_params()), temp(pk.get_params());
  for (unsigned int i= 0; i < E.size(); i++)
    {
      eq.input(vE);
      add(E[i], E[i], eq);
    }
  for (unsigned int i= 0; i < A.size(); i++)
    {
      eq.input(vA);
      add(A[i], A[i], eq);
    }
}

void ZKPoK::Step2(const vector<int> &ee, const FHE_PK &pk)
{
  e= ee;

  // First compute the z vector for this player
  vector<Rq_Element> x(
      ZK_stat_sec, Rq_Element(pk.get_params().FFTD(), polynomial, polynomial));
  for (unsigned int i= 0; i < ZK_stat_sec; i++)
    {
      x[i].from(m[i].get_iterator());
    }
  for (unsigned int i= 0; i < V; i++)
    {
      for (unsigned int j= 0; j < ZK_stat_sec; j++)
        {
          if (M(i, j, e) == 1)
            {
              add(Z[i], Z[i], x[j]);
            }
        }
    }

  // Now compute the T matrix for this player
  for (unsigned int i= 0; i < V; i++)
    {
      for (unsigned int j= 0; j < ZK_stat_sec; j++)
        {
          if (M(i, j, e) == 1)
            {
              add(T[i], T[i], r[j]);
            }
        }
    }

  eq.resize(V, Ciphertext(pk.get_params()));
  for (unsigned int i= 0; i < V; i++)
    {
      pk.quasi_encrypt(eq[i], Z[i], T[i]);
    }

  // Kill off stuff no longer needed
  r.resize(0, Random_Coins(pk.get_params()));
}

// Player calls this to enter the each other players vector vT
void ZKPoK::Step2_Step(istream &vT, istream &vZ, const FHE_PK &pk)
{
  Random_Coins Ti(pk.get_params());
  Rq_Element Zi(pk.get_params().FFTD(), polynomial, polynomial);
  Ciphertext temp(pk.get_params());
  for (unsigned int i= 0; i < T.size(); i++)
    {
      Ti.input(vT);
      Zi.input(vZ);
      add(T[i], T[i], Ti);
      add(Z[i], Z[i], Zi);
      pk.quasi_encrypt(temp, Zi, Ti);
      add(eq[i], eq[i], temp);
    }
}

bool ZKPoK::Step3(const FHE_PK &pk, const FFT_Data &PTD, unsigned int nplayers)
{
  // First the check on  sum_players (d[i]-a[i])-M_e*E = 0
  for (unsigned int i= 0; i < V; i++)
    {
      sub(eq[i], eq[i], A[i]);
      for (unsigned int j= 0; j < ZK_stat_sec; j++)
        {
          if (M(i, j, e) == 1)
            {
              sub(eq[i], eq[i], E[j]);
            }
        }
      if (!eq[i].c0().is_zero() || !eq[i].c1().is_zero())
        {
          cout << "Eq not zero : " << i << endl;
          return false;
        }
    }

  // Check z
  bigint Bz= nplayers * (PTD.get_prime() << (ZK_stat_sec));
  //Rq_Element sz(pk.get_params().FFTD(), polynomial, polynomial);
  for (unsigned int i= 0; i < V; i++)
    {
      if (Z[i].infinity_norm() > Bz)
        {
          cout << "z too big" << endl;
          return false;
        }
      if (PoKType == Diagonal)
        {
          vector<bigint> te= Z[i].to_vec_bigint();
          for (unsigned int j= 1; j < te.size(); j++)
            if (te[j] != 0)
              {
                cout << "Not diag " << endl;
                return false;
              }
        }
    }

  // Check T
  bigint B1= nplayers * (bigint(1) << ZK_stat_sec), B2= B1 * 20;
  //Random_Coins sT(pk.get_params());
  for (unsigned int i= 0; i < V; i++)
    {
      if (T[i].u().infinity_norm() > B1)
        {
          cout << "u too big" << endl;
          return false;
        }
      if (T[i].v().infinity_norm() > B2)
        {
          cout << "v too big" << endl;
          return false;
        }
      if (T[i].w().infinity_norm() > B2)
        {
          cout << "w too big" << endl;
          return false;
        }
    }

  /* Remove stuff no longer needed */
  e.resize(0);
  A.resize(0, Ciphertext(pk.get_params()));
  Z.resize(0);
  T.resize(0, Random_Coins(pk.get_params()));
  eq.resize(0, Ciphertext(pk.get_params()));

  used.resize(ZK_stat_sec);
  for (unsigned int i= 0; i < ZK_stat_sec; i++)
    {
      used[i]= false;
    }
  return true;
}

bool ZKPoK::isempty() const
{
  if (used.size() == 0)
    {
      return true;
    }
  for (unsigned int i= 0; i < ZK_stat_sec; i++)
    {
      if (used[i] == false)
        {
          return false;
        }
    }
  return true;
}

int ZKPoK::get_next_unused() const
{
  for (unsigned int i= 0; i < ZK_stat_sec; i++)
    {
      if (used[i] == false)
        {
          return (int) i;
        }
    }
  return -1;
}

void ZKPoK::get_entry(Plaintext &mess, Ciphertext &ctx, unsigned int i)
{
  if (used[i] == true)
    {
      throw ZKPoK_Used();
    }
  mess= m[i];
  ctx= E[i];
  used[i]= true;
}
