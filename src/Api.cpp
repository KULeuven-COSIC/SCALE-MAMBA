#include <iostream>

#include "App/Server.h"

using namespace std;

int main(int argc, const char *argv[]) {
  Server s(0, 14000, 1);

  s.accept_clients();

  cout << s.receive_int() << endl;
  s.send_int(1);

  return 0;
}
