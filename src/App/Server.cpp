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
  struct sockaddr_in client_addr;
  memset(&client_addr, 0, sizeof(client_addr));
  socklen_t len = sizeof(client_addr);

  cout << "Server: Waiting for client connection" << endl;
  int client = accept(socket_id, (struct sockaddr *) &client_addr, &len);

  if (client == -1)
  {
    string err= "Unable to accept connections : Error code " + errno;
    throw Networking_error(err);
  }

  clients.push_back(client);

  char client_ntop[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(client_addr.sin_addr), client_ntop, INET_ADDRSTRLEN);

  printf("Server: Connection accepted: %s:%d\n", client_ntop, ntohs(client_addr.sin_port));
}

void Server::send_int(unsigned int x)
{
  uint8_t buff[4];
  INT_TO_BYTES(buff, x);
  send_msg(clients.at(0), buff, 4);
}

int Server::receive_int()
{
  uint8_t buff[4];
  receive_msg(clients.at(0), buff, 4);
  return BYTES_TO_INT(buff);
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
