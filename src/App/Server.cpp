#include "Server.h"

Server::Server(unsigned int id, unsigned int port, unsigned int max_clients):
  player_id{id}, port_number{port}, max_clients{max_clients}
{
  cout << "Server (Player) " << player_id << ": Start listening at port " << port << endl;
  socket_id = OpenListener(port_number, max_clients);
}

Server::~Server() {
  cout << "Close" << endl;
  close(socket_id);
}

void Server::accept_clients() {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  socklen_t len = sizeof(addr);
  cout << "Waiting for client connection" << endl;
  int client = accept(socket_id, (struct sockaddr *) &addr, &len);
  printf("Accepted Connection: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
  clients.push_back(client);

  
  if (client == -1)
  {
    string err= "Unable to accept connections : Error code " + errno;
    throw Networking_error(err);
  }
  
}

void Server::send_int_to(unsigned int client_id, unsigned int x)
{
  uint8_t buff[4];
  INT_TO_BYTES(buff, x);
  send_msg(clients.at(client_id), buff, 4);
}

// void Server::broadcast_int(unsigned int x)
// { int i;
//   for (i = 0; i < max_clients; i++){
//   uint8_t buff[4];
//   INT_TO_BYTES(buff, x);
//   send_msg(clients.at(i), buff, 4);}
// }

int Server::receive_int_from(unsigned int client_id)
{ if (client_id < max_clients){
  uint8_t buff[4];
  receive_msg(clients.at(client_id), buff, 4);
  return BYTES_TO_INT(buff);}
  else{
    cout << "No connection" <<endl;
    exit(-1);
  }
}

void Server::send_msg(int socket, uint8_t *msg, int len)
{
  if (send(socket, msg, len, 0) != len)
    {
      throw Networking_error("Send error - 1 ");
    }
}

void Server::receive_msg(int socket, uint8_t *msg, int len)
{
  int i = 0, j;
  while (len - i > 0)
    {
      j = recv(socket, msg + i, len - i, 0);
      if (j < 0)
        {
          throw Networking_error("Receiving error - 1");
        }
      i = i + j;
    }
}
