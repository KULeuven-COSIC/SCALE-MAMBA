/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "CAS.h"
#include "Exceptions/Exceptions.h"

void CAS::assign(const CAS &AS)
{
  n= AS.n;
  gamma= AS.gamma;
  delta= AS.delta;
  gamma_minus= AS.gamma_minus;
  delta_plus= AS.delta_plus;
  spec_as_unqualified= AS.spec_as_unqualified;
}

imatrix add_row(const imatrix &M, const vector<int> &v)
{
  int r= M.size(), c= v.size();
  imatrix N(r + 1, vector<int>(c));
  for (int i= 0; i < r; i++)
    {
      for (int j= 0; j < c; j++)
        {
          N[i][j]= M[i][j];
        }
    }
  for (int i= 0; i < c; i++)
    {
      N[r][i]= v[i];
    }
  return N;
}

imatrix remove_row(const imatrix &M, unsigned int k)
{
  int r= M.size(), c= M[0].size();
  imatrix N(r - 1, vector<int>(c));
  for (unsigned int i= 0; i < k; i++)
    {
      for (int j= 0; j < c; j++)
        {
          N[i][j]= M[i][j];
        }
    }
  for (int i= k + 1; i < r; i++)
    {
      for (int j= 0; j < c; j++)
        {
          N[i - 1][j]= M[i][j];
        }
    }
  return N;
}

// Insert row v after row k in M
void replace_row(imatrix &M, unsigned int k, const vector<int> &v)
{
  for (unsigned int i= 0; i < v.size(); i++)
    {
      M[k][i]= v[i];
    }
}

bool check_row_equal(int r, const imatrix &M, const vector<int> &v)
{
  for (unsigned int i= 0; i < v.size(); i++)
    {
      if (v[i] != M[r][i])
        {
          return false;
        }
    }
  return true;
}

void CAS::check_all_sets(int dim, vector<int> &v)
{
  if (dim != -1)
    {
      v[dim]= 0;
      check_all_sets(dim - 1, v);
      v[dim]= 1;
      check_all_sets(dim - 1, v);
    }
  else
    {
      unsigned int j= 0, i= 0;
      bool found_subset= false;
      while (j < delta_plus.size() && !found_subset)
        {
          found_subset= true;
          i= 0;
          while (i < n && found_subset)
            {
              if (delta_plus[j][i] < v[i])
                {
                  found_subset= false;
                }
              i++;
            }
          if (found_subset && !check_row_equal(j, delta, v))
            {
              delta= add_row(delta, v);
            }
          j++;
        }

      j= 0;
      bool found_superset= false;
      while (j < gamma_minus.size() && !found_superset)
        {
          found_superset= true;
          i= 0;
          while (i < n && found_superset)
            {
              if (gamma_minus[j][i] > v[i])
                {
                  found_superset= false;
                }
              i++;
            }
          if (found_superset && !check_row_equal(j, gamma, v))
            {
              gamma= add_row(gamma, v);
            }
          j++;
        }

      if (!found_subset && !found_superset)
        {
          if (spec_as_unqualified)
            {
              gamma= add_row(gamma, v);
            }
          else
            {
              delta= add_row(delta, v);
            }
        }
    }
}

