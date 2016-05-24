#include "nameset.h"

std::vector<std::string>& name_set::get_dir_names()
{
  return dir_names;
}

std::vector<std::string>& name_set::get_file_names()
{
  return file_names;
}

void name_set::reset()
{
  dir_names.clear();
  file_names.clear();
}
