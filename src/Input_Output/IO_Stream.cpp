#include "IO_Stream.h"

void IO_Stream::init_streams(ifstream &ifs, ofstream &ofs, bool human_type)
{
  inpf = &ifs;
  outf = &ofs;
  human = human_type;
}
