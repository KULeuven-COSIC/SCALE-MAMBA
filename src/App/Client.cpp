#include "Client.h"

Client::Client(unsigned int id, unsigned int max_players):
  client_id{id}, max_players{max_players}
{
  cout << "Client " << client_id << endl;
  
}

Client::~Client() {
  cout << "Close" << endl;
//   close(socket_id);
}

int Client::connect_to_player(const char* ip_address, int port) {
  struct sockaddr_in addr;
  // create a socket
  int socket_id = socket(AF_INET , SOCK_STREAM , 0);

  if (socket_id == -1)
	{
		printf("Could not create socket");
	}
  bzero(&addr, sizeof(addr));       /* Zero the struct before filling the fields */
  addr.sin_family= AF_INET;         /* Set the connection to TCP/IP */
  // accept on any of machines IP address
  addr.sin_addr.s_addr= INADDR_ANY; /* Set our address to any interface */
  //inet_pton(AF_INET, ip_address, &(addr.sin_addr));
  // htons transforms number to network byte order 
  addr.sin_port= htons(port);       /* Set the server port number */

  if (connect(socket_id , (struct sockaddr *)&addr , sizeof(addr)) < 0)
	{
		perror("connect failed. Error");
    return -1;
	}
    players.push_back(socket_id);
    return 1;
}

// void Client::broadcast_int(unsigned int x)
// { int i;
//   for (i = 0; i < max_players; i++){
//   uint8_t buff[4];
//   INT_TO_BYTES(buff, x);
//   send_msg(players.at(i), buff, 4);}
// }

void Client::send_int_to(unsigned int player_id, unsigned int x)
{
  uint8_t buff[4];
  INT_TO_BYTES(buff, x);
  if (player_id >= max_players){
      cout << "No player " << player_id <<endl;
      exit(-1);
  }
  send_msg(players.at(player_id), buff, 4);
}

int Client::receive_int_from(unsigned int player_id)
{
  uint8_t buff[4];
  if (player_id >= max_players){
      cout << "No player " << player_id <<endl;
      exit(-1);
  }
  receive_msg(players.at(player_id), buff, 4);
  return BYTES_TO_INT(buff);
}

void Client::send_msg(int socket, uint8_t *msg, int len)
{
  if (send(socket, msg, len, 0) != len)
    {
      throw Networking_error("Send error - 1 ");
    }
}

void Client::receive_msg(int socket, uint8_t *msg, int len)
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
