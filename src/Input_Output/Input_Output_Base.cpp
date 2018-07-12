
#include "Input_Output_Base.h"

Input_Output_Base::Input_Output_Base()
{
  SHA256_Init(&sha256);
}

void Input_Output_Base::Update_Checker(const stringstream &ss)
{
  SHA256_Update(&sha256, ss.str().c_str(), ss.str().size());
}

void Input_Output_Base::Update_Checker(const gfp &input, unsigned int channel)
{
  stringstream ss;
  ss << "GFP:" << channel << " " << input;
  Update_Checker(ss);
}

void Input_Output_Base::Update_Checker(const long input, unsigned int channel)
{
  stringstream ss;
  ss << "REGI :" << channel << " " << input;
  Update_Checker(ss);
}

string Input_Output_Base::Get_Check()
{
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_Final(hash, &sha256);
  string ss((char *) hash, SHA256_DIGEST_LENGTH);
  SHA256_Init(&sha256);
  return ss;
}
