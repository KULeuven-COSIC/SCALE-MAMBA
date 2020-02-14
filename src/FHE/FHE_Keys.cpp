/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "FHE_Keys.h"
#include "Ciphertext.h"
#include "config.h"

FHE_SK::FHE_SK(const FHE_PK &pk)
    : FHE_SK(pk.get_params(), pk.p()) {}

void add(FHE_SK &a, const FHE_SK &b, const FHE_SK &c)
{
  if (a.params != b.params)
    {
      throw data_mismatch();
    }
  if (a.params != c.params)
    {
      throw data_mismatch();
    }

  add(a.sk, b.sk, c.sk);
}

void KeyGen(FHE_PK &PK, FHE_SK &SK, PRNG &G)
{
  if (PK.params != SK.params)
    {
      throw data_mismatch();
    }
  if (PK.pr != SK.pr)
    {
      throw data_mismatch();
    }

  Rq_Element sk= PK.sample_secret_key(G);
  SK.assign(sk);
  PK.KeyGen(sk, G);
}

Rq_Element FHE_PK::sample_secret_key(PRNG &G)
{
  Rq_Element sk= FHE_SK(*this).s();
  // Generate the secret key
  if ((*params).get_hwt() > 0)
    {
      sk.from_vec((*params).sample_Hwt(G));
    }
  else
    {
      sk.from(GaussianGenerator(G));
    }

  return sk;
}

void FHE_PK::KeyGen(Rq_Element &sk, PRNG &G)
{
  FHE_PK &PK= *this;

  // Generate the main public key
  PK.a0.randomize(G);

  // b0=a0*s+p*e0
  Rq_Element e0((*PK.params).FFTD(), evaluation, evaluation);
  e0.from(GaussianGenerator(G));
  mul(PK.b0, PK.a0, sk);
  mul(e0, e0, PK.pr);
  add(PK.b0, PK.b0, e0);

  // Generating the switching key data
  PK.Sw_a.randomize(G);

  // bs=as*s+p*es
  Rq_Element es((*PK.params).FFTD(), evaluation, evaluation);
  es.from(GaussianGenerator(G));
  mul(PK.Sw_b, PK.Sw_a, sk);
  mul(es, es, PK.pr);
  add(PK.Sw_b, PK.Sw_b, es);

  // Lowering level as we only decrypt at level 0
  sk.lower_level();

  // bs=bs-p1*s^2
  Rq_Element s2;
  mul(s2, sk, sk); // Mult at level 0
  s2.mul_by_p1();  // This raises back to level 1
  sub(PK.Sw_b, PK.Sw_b, s2);
}

void FHE_PK::encrypt(Ciphertext &c, const Plaintext &mess,
                     const Random_Coins &rc) const
{
  if (&c.get_params() != params)
    {
      throw data_mismatch();
    }
  if (&rc.get_params() != params)
    {
      throw data_mismatch();
    }
  if (pr == 2)
    {
      throw data_mismatch();
    }

  Rq_Element mm((*params).FFTD(), polynomial, polynomial);
  mm.from(mess.get_iterator());

  quasi_encrypt(c, mm, rc);
}

void FHE_PK::quasi_encrypt(Ciphertext &c, const Rq_Element &mess,
                           const Random_Coins &rc) const
{
  if (&c.get_params() != params)
    {
      throw data_mismatch();
    }
  if (&rc.get_params() != params)
    {
      throw data_mismatch();
    }

  Rq_Element ed, edd, c0, c1, aa;

  // c1=a0*u+p*v
  mul(aa, a0, rc.u());
  mul(ed, rc.v(), pr);
  add(c1, aa, ed);

  // c0 = b0 * u + p * w + mess
  mul(c0, b0, rc.u());
  mul(edd, rc.w(), pr);
  add(edd, edd, mess);
  edd.change_rep(evaluation, evaluation);
  add(c0, c0, edd);

  c.set(c0, c1);
}

Ciphertext FHE_PK::encrypt(const Plaintext &mess,
                           const Random_Coins &rc) const
{
  Ciphertext res(*params);
  encrypt(res, mess, rc);
  return res;
}

Ciphertext FHE_PK::encrypt(const Plaintext &mess) const
{
  Random_Coins rc(*params);
  PRNG G;
  G.ReSeed(0);
  rc.generate(G);
  return encrypt(mess, rc);
}

