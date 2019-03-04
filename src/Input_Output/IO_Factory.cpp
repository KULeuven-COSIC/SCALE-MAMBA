#include "IO_Factory.h"

unique_ptr<IO_Stream> IO_Factory::make_io(unsigned int choice)
{

  if (choice == IO_SIMPLE) {
    return unique_ptr<IO_Stream>(new Input_Output_Simple);
  }
  else if (choice == IO_FILE) {
    return unique_ptr<IO_Stream>(new Input_Output_File);
  }

  return unique_ptr<IO_Stream>(new Input_Output_Simple);

}