void CAS::set_delta_plus()
{
  delta_plus= imatrix(1, vector<int>(n));
  for (unsigned int i= 0; i < n; i++)
    {
      delta_plus[0][i]= delta[0][i];
    }

  for (unsigned int j= 0; j < delta.size(); j++)
    {
      unsigned int k= 0, i;
      bool new_is_a_subset= false;
      bool new_is_a_superset= false;
      bool found_subset= false;
      while (k < delta_plus.size() && !new_is_a_subset && !new_is_a_superset)
        {
          i= 0;
          new_is_a_superset= true;
          while (i < n && new_is_a_superset)
            {
              if (delta[j][i] < delta_plus[k][i])
                {
                  new_is_a_superset= false;
                }
              i++;
            }
          i= 0;
          new_is_a_subset= true;
          while (i < n && new_is_a_subset)
            {
              if (delta[j][i] > delta_plus[k][i])
                {
                  new_is_a_subset= false;
                }
              i++;
            }
          k++;
        }
      k--;
      if (new_is_a_superset && !new_is_a_subset)
        {
          replace_row(delta_plus, k, delta[j]);
          k++;
          while (k < delta_plus.size())
            {
              i= 0;
              found_subset= true;
              while (i < n && found_subset)
                {
                  if (delta[j][i] < delta_plus[k][i])
                    {
                      found_subset= false;
                    }
                  i++;
                }
              if (found_subset)
                {
                  delta_plus= remove_row(delta_plus, k);
                  k--;
                }
              k++;
            }
        }
      if (!new_is_a_superset && !new_is_a_subset)
        {
          delta_plus= add_row(delta_plus, delta[j]);
        }
    }
}

void CAS::set_gamma_minus()
{
  gamma_minus= imatrix(1, vector<int>(n));
  for (unsigned int i= 0; i < n; i++)
    {
      gamma_minus[0][i]= gamma[0][i];
    }

  for (unsigned int j= 0; j < gamma.size(); j++)
    {
      unsigned int k= 0, i;
      bool new_is_a_subset= false;
      bool new_is_a_superset= false;
      bool found_superset= false;
      while (k < gamma_minus.size() && !new_is_a_subset && !new_is_a_superset)
        {
          i= 0;
          new_is_a_superset= true;
          while (i < n && new_is_a_superset)
            {
              if (gamma[j][i] < gamma_minus[k][i])
                {
                  new_is_a_superset= false;
                }
              i++;
            }
          i= 0;
          new_is_a_subset= true;
          while (i < n && new_is_a_subset)
            {
              if (gamma[j][i] > gamma_minus[k][i])
                {
                  new_is_a_subset= false;
                }
              i++;
            }
          k++;
        }
      k--;
      if (!new_is_a_superset && new_is_a_subset)
        {
          replace_row(gamma_minus, k, gamma[j]);
          k++;
          while (k < gamma_minus.size())
            {
              i= 0;
              found_superset= true;
              while (i < n && found_superset)
                {
                  if (gamma[j][i] > gamma_minus[k][i])
                    {
                      found_superset= false;
                    }
                  i++;
                }
              if (found_superset)
                {
                  gamma_minus= remove_row(gamma_minus, k);
                  k--;
                }
              k++;
            }
        }
      if (!new_is_a_superset && !new_is_a_subset)
        {
          gamma_minus= add_row(gamma_minus, gamma[j]);
        }
    }
}

bool contains_all_x_vector(const imatrix &Sets, int x)
{
  for (unsigned int i= 0; i < Sets.size(); i++)
    {
      bool flag= true;
      for (unsigned int j= 0; j < Sets[i].size(); j++)
        {
          if (Sets[i][j] != x)
            {
              flag= false;
            }
        }
      if (flag)
        {
          return true;
        }
    }
  return false;
}

void CAS::assign(const imatrix &Sets, bool unqualified)
{
  n= Sets[0].size();
  spec_as_unqualified= unqualified;

  if (unqualified)
    { // Check invalid input
      if (contains_all_x_vector(Sets, 1))
        {
          throw invalid_params();
        }
      delta= Sets;
      delta_plus= Sets;
      gamma.resize(1, vector<int>(n));
      for (unsigned int i= 0; i < n; i++)
        {
          gamma[0][i]= 1;
        }
      gamma_minus= gamma;
    }
  else
    { // Check invalid input
      if (contains_all_x_vector(Sets, 0))
        {
          throw invalid_params();
        }
      delta.resize(1, vector<int>(n));
      for (unsigned int i= 0; i < n; i++)
        {
          delta[0][i]= 0;
        }
      delta_plus= delta;
      gamma= Sets;
      gamma_minus= gamma;
    }

  // Now correct the errors
  vector<int> v(n);
  for (unsigned int i= 0; i < n; i++)
    {
      v[i]= 0;
    }
  check_all_sets(n - 1, v);

  set_delta_plus();
  set_gamma_minus();
}

