/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

#include "openssl/err.h"
#include "openssl/ssl.h"

#include "FHE/FHE_Keys.h"
#include "FHE/FHE_Params.h"
#include "GC/SimplifyCircuit.h"
#include "LSSS/Share.h"
#include "OT/aBitVector.h"
#include "System/Init.h"
#include "config.h"

// This is a dummy ensuring that none of the functions in this file use the actual `cin`
// but all use the `istream` passed as an argument.
struct Dummy
{
};
static Dummy cin= Dummy();

int input_YN(std::istream &cin)
{
  string str= "";
  do
    {
      cout << "Enter Y/N" << endl;
      cin >> str;
      if (str == "y" || str == "Y")
        {
          return 1;
        }
      if (str == "n" || str == "N")
        {
          return 0;
        }
    }
  while (0 == 0);
}

/* Initializes gfp::init_field as well */
void init_FHE_Params(FHE_Params &params, bigint &pr, bigint &p0, bigint &p1,
                     unsigned int &N,
                     int lg2p, unsigned int n, unsigned int hwt)
{
// Assumes here pr=0 if we want it to be so
#ifdef TOP_GEAR
  Generate_Parameters(N, p0, p1, pr, lg2p, n, TopGear, hwt);
#else
  Generate_Parameters(N, p0, p1, pr, lg2p, n, HighGear, hwt);
#endif
  Ring Rg(2 * N);
  gfp::init_field(pr);

  params.set(Rg, p0, p1, hwt, n);
}

void init_FHE_Keys(vector<Rq_Element> &si, FHE_PK &pk,
                   unsigned int n, const FHE_Params &params, const bigint &pr)
{
  FHE_SK sk(params, pr);

  PRNG G;
  G.ReSeed(0);

  KeyGen(pk, sk, G);

  // Now create the distributed secret key
  si= sk.make_distributed_key(n, G);
}

void init_FHE(bigint &pr, int lg2p, unsigned int n)
{
  bigint p0, p1;
  FHE_Params params;
  unsigned int N, hwt= HwtSK;
  init_FHE_Params(params, pr, p0, p1, N, lg2p, n, hwt);

  FHE_PK pk(params, pr);
  vector<Rq_Element> si;
  init_FHE_Keys(si, pk, n, params, pr);

  // Now output the data
  FHE_SK sk(params, pr);
  for (unsigned int i= 0; i < n; i++)
    {
      stringstream ss;
      ss << "Data/FHE-Key-" << i << ".key";
      ofstream outk(ss.str().c_str());
      if (outk.fail())
        {
          throw file_error(ss.str());
        }
      sk.assign(si[i]);
      outk << N << " " << p0 << " " << p1 << " " << pr << " " << hwt << endl
           << ":";
      outk << pk;
      outk << sk << endl;
      outk.close();
    }
}

void init_certs(std::istream &cin)
{
  // Initialize the SSL library
  OPENSSL_init_ssl(
      OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL);
  // SSL_CTX *ctx = InitServerCTX();

  ofstream output("Data/NetworkData.txt");
  if (output.fail())
    {
      throw file_error("Data/NetworkData.txt");
    }

  cout << "Enter the name of the root CA (e.g. RootCA)" << endl;
  string CAName;
  cin >> CAName;

  string str= "Cert-Store/" + CAName + ".crt";
  ifstream cert_file(str);
  if (cert_file.fail())
    {
      throw file_error(str);
    }
  stringstream cert_buff;
  cert_buff << cert_file.rdbuf();
  cout << "Cert is\n"
       << cert_buff.str() << endl;
  BIO *bio_mem= BIO_new(BIO_s_mem());
  BIO_puts(bio_mem, cert_buff.str().c_str());
  X509 *x509= PEM_read_bio_X509(bio_mem, NULL, NULL, NULL);
  EVP_PKEY *pkey= X509_get_pubkey(x509);
  BIO_free(bio_mem);
  X509_free(x509);
  cert_file.close();

  output << CAName << ".crt\n";

  int n= -1;
  while (n < 2 || n > 100)
    {
      cout << "Number of players (2--100) : " << endl;
      cin >> n;
    }
  output << n << endl;

  cout << "For each player now enter:\n";
  cout << "\t IP address (127.0.0.1) for localhost\n";
  cout << "\t The certificate name for this player\n";

  for (int i= 0; i < n; i++)
    {
      cout << "\n\nPlayer " << i << "\n";
      cout << "IP Address (in format "
              "127.0.0.1"
              ") "
           << endl;
      cin >> str;
      output << i << " " << str << " ";
      cout << "Name of certificate (e.g. "
              "Player1.crt"
              ")"
           << endl;
      cin >> str;
      output << str << " ";
      str= "Cert-Store/" + str;
      ifstream player_cert_file(str);
      if (player_cert_file.fail())
        {
          throw file_error(str);
        }
      stringstream player_cert_buff;
      player_cert_buff << player_cert_file.rdbuf();
      cout << "Cert is\n"
           << player_cert_buff.str() << endl;
      bio_mem= BIO_new(BIO_s_mem());
      BIO_puts(bio_mem, player_cert_buff.str().c_str());
      X509 *player_x509= PEM_read_bio_X509(bio_mem, NULL, NULL, NULL);
      char buffer[256];
      X509_NAME_get_text_by_NID(X509_get_subject_name(player_x509), NID_commonName,
                                buffer, 256);
      cout << "Common name in certificate is " << buffer << endl;
      output << buffer << endl;
      int j= 0;
      while (buffer[j] != 0)
        {
          if (buffer[j] == ' ')
            {
              cout << "Common name may not contain spaces" << endl;
              abort();
            }
          j++;
        }
      int fl= X509_verify(player_x509, pkey);
      if (fl == 0)
        {
          cout << "Cerfificate does not verify" << endl;
          abort();
        }
      X509_free(player_x509);
      BIO_free(bio_mem);
      player_cert_file.close();
    }
  EVP_PKEY_free(pkey);

  /* XXXX
  cout << "Fake offline?" << endl;
  output << input_YN() << endl;
  cout << "Fake sacrifice?" << endl;
  output << input_YN() << endl;
  */

  // Choose non-fake in both cases
  output << 0 << endl;
  output << 0 << endl;

  output.close();
}

