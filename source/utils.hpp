#pragma once

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdarg.h>
#include <stdexcept>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <metrohash128/metrohash128.h>
#include "nesInstance.hpp"

// If we use NCurses, define the following useful functions
#ifdef NCURSES

#include <ncurses.h>

// Function to check for keypress taken from https://github.com/ajpaulson/learning-ncurses/blob/master/kbhit.c
inline int kbhit()
{
  int ch, r;

  // turn off getch() blocking and echo
  nodelay(stdscr, TRUE);
  noecho();

  // check for input
  ch = getch();
  if (ch == ERR) // no input
    r = FALSE;
  else // input
  {
    r = TRUE;
    ungetch(ch);
  }

  // restore block and echo
  echo();
  nodelay(stdscr, FALSE);

  return (r);
}

inline int getKeyPress()
{
  while (!kbhit())
  {
    usleep(100000ul);
    refresh();
  }
  return getch();
}

inline void initializeTerminal()
{
  // Initializing ncurses screen
  initscr();
  cbreak();
  noecho();
  nodelay(stdscr, TRUE);
  scrollok(stdscr, TRUE);
}

inline void clearTerminal()
{
  clear();
}

inline void finalizeTerminal()
{
  endwin();
}

inline void refreshTerminal()
{
  refresh();
}

#endif // NCURSES

// Function to split a string into a sub-strings delimited by a character
// Taken from stack overflow answer to https://stackoverflow.com/questions/236129/how-do-i-iterate-over-the-words-of-a-string
// By Evan Teran

template <typename Out>
inline void split(const std::string &s, char delim, Out result)
{
  std::istringstream iss(s);
  std::string item;
  while (std::getline(iss, item, delim))
  {
    *result++ = item;
  }
}

inline std::vector<std::string> split(const std::string &s, char delim)
{
  std::string newString = s;
  std::replace(newString.begin(), newString.end(), '\n', ' ');
  std::vector<std::string> elems;
  split(newString, delim, std::back_inserter(elems));
  return elems;
}

// Taken from https://stackoverflow.com/questions/116038/how-do-i-read-an-entire-file-into-a-stdstring-in-c/116220#116220
inline std::string slurp(std::ifstream &in)
{
  std::ostringstream sstr;
  sstr << in.rdbuf();
  return sstr.str();
}

// Loads a string from a given file
inline bool loadStringFromFile(std::string &dst, const std::string path)
{
  std::ifstream fi(path);

  // If file not found or open, return false
  if (fi.good() == false) return false;

  // Reading entire file
  dst = slurp(fi);

  // Closing file
  fi.close();

  return true;
}

// Save string to a file
inline bool saveStringToFile(const std::string &src, const char *fileName)
{
  FILE *fid = fopen(fileName, "w");
  if (fid != NULL)
  {
    fwrite(src.c_str(), 1, src.size(), fid);
    fclose(fid);
    return true;
  }
  return false;
}

// Function to split a vector into n mostly fair chunks
template <typename T>
inline std::vector<T> splitVector(const T size, const T n)
{
  std::vector<T> subSizes(n);

  T length = size / n;
  T remain = size % n;

  for (T i = 0; i < n; i++)
    subSizes[i] = i < remain ? length + 1 : length;

  return subSizes;
}

typedef _uint128_t hash_t;

inline hash_t calculateMetroHash(uint8_t *data, size_t size)
{
  MetroHash128 hash;
  hash.Update(data, size);
  hash_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}

inline hash_t calculateStateHash(const NESInstance* nes)
{
  return calculateMetroHash(nes->getLowMem(), _LOW_MEM_SIZE);
}