/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Schur_Matrices.h"

template<class T>
ostream &operator<<(ostream &s, const Schur_Matrices<T> &S)
{
  s << S.Schur.size() << endl;
  for (unsigned int i= 0; i < S.Schur.size(); i++)
    {
      s << S.Schur[i] << endl;
      s << S.iSchur[i] << endl;
    }
  return s;
}

template<class T>
istream &operator>>(istream &s, Schur_Matrices<T> &S)
{
  unsigned int np;
  s >> np;
  S.Schur.resize(np);
  S.iSchur.resize(np);
  for (unsigned int j= 0; j < np; j++)
    {
      s >> S.Schur[j] >> S.iSchur[j];
    }
  return s;
}

template<class T>
void tensor(vector<T> &z, const vector<T> &x, const vector<T> &y)
{
  for (unsigned int i= 0; i < x.size(); i++)
    {
      for (unsigned int j= 0; j < x.size(); j++)
        {
          z[i * y.size() + j].mul(x[i], y[j]);
        }
    }
}

template<class T>
bool Schur_Matrices<T>::initialize(const MSP<T> &M)
{
  unsigned int cws= 0, k= M.col_dim();
  for (unsigned int i= 0; i < M.nplayers(); i++)
    {
      cws+= M.shares_per_player(i) * M.shares_per_player(i);
    }

  // First form the matrix of Tensor's of each parties share vectors
  vector<vector<T>> TT(k * k, vector<T>(cws + 1));
  vector<T> zz(k * k);
  int Tc= 0, Gr= 0;
  for (unsigned int i= 0; i < M.nplayers(); i++)
    {
      for (unsigned int j0= 0; j0 < M.shares_per_player(i); j0++)
        {
          for (unsigned int j1= 0; j1 < M.shares_per_player(i); j1++)
            {
              tensor(zz, M.G(Gr + j0), M.G(Gr + j1));
              for (unsigned int t= 0; t < k * k; t++)
                {
                  TT[t][Tc]= zz[t];
                }
              Tc++;
            }
        }
      Gr+= M.shares_per_player(i);
    }
  // Add in the target vector
  for (unsigned int i= 0; i < TT.size(); i++)
    {
      TT[i][cws].assign_one();
    }

  // If not a  solution then MSP is not multiplicative
  bool result= Solvable(TT);
  if (result == false)
    {
      return false;
    }

  /* Now back substitute to get how to express the reconstruction
   * vector in terms of the tensors */
  vector<T> z(TT[0].size() - 1);
  BackSubst(z, TT);

  Schur.resize(M.nplayers());
  iSchur.resize(M.nplayers());
  Tc= 0;
  for (unsigned int i= 0; i < M.nplayers(); i++)
    {
      Schur[i].resize(M.shares_per_player(i),
                      vector<T>(M.shares_per_player(i)));
      iSchur[i].resize(M.shares_per_player(i),
                       vector<int>(M.shares_per_player(i)));
      for (unsigned int j0= 0; j0 < Schur[i].size(); j0++)
        {
          for (unsigned int j1= 0; j1 < Schur[i].size(); j1++)
            {
              Schur[i][j0][j1]= z[Tc];
              iSchur[i][j0][j1]= 0;
              if (!z[Tc].is_zero())
                {
                  iSchur[i][j0][j1]= 1;
                }
              Tc++;
            }
        }
    }

  return true;
}

template class Schur_Matrices<gfp>;
template class Schur_Matrices<gf2>;

template istream &operator>>(istream &s, Schur_Matrices<gfp> &M);
template istream &operator>>(istream &s, Schur_Matrices<gf2> &M);
template ostream &operator<<(ostream &s, const Schur_Matrices<gfp> &M);
template ostream &operator<<(ostream &s, const Schur_Matrices<gf2> &M);
