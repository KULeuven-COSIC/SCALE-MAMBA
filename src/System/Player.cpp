/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include <errno.h>
#if !defined(__MACH__)
#include <malloc.h>
#else
#include <stdlib.h>
#endif
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <resolv.h>

#include <arpa/inet.h>

#include "openssl/err.h"
#include "openssl/ssl.h"
#include <fstream>
#include <mutex>
using namespace std;

#include "Exceptions/Exceptions.h"
#include "LSSS/Open_Protocol.h"
#include "LSSS/Open_Protocol2.h"
#include "Networking.h"
#include "Player.h"

SSL_CTX *InitCTX(void)
{
  const SSL_METHOD *method;
  SSL_CTX *ctx;

  method= TLS_method();     /* create new server-method instance */
  ctx= SSL_CTX_new(method); /* create new context from method */

  if (ctx == NULL)
    {
      ERR_print_errors_fp(stdout);
      throw SSL_error("InitCTX");
    }

  SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);

  return ctx;
}

void LoadCertificates(SSL_CTX *ctx, const char *CertFile, const char *KeyFile)
{
  /* set the local certificate from CertFile */
  if (SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0)
    {
      ERR_print_errors_fp(stdout);
      throw SSL_error("LoadCertificates 1");
    }
  /* set the private key from KeyFile (may be the same as CertFile) */
  if (SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0)
    {
      ERR_print_errors_fp(stdout);
      throw SSL_error("LoadCertificates 2");
    }
  /* verify private key */
  if (!SSL_CTX_check_private_key(ctx))
    {
      throw SSL_error("Private key does not match the public certificate");
    }
}

void ShowCerts(SSL *ssl, const string CommonName, int verbose)
{
  X509 *cert;
  char *line;

  cert= SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
  if (cert != NULL)
    {
      if (verbose > 0)
        {
          printf("Server certificates:\n");
          line= X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
          printf("Subject: %s\n", line);
          free(line);
          line= X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
          printf("Issuer: %s\n", line);
          free(line);
        }

      char buffer[256];
      X509_NAME_get_text_by_NID(X509_get_subject_name(cert), NID_commonName,
                                buffer, 256);
      string name(buffer);
      if (verbose > 0)
        {
          printf("Subject Comman Name:  %s\n", buffer);
        }
      if (name.compare(CommonName) != 0)
        {
          throw SSL_error("Common name does not match what I was expecting");
        }

      X509_free(cert);
    }
  else
    printf("No certificates.\n");
}

void Init_SSL_CTX(SSL_CTX *&ctx, unsigned int me, const SystemData &SD, const string &rootDirectory)
{
  // Initialize the SSL library
  OPENSSL_init_ssl(
      OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL);
  ctx= InitCTX();

  // Load in my certificates
  string str_crt= rootDirectory + "Cert-Store/" + SD.PlayerCRT[me];
  string str_key= str_crt.substr(0, str_crt.length() - 3) + "key";
  LoadCertificates(ctx, str_crt.c_str(), str_key.c_str());

  // Turn on client auth via cert
  SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                     NULL);

  // Load in root CA
  string str= rootDirectory + "Cert-Store/" + SD.RootCRT;
  SSL_CTX_set_client_CA_list(ctx, SSL_load_client_CA_file(str.c_str()));
  SSL_CTX_load_verify_locations(ctx, str.c_str(), NULL);
}

