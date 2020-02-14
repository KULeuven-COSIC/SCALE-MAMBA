/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "handler.h"

/* Code modified from an answer on stackoverflow which I found */

#include <cxxabi.h>
#include <execinfo.h>
#include <stdlib.h>

#include <iostream>
using namespace std;

void error_handler()
{
  void *array[50];
  int size= backtrace(array, 50);

  char **messages= backtrace_symbols(array, size);
  char syscom[1024];
  // skip first stack frame (points here)
  for (int i= 0; i < size && messages != NULL; ++i)
    {
      char *mangled_name= 0, *offset_begin= 0, *offset_end= 0;

      // find parantheses and +address offset surrounding mangled name
      for (char *p= messages[i]; *p; ++p)
        {
          if (*p == '(')
            {
              mangled_name= p;
            }
          else if (*p == '+')
            {
              offset_begin= p;
            }
          else if (*p == ')')
            {
              offset_end= p;
              break;
            }
        }

      // if the line could be processed, attempt to demangle the symbol
      if (mangled_name && offset_begin && offset_end &&
          mangled_name < offset_begin)
        {
          *mangled_name++= '\0';
          *offset_begin++= '\0';
          *offset_end++= '\0';

          int status;
          char *real_name= abi::__cxa_demangle(mangled_name, 0, 0, &status);

          // if demangling is successful, output the demangled function name
          if (status == 0)
            {
              cerr << "[bt]: (" << i << ") " << messages[i] << " : "
                   << real_name << "+" << offset_begin << offset_end << endl;
            }
          // otherwise, output the mangled function name
          else
            {
              cerr << "[bt]: (" << i << ") " << messages[i] << " : "
                   << mangled_name << "+" << offset_begin << offset_end << endl;
            }
          free(real_name);
        }
      // otherwise, print the whole line
      else
        {
          cerr << "[bt]: (" << i << ") " << messages[i] << endl;
        }
      sprintf(syscom, "addr2line %s -e ./Player.x", offset_begin); //last parameter is the name of this app
      int err= system(syscom);
      if (err < 0)
        {
          abort();
        }
    }
  cerr << endl;

  free(messages);
}