void init_replicated(ShareData &SD, ShareData2 &SD2, unsigned int n, ReplicationMode mode, unsigned int t, OfflineType offline_type, imatrix Sets)
{
  CAS AS;
  switch (mode)
    {
      case ReplicationMode::QualifiedSets:
      case ReplicationMode::UnqualifiedSets:
        {
          bool unqualified= mode == ReplicationMode::UnqualifiedSets;
          AS.assign(Sets, unqualified);
          break;
        }
      case ReplicationMode::Threshold:
        {
          cout << "Threshold..." << endl;
          AS.assign(n, t);
          cout << "AS = " << AS << endl;
          break;
        }
    }

  SD.Initialize_Replicated(AS, offline_type);
  SD2.Initialize_Replicated(AS);
}

void init_Q2_MSP(ShareData &SD, vector<unsigned int> ns, vector<vector<gfp>> &Gen)
{
  cout << "Generator matrix is: " << Gen << endl;
  cout << "Target vector is [1,1,...,1]" << endl;

  MSP<gfp> M(Gen, ns);
  SD.Initialize_Q2(M);

  if (Gen.size() != SD.M.row_dim())
    {
      cout << "Note, the original MSP was not multiplicative" << endl;
      cout << "We therefore extended it to produce the following (equivalent) one"
           << endl;
      cout << SD.M << endl;
    }
}

SecretSharing init_secret_sharing(ShareType v, bigint &p, int lg2p,
                                  unsigned int t, unsigned int n,
                                  vector<unsigned int> ns, vector<vector<gfp>> &Gen,
                                  ReplicationMode mode, OfflineType offline_type,
                                  imatrix Sets)
{
  cout << "Number of players = " << n << endl;

  switch (v)
    {
      case Full:
        {
          init_FHE(p, lg2p, n); // This internally calls gfp::init_field(p) and sets p
          break;
        }

      case Shamir:
      case Replicated:
      case Q2MSP:
        {
          // gfp::init_field(p) has already been called (or should have been)
          break;
        }
      default:
        throw not_implemented();
    }

  SecretSharing ret;
  CAS AS;
  switch (v)
    {
      case Full:
        ret.SD.Initialize_Full_Threshold(n);
        break;
      case Shamir:
        ret.SD.Initialize_Shamir(n, t);
        if (ret.SD.Etype != HSS)
          {
            AS.assign(n, t);
            ret.SD2.Initialize_Replicated(AS);
          }
        break;
      case Replicated:
        init_replicated(ret.SD, ret.SD2, n, mode, t, offline_type, Sets);
        break;
      case Q2MSP:
        init_Q2_MSP(ret.SD, ns, Gen);
        break;
      default:
        throw not_implemented();
    }

  PRNG G;
  G.ReSeed(0);

  for (unsigned int i= 0; i < n; i++)
    {
      vector<gfp> keys;
      for (unsigned int j= 0; j < ret.SD.nmacs; j++)
        {
          gfp aa;
          aa.randomize(G);
          keys.push_back(aa);
        }
      ret.mkeys.push_back(keys);
    }

  cout << "Finished setting up secret sharing. \nThe underlying MSP is...\n"
       << ret.SD.M << endl;
  return ret;
}

