#include <string>
#include <iostream>
#include "dirops.h"
#include "fileops.h"
#include "barrier.h"
#include "nameset.h"
#include "namegen.h"
#include "processopts.h"

void fsmetadatabenchmark(std::string top, int nlevel, int client_index, std::vector<std::string> *addresses, int dirs_per_client, int files_per_client)
{
  barrier hurdle(client_index, 12420, addresses);

  dir_op dop;
  file_op fop;

  int op = CREATE;
  name_generator namegen1(top, std::string("d"), std::string("f"), client_index, addresses->size(), nlevel, dirs_per_client, files_per_client);
  std::vector<name_set> nameset(nlevel);
  for (int l = 0; l < nlevel; l++)
    {
      if (0 != namegen1.next_level(nameset[l]))
        {
          return;
        }
      task_result result;
      for (auto& dn : nameset[l].get_dir_names())
        {
          dop.exec(op, &dn, result);
          std::cout << "op " << std::to_string(op) << " status " << result.status_code << " in " << result.duration << "(microsec) on " << dn << std::endl;
        }
      for (auto& fn : nameset[l].get_file_names())
        {
          fop.exec(op, &fn, result);
          std::cout << "op " << std::to_string(op) << " status " << result.status_code << " in " << result.duration << "(microsec) on" << fn << std::endl;
        }

      std::cout << ">>>>>notify and wait for all clients<<<<<" << std::endl;
      if (0 != hurdle.notify())
        {
          return;
        }
      hurdle.check();
    }
  op = DELETE;
  for (int l = nlevel - 1; l >= 0; l--)
    {
      task_result result;
      for (auto& fn : nameset[l].get_file_names())
        {
          fop.exec(op, &fn, result);
          std::cout << "op " << std::to_string(op) << " status " << result.status_code << " in " << result.duration << "(microsec) on" << fn << std::endl;
        }
      for (auto& dn : nameset[l].get_dir_names())
        {
          dop.exec(op, &dn, result);
          std::cout << "op " << std::to_string(op) << " status " << result.status_code << " in " << result.duration << "(microsec) on " << dn << std::endl;
        }

      std::cout << ">>>>>notify and wait for all clients<<<<<" << std::endl;
      if (0 != hurdle.notify())
        {
          return;
        }
      hurdle.check();
    }
}

int main(int argc, char* argv[])
{
  std::vector<std::string> addresses;
  int client_index;
  std::string target_dir;
  int levels;
  int num_dirs;
  int num_files;

  if (0 != process_opts(argc, argv, addresses, client_index, target_dir, levels, num_dirs, num_files))
    {
      return 1;
    }

  fsmetadatabenchmark(target_dir, levels, client_index, &addresses, num_dirs, num_files);
  return 0;
}
