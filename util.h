#include <string>

#ifndef __UTIL_H
#define __UTIL_H

extern std::string to_string(int value);
extern std::string to_string(long int value);
extern std::string to_string(double value);

extern int stoi(const std::string& str);

#endif