Player::Player(int mynumber, const SystemData &SD, int thread, SSL_CTX *ctx,
               vector<vector<int>> &csockets, const vector<gfp> &MacK, int verbose)
{
  thread_number= thread;
  unsigned int nbChannels= csockets[0].size();
  clocks.resize(10);
  G.ReSeed(thread);
  sha256.resize(nbChannels);
  for (unsigned int i= 0; i < nbChannels; i++)
    {
      SHA256_Init(&sha256[i]);
    }

  me= mynumber;
  ssl.resize(SD.n);

  OP= new Open_Protocol;
  OP2= new Open_Protocol2;

#ifdef BENCH_NETDATA
  br_messages_sent= 0;
  pp_messages_sent= 0;
  data_sent= 0;
  data_received= 0;
#endif

#ifdef BENCH_OFFLINE
  triples= 0;
  squares= 0;
  bits= 0;
  dabits= 0;
  inputs= 0;
  abits= 0;
  aands= 0;
  mod2s= 0;
#endif
  number_instructions= 0;

  // When communicating with player i, player me acts as server when i<me
  for (unsigned int i= 0; i < SD.n; i++)
    {
      ssl[i].resize(nbChannels);
      for (unsigned int j= 0; j < nbChannels; j++)
        {
          if (i != me)
            {
              ssl[i][j]= SSL_new(ctx); /* get new SSL state with context */
              if (i < me)
                { /* set connection socket to SSL state */
                  int ret= SSL_set_fd(ssl[i][j], csockets[i][j]);
                  if (ret == 0)
                    {
                      printf("S: Player %d failed to SSL_set_fd with player %d on connection %d in thread %d\n",
                             mynumber, i, j, thread);
                      throw SSL_error("SSL_set_fd");
                    }
                  if (verbose > 0)
                    {
                      printf("S: Player %d going SSL with player %d on connction %d at %s in thread %d\n",
                             mynumber, i, j, SD.IP[i].c_str(), thread);
                    }
                  /* do SSL-protocol accept */
                  ret= SSL_accept(ssl[i][j]);
                  if (ret <= 0)
                    {
                      printf("S: Error in player %d accepting to player %d on connection %d at address %s "
                             "in thread %d\n",
                             mynumber, i, j, SD.IP[i].c_str(), thread);
                      ERR_print_errors_fp(stdout);
                      throw SSL_error("SSL_accept");
                    }
                  if (verbose > 0)
                    {
                      printf("S: Player %d connected to player %d on connection %d in thread %d with %s "
                             "encryption\n",
                             mynumber, i, j, thread, SSL_get_cipher(ssl[i][j]));
                    }
                }
              else
                { // Now client side stuff
                  int ret= SSL_set_fd(ssl[i][j], csockets[i][j]);
                  if (ret == 0)
                    {
                      printf("C: Player %d failed to SSL_set_fd with player %d on connection% d in thread %d\n",
                             mynumber, i, j, thread);
                      throw SSL_error("SSL_set_fd");
                    }
                  if (verbose > 0)
                    {
                      printf("C: Player %d going SSL with player %d on connection %d at %s in thread %d\n",
                             mynumber, i, j, SD.IP[i].c_str(), thread);
                    }
                  /* do SSL-protocol connect */
                  ret= SSL_connect(ssl[i][j]);
                  if (ret <= 0)
                    {
                      printf("C: Error player %d connecting to player %d on connection %d at address %s in "
                             "thread %d\n",
                             mynumber, i, j, SD.IP[i].c_str(), thread);
                      ERR_print_errors_fp(stdout);
                      throw SSL_error("SSL_connect");
                    }
                  if (verbose > 0)
                    {
                      printf("C: Player %d connected to player %d on connection %d in thread %d with %s "
                             "encryption\n",
                             mynumber, i, j, thread, SSL_get_cipher(ssl[i][j]));
                    }
                }
              ShowCerts(ssl[i][j], SD.PlayerCN[i], verbose - 1); /* get cert and test common name */
            }
        }
    }

  mac_keys= MacK;
}

Player::~Player()
{
  for (unsigned int i= 0; i < ssl.size(); i++)
    {
      if (i != me)
        {
          SSL_free(ssl[i][0]);
          SSL_free(ssl[i][1]);
          SSL_free(ssl[i][2]);
        }
    }
  delete OP;
  delete OP2;
}

void Player::send_all(const uint8_t *data, unsigned int length,
                      int connection, bool verbose) const
{
  uint8_t szbuff[4];
#ifdef BENCH_NETDATA
  br_messages_sent++;
  int len_buff= 4;
#endif
  encode_length(szbuff, length);
  for (unsigned int i= 0; i < ssl.size(); i++)
    {
      if (i != me)
        {
          if (SSL_write(ssl[i][connection], szbuff, 4) != 4)
            {
              throw sending_error();
            }
          if (SSL_write(ssl[i][connection], data, length) != (int) length)
            {
              throw sending_error();
            }
#ifdef BENCH_NETDATA
          data_sent+= len_buff + length;
#endif
          if (verbose)
            {
              printf("Sent to player %d : ", i);
              for (unsigned int j= 0; j < 4; j++)
                {
                  printf("%.2X", buff.get_buffer()[j]);
                }
              printf(" : ");
              for (unsigned int j= 0; j < length; j++)
                {
                  printf("%.2X", data[j]);
                }
              printf("\n");
            }
        }
    }
}

void Player::send_to_player(int player, const uint8_t *data, unsigned int length,
                            int connection) const
{
  uint8_t szbuff[4];
  int len_buff= 4;
  encode_length(szbuff, length);
  if (SSL_write(ssl[player][connection], szbuff, len_buff) != len_buff)
    {
      throw sending_error();
    }
  if (SSL_write(ssl[player][connection], data, length) != (int) length)
    {
      throw sending_error();
    }
#ifdef BENCH_NETDATA
  pp_messages_sent++;
  data_sent+= len_buff + length;
#endif
}

void receive(SSL *ssl, uint8_t *data, int len)
{
  int i= 0, j;
  while (len - i > 0)
    {
      j= SSL_read(ssl, data + i, len - i);
      if (j <= 0)
        {
          /* 
             int e0=SSL_get_error(ssl,j);
             int e1=ERR_get_error();
             printf("SSL_READ error : %d : %d % d : Was trying to read % d bytes out
	     of % d bytes \n",j,e0,e1,len-i,len);
             perror("Some Error" );
          */
          throw receiving_error();
        }
      i= i + j;
    }
  if (len - i != 0)
    {
      throw receiving_error();
    }
}

