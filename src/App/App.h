#ifndef _App
#define _App

#include <iostream>
#include <memory>
#include "Input_Output/IO_Stream.h"
#include "Input_Output/IO_Factory.h"

using namespace std;

class App {
public:
  unsigned int playerID;
  ifstream ifs;
  ofstream ofs;

  App(unsigned int id): playerID{id} {}


  unique_ptr<IO_Stream> make_IO();

  ~App() {}

  void init();
};

#endif
