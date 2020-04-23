/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Exceptions
#define _Exceptions

#include <exception>
#include <sstream>
#include <string>
using namespace std;

#include "handler.h"

/* General Exceptions */
class not_implemented : public exception
{
  virtual const char *what() const throw()
  {
    return "Case not implemented";
  }
};
class invalid_length : public exception
{
  virtual const char *what() const throw()
  {
    return "Invalid length";
  }
};
class invalid_params : public exception
{
  virtual const char *what() const throw()
  {
    return "Invalid Params";
  }
};
// bad_value indicates something seriously wrong with code
class bad_value : public exception
{
  virtual const char *what() const throw()
  {
    return "Some value is wrong somewhere";
  }
};
class data_mismatch : public exception
{
  virtual const char *what() const throw()
  {
    return "Some value is mismatched";
  }
};
class C_problem : public exception
{
  string msg, ans;

public:
  C_problem(string m)
      : msg(m)
  {
    ans= "C_problem : " + msg;
  }
  ~C_problem() throw()
  {
  }
  virtual const char *what() const throw()
  {
    return ans.c_str();
  }
};

/* Networking Exceptions */
class sending_error : public exception
{
  virtual const char *what() const throw()
  {
    return "Sending error";
  }
};
class receiving_error : public exception
{
  virtual const char *what() const throw()
  {
    return "Receiving error";
  }
};
class IO_Error : public exception
{
  string msg, ans;

public:
  IO_Error(string m)
      : msg(m)
  {
    ans= "IO-Error : " + msg;
  }
  ~IO_Error() throw()
  {
  }
  virtual const char *what() const throw()
  {
    return ans.c_str();
  }
};
class Networking_error : public exception
{
  string msg, ans;

public:
  Networking_error(string m)
      : msg(m)
  {
    ans= "Networking-Error : " + msg;
  }
  ~Networking_error() throw()
  {
  }
  virtual const char *what() const throw()
  {
    perror(NULL);
    return ans.c_str();
  }
};
class SSL_error : public exception
{
  string msg, ans;

public:
  SSL_error(string m)
      : msg(m)
  {
    ans= "SSL-Error : " + msg;
  }
  ~SSL_error() throw()
  {
  }
  virtual const char *what() const throw()
  {
    return ans.c_str();
  }
};

/* File Exceptions */
class file_error : public exception
{
  string filename, ans;

public:
  file_error(string m= "")
      : filename(m)
  {
    ans= "File Error : " + filename;
  }
  ~file_error() throw()
  {
  }
  virtual const char *what() const throw()
  {
    return ans.c_str();
  }
};

/* Arithmetic Exceptions */
class max_mod_sz_too_small : public exception
{
  int len;

public:
  max_mod_sz_too_small(int len)
      : len(len)
  {
  }
  ~max_mod_sz_too_small() throw()
  {
  }
  virtual const char *what() const throw()
  {
    stringstream out;
    out << "MAX_MOD_SZ too small for desired bit length of p, "
        << "must be at least ceil(len(p)/len(word))+1, "
        << "in this case: " << len;
    return out.str().c_str();
  }
};
class division_by_zero : public exception
{
  virtual const char *what() const throw()
  {
    return "Division by zero";
  }
};
class arithmetic_bug : public exception
{
  virtual const char *what() const throw()
  {
    return "Arithmetic bug";
  }
};

/* Crypto Exceptions */
class invalid_commitment : public exception
{
  virtual const char *what() const throw()
  {
    return "Invalid Commitment";
  }
};
class not_Q2 : public exception
{
  virtual const char *what() const throw()
  {
    return "Not Q2 Access Structure";
  }
};

/* FHE Exceptions */
class level_mismatch : public exception
{
  virtual const char *what() const throw()
  {
    return "FHE levels do not match when they should";
  }
};
class FHE_params : public exception
{
  virtual const char *what() const throw()
  {
    return "FHE parameters cannot be computed";
  }
};
class ZKPoK_Used : public exception
{
  virtual const char *what() const throw()
  {
    return "Trying to access an already used ZKPoK output";
  }
};
class ZKPoK_Fail : public exception
{
  virtual const char *what() const throw()
  {
    return "ZKPoK does not verify";
  }
};

/* Protocol Errors */
class mac_fail : public exception
{
  virtual const char *what() const throw()
  {
    return "Mac Fail";
  }
};
class hash_fail : public exception
{
  virtual const char *what() const throw()
  {
    return "Hash Fail";
  }
};
class Sacrifice_Check_Error : public exception
{
  string filename, ans;

public:
  Sacrifice_Check_Error(string m= "")
      : filename(m)
  {
    ans= "Sacrifice : " + filename;
  }
  ~Sacrifice_Check_Error() throw()
  {
  }
  virtual const char *what() const throw()
  {
    return ans.c_str();
  }
};
class inconsistent_inputs : public exception
{
  virtual const char *what() const throw()
  {
    return "The players inputs were inconsisent in the IO input routines";
  }
};

/* Processor Exceptions */
class invalid_program : public exception
{
  virtual const char *what() const throw()
  {
    return "Invalid Program";
  }
};
class Processor_Error : public exception
{
  string msg;

public:
  Processor_Error(string m)
  {
    msg= "Processor-Error : " + m;
  }
  ~Processor_Error() throw()
  {
  }
  virtual const char *what() const throw()
  {
    return msg.c_str();
  }
};
class Invalid_Instruction : public Processor_Error
{
public:
  Invalid_Instruction(string m)
      : Processor_Error(m)
  {
  }
};
class IO_thread : public exception
{
  virtual const char *what() const throw()
  {
    return "IO requested in a thread different from zero";
  }
};
class crash_requested : public exception
{
  virtual const char *what() const throw()
  {
    return "Crash requested by program";
  }
};

class OT_error : public exception
{
  virtual const char *what() const throw()
  {
    return "Problem in OT generation";
  }
};

class OT_disabled : public exception
{
  virtual const char *what() const throw()
  {
    return "You are calling an operation which needs OT, but you disabled it";
  }
};

class stack_error : public exception
{
  virtual const char *what() const throw()
  {
    return "Stack error";
  }
};

class circuit_error : public exception
{
  virtual const char *what() const throw()
  {
    return "Circuit problem";
  }
};
class circuit_eval_error : public exception
{
  virtual const char *what() const throw()
  {
    return "Circuit evaluation problem";
  }
};

class Timer_Error : public exception
{
  string msg;

public:
  Timer_Error(string m)
  {
    msg= "Timer-Error : " + m;
  }
  ~Timer_Error() throw()
  {
  }
  virtual const char *what() const throw()
  {
    return msg.c_str();
  }
};

class cannot_do_conversion : public exception
{
  virtual const char *what() const throw()
  {
    return "The LSSS prime is too small to do conversions to GC format alas";
  }
};

class GC_Error : public exception
{
  string msg;

public:
  GC_Error(string m)
  {
    msg= "GC-Error : " + m;
  }
  ~GC_Error() throw()
  {
  }
  virtual const char *what() const throw()
  {
    return msg.c_str();
  }
};

class LF_Error : public exception
{
  string msg;

public:
  LF_Error(string m)
  {
    msg= "LF-Error : " + m;
  }
  ~LF_Error() throw()
  {
  }
  virtual const char *what() const throw()
  {
    return msg.c_str();
  }
};

#endif
