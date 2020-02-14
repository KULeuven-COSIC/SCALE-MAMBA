/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#include "FHE/ZKPoK.h"

/* Kills things off when no longer needed so as to keep memory footprint
 * small
 */

void ZKPoK::Step0(condition type, PoKVersion vers,
                  const FHE_PK &pk, const FFT_Data &PTD,
                  PRNG &G, const vector<gfp> &alpha)
{
  PoKType= type;
  version= vers;

  if (version == HighGear)
    {
      U= ZK_sound_sec;
      V= 2 * U - 1;
      ssec= U;
      if (alpha.size() > U)
        { // Have problem here as we assume this below for the Diag case
          throw bad_value();
        }
    }
  else if (PoKType != Diagonal)
    {
      int lg2N= CEIL_LOG2(2 * pk.get_params().phi_m() + 1);
      V= DIV_CEIL(ZK_sound_sec + 2, lg2N);
      U= 2 * V;
      ssec= V * lg2N;
    }
  else
    { // Do TopGear diag proofs
      V= 16;
      U= alpha.size();
      ssec= V;
    }

  // Make plaintexts first
  m.resize(U, Plaintext(PTD));
  E.resize(U, pk.get_params());

  if (single == false || prover == true)
    {
      vector<bigint> bi_alpha(alpha.size());
      if (PoKType == Diagonal)
        {
          for (unsigned int i= 0; i < alpha.size(); i++)
            {
              to_bigint(bi_alpha[i], alpha[i]);
            }
        }

      for (unsigned int i= 0; i < U; i++)
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
      r.resize(U, Random_Coins(pk.get_params()));
      if (PoKType == Diagonal && version == HighGear)
        {
          for (unsigned int i= 0; i < alpha.size(); i++)
            {
              r[i].generate(G);
            }
          for (unsigned int i= alpha.size(); i < U; i++)
            {
              r[i]= r[i % alpha.size()];
            }
        }
      else
        {
          for (unsigned int i= 0; i < U; i++)
            {
              r[i].generate(G);
            }
        }

      // Now do the encryptions
      for (unsigned int i= 0; i < U; i++)
        {
          pk.encrypt(E[i], m[i], r[i]);
        }
    }
}

void ZKPoK::Step1(const FHE_PK &pk, const FFT_Data &PTD, PRNG &G)
{
  Z.resize(V, Rq_Element(pk.get_params().FFTD(), polynomial, polynomial));
  T.resize(V, pk.get_params());
  A.resize(V, pk.get_params());

  if (single == false || prover == true)
    {
      // We first do the fake plaintexts
      bigint Bp= PTD.get_prime() << (ZK_ZK_sec - 1);
      vector<bigint> vv(pk.get_params().phi_m());
      if (PoKType == Diagonal)
        {
          for (unsigned int i= 0; i < pk.get_params().phi_m(); i++)
            {
              vv[i]= 0;
            }
        }

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
      bigint B1= bigint(1) << ZK_ZK_sec, B2= NewHopeB * B1;
      for (unsigned int i= 0; i < V; i++)
        {
          T[i].generateUniform(G, B1, B2, B2);
        }

      // Now do the fake encryptions
      for (unsigned int i= 0; i < V; i++)
        {
          pk.quasi_encrypt(A[i], Z[i], T[i]);
        }
    }
}

