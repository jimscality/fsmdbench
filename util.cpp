#include <stdio.h>
#include <stdlib.h>
#include "util.h"

std::string to_string(int value)
{
  char buf[256];
  sprintf(buf, "%d", value);
  return buf;
}

std::string to_string(long int value)
{
  char buf[256];
  sprintf(buf, "%ld", value);
  return buf;
}

std::string to_string(double value)
{
  char buf[256];
  sprintf(buf, "%f", value);
  return buf;
}

int stoi(const std::string& str)
{
  return strtol(str.c_str(), NULL, 10);
}
