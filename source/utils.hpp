#pragma once

#include <algorithm>
#include <fstream>
#include <metrohash128/metrohash128.h>
#include <sstream>
#include <stdarg.h>
#include <stdexcept>
#include <stdio.h>
#include <unistd.h>
#include <vector>

// If we use NCurses, we need to use the appropriate printing function
#ifdef NCURSES
  #include <ncurses.h>
  #define LOG printw
#else
  #define LOG printf
#endif

// If we use NCurses, define the following useful functions
#ifdef NCURSES

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

void initializeTerminal()
{
  // Initializing ncurses screen
  initscr();
  cbreak();
  noecho();
  nodelay(stdscr, TRUE);
  scrollok(stdscr, TRUE);
}

void clearTerminal()
{
  clear();
}

void finalizeTerminal()
{
  endwin();
}

void refreshTerminal()
{
  refresh();
}

#endif // NCURSES

typedef _uint128_t hash_t;
inline hash_t calculateMetroHash(uint8_t *data, size_t size)
{
  MetroHash128 hash;
  hash.Update(data, size);
  hash_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}

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

#define EXIT_WITH_ERROR(...) exitWithError(__FILE__, __LINE__, __VA_ARGS__)
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

inline std::string simplifyMove(const std::string &move)
{
  std::string simpleMove;

  bool isEmptyMove = true;
  for (size_t i = 0; i < move.size(); i++)
    if (move[i] != '.' && move[i] != '|')
    {
      simpleMove += move[i];
      isEmptyMove = false;
    }
  if (isEmptyMove) return ".";
  return simpleMove;
}

inline bool getBitFlag(const uint8_t value, const uint8_t idx)
{
  if (((idx == 7) && (value & 0b10000000)) ||
      ((idx == 6) && (value & 0b01000000)) ||
      ((idx == 5) && (value & 0b00100000)) ||
      ((idx == 4) && (value & 0b00010000)) ||
      ((idx == 3) && (value & 0b00001000)) ||
      ((idx == 2) && (value & 0b00000100)) ||
      ((idx == 1) && (value & 0b00000010)) ||
      ((idx == 0) && (value & 0b00000001))) return true;
  return false;
}

inline size_t countButtonsPressedString(const std::string &input)
{
  size_t count = 0;
  for (size_t i = 0; i < input.size(); i++)
    if (input[i] != '.') count++;
  return count;
};

template <typename T>
inline uint16_t countButtonsPressedNumber(const T &input)
{
  uint16_t count = 0;
  if (input & 0b0000000000000001) count++;
  if (input & 0b0000000000000010) count++;
  if (input & 0b0000000000000100) count++;
  if (input & 0b0000000000001000) count++;
  if (input & 0b0000000000010000) count++;
  if (input & 0b0000000000100000) count++;
  if (input & 0b0000000001000000) count++;
  if (input & 0b0000000010000000) count++;
  if (input & 0b0000000100000000) count++;
  if (input & 0b0000001000000000) count++;
  if (input & 0b0000010000000000) count++;
  if (input & 0b0000100000000000) count++;
  if (input & 0b0001000000000000) count++;
  if (input & 0b0010000000000000) count++;
  if (input & 0b0100000000000000) count++;
  if (input & 0b1000000000000000) count++;
  return count;
};

static auto moveCountComparerString = [](const std::string &a, const std::string &b)
{
  return countButtonsPressedString(a) < countButtonsPressedString(b);
};
static auto moveCountComparerNumber = [](const uint8_t a, const uint8_t b)
{
  return countButtonsPressedNumber(a) < countButtonsPressedNumber(b);
};
