#ifndef __GXX_EXPERIMENTAL_CXX0X__
#include "util.h"
#endif
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
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  for (auto& parent : (*names_in_prev_level))
    {
#else
  for (int i = 0; i < names_in_prev_level->size(); i++)
    {
      std::string& parent = names_in_prev_level->at(i);
#endif
#ifdef __GXX_EXPERIMENTAL_CXX0X__
      std::string partial = parent + std::string("/") + basename + std::to_string(curr_level) + std::string("_c");
#else
      std::string partial = parent + std::string("/") + basename + to_string(curr_level) + std::string("_c");
#endif
      for (int inst = 0; inst < num_names_per_client; inst++)
        {
          for (int c = 0; c < nclient; c++)
            {
#ifdef __GXX_EXPERIMENTAL_CXX0X__
              std::string path = partial + std::to_string(c) + std::string("_i") + std::to_string(inst);
#else
              std::string path = partial + to_string(c) + std::string("_i") + to_string(inst);
#endif
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

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  auto& dnames =  nameset.get_dir_names();
  auto& fnames = nameset.get_file_names();
#else
  std::vector<std::string>& dnames =  nameset.get_dir_names();
  std::vector<std::string>& fnames = nameset.get_file_names();
#endif

  for (int i = client; i < names_in_curr_level->size(); i+= nclient)
    {
      dnames.push_back(names_in_curr_level->at(i));
    }

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  for (auto& p : (*names_in_prev_level))
    {
#else
  for (int i = 0; i < names_in_prev_level->size(); i++)
    {
      std::string& p = names_in_prev_level->at(i);
#endif
#ifdef __GXX_EXPERIMENTAL_CXX0X__
      std::string partial = p + std::string("/") + attribute + std::to_string(curr_level) + std::string("_c") + std::to_string(client) + std::string("_i");
#else
      std::string partial = p + std::string("/") + attribute + to_string(curr_level) + std::string("_c") + to_string(client) + std::string("_i");
#endif
      for (int i = 0; i < num_flat_names_per_client; i++)
        {
#ifdef __GXX_EXPERIMENTAL_CXX0X__
          std::string path = partial + std::to_string(i);
#else
          std::string path = partial + to_string(i);
#endif
          fnames.push_back(path);
        }
    }

  delete names_in_prev_level;
  names_in_prev_level = names_in_curr_level;
  names_in_curr_level = NULL;
  curr_level++;

  return 0;
}
