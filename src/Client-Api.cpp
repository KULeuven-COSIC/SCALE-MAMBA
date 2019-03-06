#include <iostream>

#include "App/Client.h"

using namespace std;

int main(int argc, const char *argv[]) {

  int id, int_msg, f;
  if (argc > 1){
    id = atoi(argv[1]);
  }
  else {
    cout << "Enter Player Id. Exiting"<<endl;
    exit(-1);
  }

  Client c(id, 3);

  f = c.connect_to_player("127.0.0.1", 14000);
  
  if (f == -1){
    exit(-1);
  }
  while(1){
  cout << "Enter message: " << endl;
  cin >> int_msg;
  c.send_int_to(0, int_msg);
  cout << "Got: " << c.receive_int_from(0) << endl;
  }
  return 0;
}