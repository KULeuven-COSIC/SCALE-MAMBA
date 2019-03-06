#ifndef _Server
#define _Server

#include <iostream>
#include <memory>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "System/Networking.h"
#include "Exceptions/Exceptions.h"
#include "Tools/int.h"

using namespace std;

class Client {
private:
  unsigned int client_id;
  unsigned int max_players;
  vector<int> players;

public:
  Client(unsigned int id, unsigned int max_clients);

  ~Client();

  int connect_to_player(const char* ip_address, int port);
  void send_int_to(unsigned int player_id, unsigned int x);
  int receive_int_from(unsigned int player_id);
  void broadcast_int(unsigned int x);
  void send_msg(int socket, uint8_t *msg, int len);
  void receive_msg(int socket, uint8_t *msg, int len);

};

#endif
