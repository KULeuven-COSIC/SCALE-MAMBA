/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Share2
#define _Share2

/* Class for holding a set of 64 shares of gf2 elements 
 *  - We also use this for shares of single bits.
 *    In which case the sharing is in the lsbit
 */

#include <iostream>
#include <vector>
using namespace std;

#include "ShareData2.h"
#include "System/Player.h"

class Share2
{
  int p;          // The player holding this share
  vector<word> a; // The shares, one share per bit

public:
  static ShareData2 SD;

  static void init_share_data(const ShareData2 &S)
  {
    SD= S;
  }
  static string type_string()
  {
    return "Share2";
  }

  void assign(unsigned int i, vector<word> &sv)
  {
    p= (int) i;
    a= sv;
  }

  void assign(unsigned int i, vector<gf2> &sv)
  {
    p= (int) i;
    a.resize(sv.size());
    for (unsigned int i= 0; i < sv.size(); i++)
      {
        a[i]= sv[i].get();
      }
  }

  // Assign the share value when assigning constant aa
  void assign(const word &aa);
  void assign_zero();
  void assign_one();

  // Same but also assign the player
  void assign(int i, const word &aa)
  {
    set_player(i);
    assign(aa);
  }
  void assign_zero(int i)
  {
    set_player(i);
    assign_zero();
  }
  void assign_one(int i)
  {
    set_player(i);
    assign_one();
  }

  Share2()
  { // To try and create a run time error to capture Share2s
    // without a player assigned
    p= -1;
  }
  Share2(int pp)
  {
    p= pp;
    a.resize(SD.M.shares_per_player(p));
    assign_zero();
  }

  // This is for assigning a constant value aa to the Share2
  Share2(const word &aa, int my_num)
  {
    p= my_num;
    a.resize(SD.M.shares_per_player(p));
    assign(aa);
  }
  // This one is for when the input sv is defintely correct
  Share2(unsigned int i, vector<word> sv)
  {
    p= (int) i;
    a= sv;
  }

  int get_player() const
  {
    return p;
  }
  const vector<word> &get_shares() const
  {
    return a;
  }
  word get_share(unsigned int i) const
  {
    return a[i];
  }
  void set_shares(const vector<word> &aa);
  void set_share(unsigned int i, const word &aa)
  {
    a[i]= aa;
  }
  void set_player(int pp)
  {
    p= pp;
    a.resize(SD.M.shares_per_player(p));
    assign_zero();
  }
  void set_player_and_shares(int pp, const vector<word> &aa);

  /* Arithmetic Routines */
  void mul(const Share2 &S, const word &aa);
  void add(const Share2 &S, const word &aa);
  void sub(const Share2 &S, const word &aa)
  {
    add(S, aa);
  }
  void sub(const word &aa, const Share2 &S)
  {
    add(S, aa);
  }
  void add(const Share2 &S1, const Share2 &S2);
  void sub(const Share2 &S1, const Share2 &S2)
  {
    add(S1, S2);
  }
  void add(const Share2 &S1)
  {
    add(*this, S1);
  }
  void sub(const Share2 &S1)
  {
    sub(*this, S1);
  }
  void add(const word &aa)
  {
    add(*this, aa);
  }

  /* Arithmetic Operators */
  Share2 operator+(const Share2 &x) const
  {
    Share2 res;
    res.add(*this, x);
    return res;
  }
  Share2 operator-(const Share2 &x) const
  {
    Share2 res;
    res.sub(*this, x);
    return res;
  }
  Share2 operator*(const word &x) const
  {
    Share2 res;
    res.mul(*this, x);
    return res;
  }
  Share2 &operator+=(const Share2 &x)
  {
    add(x);
    return *this;
  }
  Share2 &operator*=(const word &x)
  {
    mul(*this, x);
    return *this;
  }

  /* Shift operations */
  void SHL(const Share2 &a, unsigned int n);
  void SHR(const Share2 &a, unsigned int n);

  /* Duplicate the lsb of a into the whole of this */
  void duplicate(const Share2 &a);

  /* Inverts all the bits of x */
  void negate_word(const Share2 &a)
  {
    add(a, 0xFFFFFFFFFFFFFFFFULL);
  }
  /* Inverts the lsb bit of x */
  void negate(const Share2 &a)
  {
    add(a, 0x1ULL);
  }
  void negate()
  {
    negate(*this);
  }

  // Input and output from a stream
  //  - Can do in human or machine only format (later should be faster)
  void output(ostream &s, bool human) const;
  void input(istream &s, bool human);

  // Output directly to a string of chars, assumes enough space is
  // allocated. Returns the number of chars written
  unsigned int output(uint8_t *buff) const
  {
    buff[0]= (char) p;
    unsigned int pos= 1;
    for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
      {
        memcpy(buff + pos, &a[i], sizeof(word));
        pos+= sizeof(word);
      }
    return pos;
  }
  // Input directly from a string of chars
  unsigned int input(const uint8_t *buff)
  {
    p= (int) buff[0];
    unsigned int pos= 1;
    for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
      {
        memcpy(&a[i], buff + pos, sizeof(word));
        pos+= sizeof(word);
      }
    return pos;
  }

  /* Input/Output to a string at position pos.
   * String is already of enough size in both cases.
   * The number of chars read/written is returned
   */
  unsigned int output(string &s, unsigned long pos) const
  {
    return output((uint8_t *) s.c_str() + pos);
  }
  unsigned int input(const string &s, unsigned long pos)
  {
    return input((uint8_t *) s.c_str() + pos);
  }

  /* This takes two single shares and adds them, placing
   * them in bit position 1 and 0 respectively
   */
  void multi_add(const Share2 &x1, const Share2 &y1,
                 const Share2 &x0, const Share2 &y0);

  /* Takes a vector of shares, one from each player and
     * determines the shared value
     *   - i.e. Partially open the shares
     */
  friend word combine(const vector<Share2> &S);

  friend class Open_Protocol2;

  /* Swap the share in bit pos i in the first share
   * with those of bit j in the second share
   */
  friend void swap_shares(Share2 &S1, Share2 &S2, unsigned int i, unsigned int j);

  /* Randomize */
  void randomize(unsigned int online_thread_no, Player &P);
};

// Make a random sharing of a single gf2 value val.
//  Thus the sharing is 63 sharings of zero, and one of val
void make_shares(vector<Share2> &share, const gf2 &val, PRNG &G);

// Does the same for 64 shares at once
void make_shares(vector<Share2> &share, const word &val, PRNG &G);

#endif
