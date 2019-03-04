#ifndef _IOFactory
#define _IOFactory

#include <memory>
#include "IO.h"

using namespace std;


enum IO_CLASS {
    IO_FILE, IO_SIMPLE
};

class IO_Factory
{
  public:
    IO_Factory() {
      ;
    }
    // Factory Method
    static unique_ptr<IO_Stream> make_io(unsigned int choice);
};

#endif
