/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

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

using namespace std;

#include "Exceptions/Exceptions.h"
#include "Networking.h"
#include "Tools/int.h"

string number_to_string(int num)
{
  char buff[10];
  sprintf(buff, "%d", num);
  string ss(buff);
  return ss;
}

void send(int socket, uint8_t *msg, int len)
{
  if (send(socket, msg, len, 0) != len)
    {
      throw Networking_error("Send error - 1 ");
    }
}

void receive(int socket, uint8_t *msg, int len)
{
  int i= 0, j;
  while (len - i > 0)
    {
      j= recv(socket, msg + i, len - i, 0);
      if (j < 0)
        {
          throw Networking_error("Receiving error - 1");
        }
      i= i + j;
    }
}

// Create the server socket and initialize the socket address structure
// max is the max number of connections to expect
int OpenListener(int port, int max)
{
  int sd;
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));

  sd= socket(AF_INET, SOCK_STREAM, 0);
  if (sd == -1)
    {
      throw Networking_error("Unable to open socket");
    }

  bzero(&addr, sizeof(addr));       /* Zero the struct before filling the fields */
  addr.sin_family= AF_INET;         /* Set the connection to TCP/IP */
  addr.sin_addr.s_addr= INADDR_ANY; /* Set our address to any interface */
  addr.sin_port= htons(port);       /* Set the server port number */

  int one= 1;
  int fl= setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof(int));
  if (fl < 0)
    {
      throw Networking_error("OpenListener: setsockopt : SO_REUSEADDR");
    }

  fl= setsockopt(sd, SOL_SOCKET, SO_REUSEPORT, (char *) &one, sizeof(int));
  if (fl < 0)
    {
      throw Networking_error("OpenListener: setsockopt : SO_REUSEPORT");
    }

  /* disable Nagle's algorithm */
  fl= setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, (char *) &one, sizeof(int));
  if (fl < 0)
    {
      throw Networking_error("OpenListener: setsockopt : TCP_NODELAY");
    }

  /* bind serv information to mysocket
     *   - Just assume it will eventually wake up
     */
  fl= 1;
  while (fl != 0)
    {
      fl= ::bind(sd, (struct sockaddr *) &addr, sizeof(addr));
      if (fl != 0)
        {
          printf("Binding to socket on %d failed, trying again in a second ...\n",
                 port);
          sleep(1);
        }
    }

  /* start listening, allowing a queue of up to 1 pending connection */
  if (listen(sd, max) != 0)
    {
      string err= "Unable to listen for connections : Error code " +
                  number_to_string(errno);
      throw Networking_error(err);
    }
  return sd;
}

// Connect for the client
int OpenConnection(const string &hostname, int port)
{
  int sd;

  sd= socket(AF_INET, SOCK_STREAM, 0);
  if (sd == -1)
    {
      throw Networking_error("Unable to open socket");
    }

  /* disable Nagle's algorithm */
  int one= 1;
  int fl= setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, (char *) &one, sizeof(int));
  if (fl < 0)
    {
      throw Networking_error("OpenConnection: setsockopt : TCP_NODELAY");
    }

  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_family= AF_INET;
  addr.sin_port= htons(port);

  struct addrinfo hints, *ai= NULL, *rp;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family= AF_INET;
  hints.ai_flags= AI_CANONNAME;

  char my_name[512];
  memset(my_name, 0, 512 * sizeof(char));
  gethostname((char *) my_name, 512);

  int erp;
  for (int i= 0; i < 60; i++)
    {
      erp= getaddrinfo(hostname.c_str(), NULL, &hints, &ai);
      if (erp == 0)
        {
          break;
        }
      else
        {
          printf("Getaddrinfo on %s  has returned '%s' for %s trying again in a "
                 "second ...\n",
                 my_name, gai_strerror(erp), hostname.c_str());
          if (ai)
            freeaddrinfo(ai);
          sleep(1);
        }
    }
  if (erp != 0)
    {
      throw Networking_error("set_up_socket:getaddrinfo");
    }

  for (rp= ai; rp != NULL; rp= rp->ai_next)
    {
      const struct in_addr *addr4=
          &((const struct sockaddr_in *) ai->ai_addr)->sin_addr;
      if (ai->ai_family == AF_INET)
        {
          memcpy((char *) &addr.sin_addr.s_addr, addr4, sizeof(in_addr));
          continue;
        }
    }
  freeaddrinfo(ai);

  // Loop until connection made
  do
    {
      fl= 1;
      while (fl == 1 || errno == EINPROGRESS)
        {
          fl= connect(sd, (struct sockaddr *) &addr, sizeof(struct sockaddr));
        }
    }
  while (fl == -1 && errno == ECONNREFUSED);
  if (fl < 0)
    {
      close(sd);
      string err= hostname + " on port " + number_to_string(port);
      throw Networking_error("Set_up_socket:connect: to " + err);
    }

  return sd;
}