void FHE_SK::decrypt(Plaintext &mess, const Ciphertext &c) const
{
  if (&c.get_params() != params)
    {
      throw data_mismatch();
    }
  if (pr == 2)
    {
      throw data_mismatch();
    }

  Rq_Element ans;

  mul(ans, c.c1(), sk);
  sub(ans, c.c0(), ans);
  ans.change_rep(polynomial);
  mess.set_poly_mod(ans.get_iterator(), ans.get_modulus());
}

Plaintext FHE_SK::decrypt(const Ciphertext &c, const FFT_Data &FieldD)
{
  Plaintext res(FieldD);
  decrypt_any(res, c);
  return res;
}

void FHE_SK::decrypt_any(Plaintext &res, const Ciphertext &c)
{
  if (sk.level())
    sk.lower_level();
  if (c.level())
    {
      Ciphertext cc= c;
      cc.Scale(res.get_field().get_prime());
      decrypt(res, cc);
    }
  else
    decrypt(res, c);
}

vector<Rq_Element> FHE_SK::make_distributed_key(unsigned int n, PRNG &G) const
{
  vector<Rq_Element> si(n, Rq_Element((*params).FFTD(), evaluation, evaluation));
  si[0]= sk;
  for (unsigned int i= 1; i < n; i++)
    {
      si[i].randomize(G, 0);
      sub(si[0], si[0], si[i]);
    }
  return si;
}

/* Distributed Decryption Stuff */
void FHE_SK::dist_decrypt_1(vector<bigint> &vv, const Ciphertext &ctx, int player_number) const
{
  // Need Ciphertext to be at level 0, so we force this here
  Ciphertext cc= ctx;
  cc.Scale(pr);

  // First do the basic decryption
  Rq_Element dec_sh;
  mul(dec_sh, cc.c1(), sk);
  if (player_number == 0)
    {
      sub(dec_sh, cc.c0(), dec_sh);
    }
  else
    {
      dec_sh.negate();
    }

  // Now convert to a vector of bigint's and add the required randomness
  bigint Bd= ((*params).B() << DD_stat_sec) / pr;
  Bd= Bd / 2; // make slightly smaller due to rounding issues

  dec_sh.to_vec_bigint(vv);
  if (vv.size() != params->phi_m())
    throw length_error("wrong length of ring element");
  bigint mod= (*params).p0();
  PRNG G;
  G.ReSeed(0);
  bigint mask;
  bigint two_Bd= 2 * Bd;
  for (unsigned int i= 0; i < (*params).phi_m(); i++)
    {
      mask= randomBnd(G, two_Bd);
      mask-= Bd;
      mask*= pr;
      vv[i]+= mask;
      vv[i]%= mod;
      if (vv[i] < 0)
        {
          vv[i]+= mod;
        }
    }
}

void FHE_SK::dist_decrypt_1a(vector<bigint> &vv, vector<bigint> &f,
                             const Ciphertext &ctx, int player_number) const
{
  // Need Ciphertext to be at level 0, so we force this here
  Ciphertext cc= ctx;
  cc.Scale(pr);

  // First do the basic decryption
  Rq_Element dec_sh;
  mul(dec_sh, cc.c1(), sk);
  if (player_number == 0)
    {
      sub(dec_sh, cc.c0(), dec_sh);
    }
  else
    {
      dec_sh.negate();
    }

  // Now convert to a vector of bigint's and add the required randomness
  bigint Bd= ((*params).B() << DD_stat_sec);
  Bd= Bd / 2; // make slightly smaller due to rounding issues

  dec_sh.to_vec_bigint(vv);
  if (vv.size() != params->phi_m())
    throw length_error("wrong length of ring element");
  bigint mod= (*params).p0();
  PRNG G;
  G.ReSeed(0);
  bigint two_Bd= 2 * Bd;
  for (unsigned int i= 0; i < (*params).phi_m(); i++)
    {
      f[i]= randomBnd(G, two_Bd);
      f[i]-= Bd;
      vv[i]-= f[i];
      vv[i]%= mod;
      if (vv[i] < 0)
        {
          vv[i]+= mod;
        }
    }
}

void FHE_SK::dist_decrypt_2(vector<bigint> &vv,
                            const vector<bigint> &vv1) const
{
  bigint mod= (*params).p0();
  for (unsigned int i= 0; i < (*params).phi_m(); i++)
    {
      vv[i]+= vv1[i];
      vv[i]%= mod;
    }
}
