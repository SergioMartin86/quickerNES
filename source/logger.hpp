#pragma once

#include <stdarg.h>
#include <stdexcept>
#include <stdio.h>

// If we use NCurses, we need to use the appropriate printing function
#ifndef LOG
  #ifdef NCURSES
    #include <ncurses.h>
    #define LOG printw
  #else
    #define LOG printf
  #endif
#endif

#ifndef EXIT_WITH_ERROR
 #define EXIT_WITH_ERROR(...) exitWithError(__FILE__, __LINE__, __VA_ARGS__)
#endif

inline void exitWithError [[noreturn]] (const char *fileName, const int lineNumber, const char *format, ...)
{
  char *outstr = 0;
  va_list ap;
  va_start(ap, format);
  int ret = vasprintf(&outstr, format, ap);
  if (ret < 0) exit(-1);

  std::string outString = outstr;
  free(outstr);

  char info[1024];

  snprintf(info, sizeof(info) - 1, " + From %s:%d\n", fileName, lineNumber);
  outString += info;

  throw std::runtime_error(outString.c_str());
}

