#include <string>
#include <vector>
#include "nameset.h"

class name_generator
{
  public:
    name_generator(const std::string top, const std::string base, const std::string attr, int c, int nc, int nl, int num_dirs_per_client, int num_files_per_client);
    virtual ~name_generator();

    int next_level(name_set& nameset);

  private:
    void populate_current_level();

    std::string top_dir;
    std::string basename;
    std::string attribute;
    int nlevel;
    int client;
    int nclient;
    int num_names_per_client; 
    int num_flat_names_per_client; 

    int curr_level;
    std::vector<std::string> *names_in_prev_level;
    std::vector<std::string> *names_in_curr_level;
};