const uint8_t *Player::receive_from_player(int i, unsigned int &length,
                                           int connection, bool verbose) const
{
  uint8_t szbuff[4];
  unsigned int len_buff= 4;
  receive(ssl[i][connection], szbuff, len_buff);
  length= decode_length(szbuff);

  // If not enough memory make it
  //   Assign double the amount to avoid minor tweaks later
  buff.resize(length);
  receive(ssl[i][connection], buff.get_buffer(), length);
#ifdef BENCH_NETDATA
  data_received+= len_buff + length;
#endif
  if (verbose)
    {
      printf("Received from player %d : ", i);
      for (unsigned int j= 0; j < len_buff; j++)
        {
          printf("%.2X", buff.get_buffer()[j]);
        }
      printf(" : ");
      for (unsigned int j= 0; j < length; j++)
        {
          printf("%.2X", buff.get_buffer()[j]);
        }
      printf("\n");
    }
  return buff.get_buffer();
}

/* This is deliberately weird to avoid problems with OS max buffer
 * size getting in the way
 */
void Player::Broadcast_Receive(vector<string> &o, bool check, int connection)
{
  for (unsigned int i= 0; i < ssl.size(); i++)
    {
      if (i > me)
        {
          send_to_player(i, o[me], connection);
        }
      else if (i < me)
        {
          receive_from_player(i, o[i], connection);
        }
    }
  for (unsigned int i= 0; i < ssl.size(); i++)
    {
      if (i < me)
        {
          send_to_player(i, o[me], connection);
        }
      else if (i > me)
        {
          receive_from_player(i, o[i], connection);
        }
    }

  if (check)
    {
      for (unsigned int i= 0; i < ssl.size(); i++)
        {
          SHA256_Update(&sha256[connection], o[i].c_str(), o[i].size());
        }
    }
}

void Player::Check_Broadcast(int connection)
{
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_Final(hash, &sha256[connection]);
  string ss((char *) hash, SHA256_DIGEST_LENGTH);
  send_all(ss, connection);
  for (unsigned int i= 0; i < nplayers(); i++)
    {
      if (i != whoami())
        {
          string is;
          receive_from_player(i, is, connection);
          if (is != ss)
            {
              throw hash_fail();
            }
        }
    }
  SHA256_Init(&sha256[connection]);
}

/* Again this is deliberately weird to avoid problems with OS max buffer
 * size getting in the way
 */
void Player::Send_Distinct_And_Receive(vector<string> &o, int connection) const
{
  vector<string> rec(ssl.size());
  for (unsigned int i= 0; i < ssl.size(); i++)
    {
      if (i > me)
        {
          send_to_player(i, o[i], connection);
        }
      else if (i < me)
        {
          receive_from_player(i, rec[i], connection);
        }
    }
  for (unsigned int i= 0; i < ssl.size(); i++)
    {
      if (i < me)
        {
          send_to_player(i, o[i], connection);
        }
      else if (i > me)
        {
          receive_from_player(i, rec[i], connection);
        }
    }
  for (unsigned int i= 0; i < ssl.size(); i++)
    {
      if (i != me)
        {
          o[i]= move(rec[i]);
        }
    }
}

#ifdef BENCH_NETDATA
void Player::print_network_data()
{
  printf(BENCH_TEXT_BOLD BENCH_COLOR_BLUE BENCH_MAGIC_START
         "{\"player\":%u,\n"
         "  \"thread\":%d,\n"
         "  \"netdata\":{\n"
         "    \"sent\":{\"bytes\":%ld,\"MB\":%.2f},\n"
         "    \"received\":{\"bytes\":%ld,\"MB\":%.2f}\n"
         "  }\n"
         "  \"roundsdata\":{\n"
         "    \"broadcast\":%ld\n"
         "    \"p-to-p\":%ld\n"
         "  }\n"
         "}\n" BENCH_MAGIC_END BENCH_ATTR_RESET,
         me, thread_number, data_sent, ((double) data_sent / 1000000), data_received, ((double) data_received / 1000000), br_messages_sent, pp_messages_sent);
}
#endif

#ifdef BENCH_OFFLINE
void Player::print_offline()
{
  printf(BENCH_TEXT_BOLD BENCH_COLOR_BLUE BENCH_MAGIC_START
         "{\"player\":%u,\n"
         "  \"thread\":%d,\n"
         "  \"offlinedata\":{\n"
         "    \"triples\":{\%lu\"},\n"
         "    \"squares\":{\%lu\"},\n"
         "    \"bits\":{\%lu\"},\n"
         "    \"dabits\":{\%lu\"},\n"
         "    \"inputs\":{\%lu\"},\n"
         "    \"aBits\":{\%lu\"},\n"
         "    \"aANDs\":{\%lu\"},\n"
         "    \"Mod2Tripless\":{\%lu\"},\n"
         "  }\n"
         "}\n" BENCH_MAGIC_END BENCH_ATTR_RESET,
         me, thread_number, triples, squares, bits, dabits, inputs, abits, aands, mod2s);
}
#endif
