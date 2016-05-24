#include <string>
#include <vector>

#ifndef __NAMESET_H
#define __NAMESET_H

class name_set
{
  public:
    std::vector<std::string>& get_dir_names();
    std::vector<std::string>& get_file_names();
    void reset();
  private:
    std::vector<std::string> dir_names;
    std::vector<std::string> file_names;
};

#endif