// Convert pair into matrix element
//   For HighGear and TopGear Diagonal this is just 0/1 entries
//   For TopGear non Digaonal we use -1 to represent zero, as
//   e[i] represents the exponent of the thing in the
//   challenge set, i.e. X^{e[i]}
int ZKPoK::M(unsigned int k, unsigned int l, const vector<int> &e)
{
  if (version != TopGear)
    {
      if (l > k)
        {
          return 0;
        }
      unsigned int t= k - l; // No need to add one, as we index the vector e from zero
      if (t >= U)
        {
          return 0;
        }
      return e[t];
    }

  return e[k * U + l];
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

// Player calls this to enter the each other players vector vE
void ZKPoK::Step0_Step(istream &vE, const FHE_PK &pk)
{
  Ciphertext eq(pk.get_params()), temp(pk.get_params());
  for (unsigned int i= 0; i < E.size(); i++)
    {
      if (single == true && prover == false)
        {
          E[i].input(vE);
        }
      else if (single == false)
        {
          eq.input(vE);
          add(E[i], E[i], eq);
        }
    }
}

// Player calls this to enter the each other players vector vA
void ZKPoK::Step1_Step(istream &vA, const FHE_PK &pk)
{
  Ciphertext eq(pk.get_params()), temp(pk.get_params());
  for (unsigned int i= 0; i < A.size(); i++)
    {
      if (single == true && prover == false)
        {
          A[i].input(vA);
        }
      else if (single == false)
        {
          eq.input(vA);
          add(A[i], A[i], eq);
        }
    }
}

// Player calls this to enter the each other players vector vT
void ZKPoK::Step2_Step(istream &vT, istream &vZ, const FHE_PK &pk)
{
  Random_Coins Ti(pk.get_params());
  Rq_Element Zi(pk.get_params().FFTD(), polynomial, polynomial);
  Ciphertext temp(pk.get_params());

  for (unsigned int i= 0; i < T.size(); i++)
    {
      if (single == true && prover == false)
        {
          T[i].input(vT);
          Z[i].input(vZ);
          pk.quasi_encrypt(eq[i], Z[i], T[i]);
        }
      else if (single == false)
        {
          Ti.input(vT);
          Zi.input(vZ);
          add(T[i], T[i], Ti);
          add(Z[i], Z[i], Zi);
          pk.quasi_encrypt(temp, Zi, Ti);
          add(eq[i], eq[i], temp);
        }
    }
}

void ZKPoK::Generate_e(vector<int> &e, uint8_t seed[SEED_SIZE])
{
  if (version == TopGear)
    {
      e.resize(U * V);
    }
  else
    {
      e.resize(U);
    }
  PRNG G;
  G.SetSeedFromRandom(seed);
  if (version == HighGear || PoKType == Diagonal)
    {
      unsigned int c= 0;
      while (c < e.size())
        {
          unsigned char x= G.get_uchar();
          for (unsigned int j= 0; j < 8; j++)
            {
              if (c < e.size())
                {
                  e[c]= (x >> j) & 1;
                  c++;
                }
            }
        }
    }
  else
    { // Now standard TopGear
      unsigned int N= E[0].get_params().phi_m();
      for (unsigned int j= 0; j < e.size(); j++)
        {
          unsigned int t= G.get_uint();
          e[j]= t % (2 * N + 1);
          if ((unsigned int) e[j] == 2 * N)
            {
              e[j]= -1;
            }
        }
    }
}

void ZKPoK::Step2(const vector<int> &ee, const FHE_PK &pk)
{
  e= ee;
  eq.resize(V, Ciphertext(pk.get_params()));

  if (single == false || prover == true)
    {
      // First compute the z vector for this player and the T matrix
      vector<Rq_Element> x(U, Rq_Element(pk.get_params().FFTD(), polynomial, polynomial));

      Rq_Element temp_rq;
      Random_Coins temp_rc(pk.get_params());
      for (unsigned int i= 0; i < U; i++)
        {
          x[i].from(m[i].get_iterator());
        }
      for (unsigned int i= 0; i < V; i++)
        {
          for (unsigned int j= 0; j < U; j++)
            {
              int mm= M(i, j, e);
              if (version == TopGear && PoKType != Diagonal)
                {
                  if (mm >= 0)
                    {
                      mul_by_X_i(temp_rq, x[j], mm);
                      mul_by_X_i(temp_rc, r[j], mm);
                      add(Z[i], Z[i], temp_rq);
                      add(T[i], T[i], temp_rc);
                    }
                }
              else
                {
                  if (mm == 1)
                    {
                      add(Z[i], Z[i], x[j]);
                      add(T[i], T[i], r[j]);
                    }
                }
            }
        }

      for (unsigned int i= 0; i < V; i++)
        {
          pk.quasi_encrypt(eq[i], Z[i], T[i]);
        }
    }
}

bool ZKPoK::Step3(const FHE_PK &pk, const FFT_Data &PTD, unsigned int nplayers)
{

  if (single == false || prover == false)
    {
      Ciphertext temp(pk.get_params());

      // First the check on  sum_players (d[i]-a[i])-M_e*E = 0
      for (unsigned int i= 0; i < V; i++)
        {
          sub(eq[i], eq[i], A[i]);
          for (unsigned int j= 0; j < U; j++)
            {
              int mm= M(i, j, e);
              if (version == TopGear && PoKType != Diagonal)
                {
                  if (mm >= 0)
                    {
                      mul_by_X_i(temp, E[j], mm);
                      sub(eq[i], eq[i], temp);
                    }
                }
              else
                {
                  if (mm == 1)
                    {
                      sub(eq[i], eq[i], E[j]);
                    }
                }
            }
          if (!eq[i].c0().is_zero() || !eq[i].c1().is_zero())
            {
              cout << "Eq not zero : " << i << endl;
              return false;
            }
        }

      // Check z
      bigint Bz= (PTD.get_prime() << (ZK_ZK_sec));
      bigint B1= (bigint(1) << (ZK_ZK_sec + 1));
      if (single == false)
        {
          B1*= nplayers;
          Bz*= nplayers;
        }
      bigint B2= B1 * NewHopeB;
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
      //  Random_Coins sT(pk.get_params());
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
    }

  /* Remove stuff no longer needed */
  e.resize(0);
  A.resize(0, Ciphertext(pk.get_params()));
  Z.resize(0);
  T.resize(0, Random_Coins(pk.get_params()));
  eq.resize(0, Ciphertext(pk.get_params()));

  used.resize(U);
  for (unsigned int i= 0; i < used.size(); i++)
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
  for (unsigned int i= 0; i < used.size(); i++)
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
  for (unsigned int i= 0; i < used.size(); i++)
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