void init_conversion()
{
  bigint p;
  ifstream inpf("Data/SharingData.txt");
  if (inpf.fail())
    {
      throw file_error("Data/SharingData.txt");
    }

  inpf >> p;
  inpf.close();

  unsigned int nBits= numBits(p);
  if (nBits >= 512)
    {
      cout << "Prime must be less than 512 bits long alas" << endl;
      cout << "Please contact maintainers if you want a bigger prime" << endl;
      throw bad_value();
    }
  if (nBits <= sreg_bitl)
    {
      cout << "There is no need to create a conversion circuit to GC for small primes" << endl;
      cout << "This is done programmatically in the code" << endl;
      return;
    }
  unsigned int size1= 64 + conv_stat_sec;
  if (size1 > nBits)
    {
      size1= nBits;
    }

  vector<int> p_bits(512);

  bigint_to_bits(p_bits, p);
  Circuit CSub, C512;
  SimplifyCircuit CC;
  vector<unsigned int> assign_in;
  unsigned int nzeros0, nzeros1;
  ofstream outf;

  cout << "Producing conversion circuit LSSS to GC for prime " << p << endl;
  // Now load the 512 bit adder
  inpf.open("Circuits/Bristol/LSSS_to_GC.txt");
  if (inpf.fail())
    {
      throw file_error("Circuits/Bristol/LSSS_to_GC.txt");
    }
  inpf >> C512;
  inpf.close();

  /* Now do the specialization */
  assign_in.resize(512 * 3);
  nzeros0= 512 - nBits;
  nzeros1= 512 - size1;
  cout << "nBits= " << nBits << " nzeros0=" << nzeros0 << " nzeros1=" << nzeros1 << endl;
  for (unsigned int i= 0; i < 512 * 2; i++)
    {
      assign_in[i]= 2;
    }
  for (unsigned int i= 0; i < nzeros0; i++)
    {
      assign_in[nBits + i]= 0;
    }
  for (unsigned int i= 0; i < nzeros1; i++)
    {
      assign_in[512 + size1 + i]= 0;
    }
  for (unsigned int i= 0; i < 512; i++)
    {
      assign_in[1024 + i]= p_bits[i];
    }

  CC.assign(C512);
  CC.Set_Inputs(assign_in);
  CC.Simplify();

  /* Now try and simplify further by doing some substitutions */

  cout << "Going to try and simplify circuit a bit" << endl;
  cout << "\tPlease be a little patient, it is worth it in the long run :-) " << endl;
  int numI= 0;
  bool flag= true;
  while (flag && numI < iter_modp_Search_SubCircuit)
    {
      CSub= CC.Get_Circuit();
      cout << numI << " : ";
      cout << "nwires : " << CSub.get_nWires() << "    ";
      cout << "ngates : " << CSub.get_nGates() << "    ";
      cout << "and gt : " << CSub.num_AND_gates() << endl;
      flag= CC.Search_SubCircuit();
      CC.Simplify();
      numI++;
    }

  CSub= CC.Get_Circuit();

  outf.open("Data/ConversionCircuit-LSSS_to_GC.txt");
  if (outf.fail())
    {
      throw file_error("Data/ConversionCircuit-LSSS_to_GC.txt");
    }
  outf << CSub << endl;
  outf.close();

  cout << "Completed the conversion" << endl;
}

void enter_sets(std::istream &cin, imatrix &Sets, bool unqualified, unsigned int n)
{
  unsigned int v= 0;
  while (v < 1)
    {
      cout << "How many ";
      if (unqualified)
        {
          cout << "un";
        }
      cout << "qualified sets are you going to enter?\n";
      cin >> v;
    }
  Sets.resize(v);

  cout << "Each set is enterred as a vector of n 0/1 values\n";
  cout << "With 1 meaning that player is in the set\n";
  cout << "For example \n\t0 1 1 0\n";
  cout << "Would mean players 1 and 2 are in the set, but players\n";
  cout << "zero and 3 are not." << endl;

  for (unsigned int i= 0; i < v; i++)
    {
      Sets[i].resize(n);
      bool ok= false;
      while (ok == false)
        {
          ok= true;
          cout << "Enter set number " << i << endl;
          for (unsigned int j= 0; j < n; j++)
            {
              cin >> Sets[i][j];
              if (Sets[i][j] != 0 && Sets[i][j] != 1)
                {
                  ok= false;
                }
            }
          if (ok == false)
            {
              cout << "ERROR!\nTry Again!!!" << endl;
            }
        }
    }
}
