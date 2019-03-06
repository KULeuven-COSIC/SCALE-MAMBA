#include <iostream>

#include "App/Server.h"

using namespace std;

int main(int argc, const char *argv[]) {
  int id;
  
  if (argc > 1){
    id = atoi(argv[1]);
  }
  else {
    cout << "Enter Player Id. Exiting"<<endl;
    return -1;
  }
  int max_c =3;
  Server s(id, 14000 + id, max_c);

  s.accept_clients();

  while(1){
    int a = s.receive_int_from(0);
    cout << a << endl;
    s.send_int_to(0, a*a);
  }
  return 0;
}