#include "App.h"

unique_ptr<IO_Stream> App::make_IO() {
  unique_ptr<IO_Stream> io = IO_Factory::make_io(IO_FILE);
  unique_ptr<Input_Output_File> iof(dynamic_cast<Input_Output_File*>(io.release()));

  iof->set_file_names("src/App/files/in" + to_string(playerID) + ".txt", "src/App/files/out" + to_string(playerID) + ".txt");
  iof->init_streams(ifs, ofs, true);

  return iof;
}