void CAS::generate_sets(unsigned int hw, int dim, vector<int> &v)
{
  if (dim != -1)
    {
      v[dim]= 0;
      generate_sets(hw, dim - 1, v);
      v[dim]= 1;
      generate_sets(hw - 1, dim - 1, v);
    }
  else
    {
      if (hw == 0)
        {
          delta_plus= add_row(delta_plus, v);
        }
    }
}

unsigned int Combinations(unsigned int n, unsigned int r)
{
  if (r > n)
    return 0;

  /** We can use Pascal's triange to determine the amount
      * of combinations. To calculate a single line:
      *
      * v(r) = (n - r) / r
      *
      * Since the triangle is symmetrical, we only need to calculate
      * until r -column.
      */

  unsigned int v= n--;

  for (unsigned int i= 2; i < r + 1; ++i, --n)
    {
      v= v * n / i;
    }

  return v;
}

void CAS::assign(unsigned int nn, unsigned int t)
{
  if (t >= nn)
    {
      throw invalid_params();
    }
  n= nn;
  delta.resize(0);
  gamma.resize(0);
  delta_plus.resize(0);
  gamma_minus.resize(0);
  spec_as_unqualified= true;

  unsigned int ncomb= Combinations(n, t);
  if (ncomb > 50)
    {
      printf("\nAm not going to compute the CAS as number of combinations is too big\n");
      printf("\t%d C %d = %d\n", n, t, ncomb);
      printf("This will mean PRSS calls are replaced by a protocol\n\n");
      return;
    }

  /* For Shamir only delta_plus is defined, which is 
   * done in the function generate_sets
   */
  vector<int> v(n);
  for (unsigned int i= 0; i < n; i++)
    {
      v[i]= 0;
    }
  generate_sets(t, (int) n - 1, v);
}

bool CAS::check_not_q_l(unsigned int hw, int dim, vector<int> &v) const
{
  if (dim != -1)
    {
      v[dim]= 0;
      if (check_not_q_l(hw, dim - 1, v))
        {
          return true;
        }

      v[dim]= 1;
      if (check_not_q_l(hw - 1, dim - 1, v))
        {
          return true;
        }
    }
  else
    {
      if (hw == 0)
        {
          unsigned int i= 0;
          bool found_all_parties= true;
          while (i < n && found_all_parties)
            {
              found_all_parties= false;
              for (unsigned int j= 0; j < delta_plus.size(); j++)
                {
                  if (v[j] == 1)
                    {
                      found_all_parties= found_all_parties || (delta_plus[j][i] == 1);
                    }
                }
              i++;
            }
          return found_all_parties;
        }
    }
  return false;
}

bool CAS::is_q(unsigned int l) const
{
  if (l < n)
    {
      int dim= delta_plus.size();
      vector<int> v(dim);
      for (int i= 0; i < dim; i++)
        {
          v[i]= 0;
        }
      return !check_not_q_l(l, dim - 1, v);
    }
  return false;
}

ostream &operator<<(ostream &s, const CAS &AS)
{
  s << AS.n << endl;
  s << AS.gamma << endl;
  s << AS.delta << endl;
  s << AS.gamma_minus << endl;
  s << AS.delta_plus << endl;
  s << AS.spec_as_unqualified << endl;
  return s;
}

istream &operator>>(istream &s, CAS &AS)
{
  s >> AS.n;
  s >> AS.gamma;
  s >> AS.delta;
  s >> AS.gamma_minus;
  s >> AS.delta_plus;
  s >> AS.spec_as_unqualified;
  return s;
}
