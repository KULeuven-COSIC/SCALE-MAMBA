/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _FHE_Keys
#define _FHE_Keys

/* These are standardly generated FHE public and private key pairs */

#include "FHE/Ciphertext.h"
#include "FHE/FHE_Params.h"
#include "FHE/Plaintext.h"
#include "FHE/Random_Coins.h"
#include "FHE/Rq_Element.h"

class FHE_PK;
class Ciphertext;

class FHE_SK
{
  Rq_Element sk;
  const FHE_Params *params;
  bigint pr;

public:
  const FHE_Params &get_params() const
  {
    return *params;
  }

  bigint p() const
  {
    return pr;
  }

  // secret key always on lower level
  void assign(const Rq_Element &s)
  {
    sk= s;
    sk.lower_level();
  }

  FHE_SK();
  FHE_SK(const FHE_Params &pms, const bigint &p= 0)
      : sk(pms.FFTD(), evaluation, evaluation)
  {
    params= &pms;
    pr= p;
  }

  FHE_SK(const FHE_PK &pk);

  // Rely on default copy constructor/assignment

  const Rq_Element &s() const
  {
    return sk;
  }

  // Assumes params is set out of band
  friend ostream &operator<<(ostream &s, const FHE_SK &SK)
  {
    SK.sk.output(s, false);
    return s;
  }
  friend istream &operator>>(istream &s, FHE_SK &SK)
  {
    SK.sk.input(s);
    return s;
  }

  // Assumes Ring and prime of mess have already been set correctly
  // Ciphertext c must be at level 0 or an error occurs
  //            c must have same params as SK
  void decrypt(Plaintext &mess, const Ciphertext &c) const;

  Plaintext decrypt(const Ciphertext &c, const FFT_Data &FieldD);

  void decrypt_any(Plaintext &mess, const Ciphertext &c);

  // Make distributed secret key
  vector<Rq_Element> make_distributed_key(unsigned int, PRNG &G) const;

  // Three stage procedure for Distributed Decryption
  //  - First stage produces my shares
  //  - Second stage adds in another players shares, do this once for each other
  //  player
  //  - Third stage outputs the message by executing
  //        mess.set_poly_mod(vv,mod)
  //    where mod p0 and mess is Plaintext
  void dist_decrypt_1(vector<bigint> &vv, const Ciphertext &ctx, int player_number) const;
  void dist_decrypt_2(vector<bigint> &vv, const vector<bigint> &vv1) const;

  // Same as dist_decrypt_1 but does not muly by p, and also returns the
  // randomness used by this player
  void dist_decrypt_1a(vector<bigint> &vv, vector<bigint> &f,
                       const Ciphertext &ctx, int player_number) const;

  friend void KeyGen(FHE_PK &PK, FHE_SK &SK, PRNG &G);

  /* Add secret key onto the existing one
   *   Used for adding distributed keys together
   *   a,b,c must have same params otherwise an error
   */
  friend void add(FHE_SK &a, const FHE_SK &b, const FHE_SK &c);

  FHE_SK operator+(const FHE_SK &x)
  {
    FHE_SK res(*params, pr);
    add(res, *this, x);
    return res;
  }
  FHE_SK &operator+=(const FHE_SK &x)
  {
    add(*this, *this, x);
    return *this;
  }
};

class FHE_PK
{
  Rq_Element a0, b0;
  Rq_Element Sw_a, Sw_b;
  const FHE_Params *params;
  bigint pr;

public:
  const FHE_Params &get_params() const
  {
    return *params;
  }

  bigint p() const
  {
    return pr;
  }

  void assign(const Rq_Element &a, const Rq_Element &b, const Rq_Element &sa,
              const Rq_Element &sb)
  {
    a0= a;
    b0= b;
    Sw_a= sa;
    Sw_b= sb;
  }

  FHE_PK(const FHE_Params &pms, const bigint &p= 0)
      : a0(pms.FFTD(), evaluation, evaluation),
        b0(pms.FFTD(), evaluation, evaluation),
        Sw_a(pms.FFTD(), evaluation, evaluation),
        Sw_b(pms.FFTD(), evaluation, evaluation)
  {
    params= &pms;
    pr= p;
  }

  // Rely on default copy constructor/assignment

  const Rq_Element &a() const
  {
    return a0;
  }
  const Rq_Element &b() const
  {
    return b0;
  }

  const Rq_Element &as() const
  {
    return Sw_a;
  }
  const Rq_Element &bs() const
  {
    return Sw_b;
  }

  // c must have same params as PK and rc
  void encrypt(Ciphertext &c, const Plaintext &mess,
               const Random_Coins &rc) const;
  void quasi_encrypt(Ciphertext &c, const Rq_Element &mess,
                     const Random_Coins &rc) const;

  Ciphertext encrypt(const Plaintext &mess, const Random_Coins &rc) const;
  Ciphertext encrypt(const Plaintext &mess) const;

  friend void KeyGen(FHE_PK &PK, FHE_SK &SK, PRNG &G);

  Rq_Element sample_secret_key(PRNG &G);
  void KeyGen(Rq_Element &sk, PRNG &G);

  // params setting is done out of these IO/pack/unpack functions

  friend ostream &operator<<(ostream &s, const FHE_PK &PK)
  {
    PK.a0.output(s, false);
    PK.b0.output(s, false);
    PK.Sw_a.output(s, false);
    PK.Sw_b.output(s, false);
    return s;
  }

  friend istream &operator>>(istream &s, FHE_PK &PK)
  {
    PK.a0.input(s);
    PK.b0.input(s);
    PK.Sw_a.input(s);
    PK.Sw_b.input(s);
    return s;
  }
};

// PK and SK must have the same params, otherwise an error
void KeyGen(FHE_PK &PK, FHE_SK &SK, PRNG &G);

#endif
