#include "namegen.h"

name_generator::name_generator(const std::string top, const std::string base, const std::string attr, int c, int nc, int nl, int num_dirs_per_client, int num_files_per_client) :
  top_dir(top), basename(base), attribute(attr), nlevel(nl), client(c), nclient(nc), num_names_per_client(num_dirs_per_client), num_flat_names_per_client(num_files_per_client)
{
  curr_level = 0;
  names_in_prev_level = new std::vector<std::string>();
  names_in_prev_level->push_back(top_dir);
}

name_generator::~name_generator()
{
  delete names_in_prev_level;
}

void name_generator::populate_current_level()
{
  names_in_curr_level = new std::vector<std::string>();
  for (auto& parent : (*names_in_prev_level))
    {
      std::string partial = parent + std::string("/") + basename + std::to_string(curr_level) + std::string("_c");
      for (int inst = 0; inst < num_names_per_client; inst++)
        {
          for (int c = 0; c < nclient; c++)
            {
              std::string path = partial + std::to_string(c) + std::string("_i") + std::to_string(inst);
              names_in_curr_level->push_back(path);
            }
        }
    }
}

int name_generator::next_level(name_set& nameset)
{
  if (curr_level >= nlevel)
    {
      return -1;
    }

  populate_current_level();

  auto& dnames =  nameset.get_dir_names();
  auto& fnames = nameset.get_file_names();

  for (int i = client; i < names_in_curr_level->size(); i+= nclient)
    {
      dnames.push_back(names_in_curr_level->at(i));
    }

  for (auto& p : (*names_in_prev_level))
    {
      std::string partial = p + std::string("/") + attribute + std::to_string(curr_level) + std::string("_c") + std::to_string(client) + std::string("_i");
      for (int i = 0; i < num_flat_names_per_client; i++)
        {
          std::string path = partial + std::to_string(i);
          fnames.push_back(path);
        }
    }

  delete names_in_prev_level;
  names_in_prev_level = names_in_curr_level;
  names_in_curr_level = NULL;
  curr_level++;

  return 0;
}