/* This gets nthreads connections between SD.n players
 * Returns the *connection* socket information in csocket[thread][player][connection]
 * The variable ssocket contains the server socket number.
 * The server portnum for each player is in the vector portnum
 */
void Get_Connections(int &ssocket, vector<vector<vector<int>>> &csocket,
                     const vector<unsigned int> &portnum, unsigned int me,
                     const SystemData &SD, int verbose)
{
  // create server socket
  unsigned int nthreads= csocket.size();
  if (verbose > 0)
    {
      printf("S: Player %d opening a server socket on port %d\n", me, portnum[me]);
    }
  ssocket= OpenListener(portnum[me], 2 * SD.n * nthreads);
  for (unsigned int i= 0; i < SD.n; i++)
    {
      if (i != me)
        {
          if (i < me)
            {
              struct sockaddr_in addr;
              bzero(&addr, sizeof(addr));
              socklen_t len= sizeof(addr);

              /* We know that we are going to accept nthreads connections 
	       * per player num with num<me so even though we dont know i 
	       * is correct, we just loop on nthreads here 
	       */
              for (unsigned int j= 0; j < nthreads; j++)
                {
                  for (unsigned int k= 0; k < csocket[0][0].size(); k++)
                    {
                      int client= accept(ssocket, (struct sockaddr *) &addr,
                                         &len); /* accept connection as usual */
                      if (client == -1)
                        {
                          string err= "Unable to accept connections : Error code " +
                                      number_to_string(errno);
                          throw Networking_error(err);
                        }
                      if (verbose > 0)
                        {
                          printf("S: Connection: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                        }

                      // Receive the player connected, the thread number and the connection
                      uint8_t buff[4];
                      int p, t, c;
                      receive(client, buff, 4);
                      p= BYTES_TO_INT(buff);
                      receive(client, buff, 4);
                      t= BYTES_TO_INT(buff);
                      receive(client, buff, 4);
                      c= BYTES_TO_INT(buff);
                      csocket[t][p][c]= client;
                      if (verbose > 0)
                        {
                          printf("S: Player %d connected to %d on connection %d for thread %d\n", me, p, c, t);
                        }
                    }
                }
            }
          else
            { // Now client side stuff
              for (unsigned int j= 0; j < nthreads; j++)
                {
                  for (unsigned int k= 0; k < csocket[0][0].size(); k++)
                    {
                      if (verbose > 0)
                        {
                          printf("C: Player %d connecting to player %d on connection %d at address %s for "
                                 "thread %d on port %d\n",
                                 me, i, k, SD.IP[i].c_str(), j, portnum[i]);
                        }
                      csocket[j][i][k]= OpenConnection(SD.IP[i], portnum[i]);
                      if (verbose > 0)
                        {
                          printf("C: Player %d connected to %d on connection %d for thread %d\n", me, i, k, j);
                        }
                      // Send my number, my thread number and my connection
                      uint8_t buff[4];
                      INT_TO_BYTES(buff, me);
                      send(csocket[j][i][k], buff, 4);
                      INT_TO_BYTES(buff, j);
                      send(csocket[j][i][k], buff, 4);
                      INT_TO_BYTES(buff, k);
                      send(csocket[j][i][k], buff, 4);
                    }
                }
            }
        }
    }
}

void Close_Connections(int ssocket, vector<vector<vector<int>>> &csocket,
                       unsigned int me)
{
  // Close connection sockets
  for (unsigned int i= 0; i < csocket.size(); i++)
    {
      for (unsigned int j= 0; j < csocket[i].size(); j++)
        {
          if (j != me)
            {
              for (unsigned int k= 0; k < csocket[0][0].size(); k++)
                {
                  close(csocket[i][j][k]);
                }
            }
        }
    }
  // Close server socket
  close(ssocket);
}
