#include <string>
#include <iostream>
#include "dirops.h"
#include "fileops.h"
#include "barrier.h"
#include "nameset.h"
#include "namegen.h"
#include "processopts.h"
#include "stats.h"

void handle_one(int op, timed_task *task, std::vector<std::string>& names, stats *data_handler)
{
  task_result result;
  for (auto& dn : names)
    {
      task->exec(op, &dn, result);
      if (0 == result.status_code)
        {
          data_handler->process((double)result.duration);
        }
      std::cout << "op " << std::to_string(op) << " status " << result.status_code << " in " << result.duration << "(microsec) on " << dn << std::endl;
    }
}

int measure_op(int op, std::vector<name_set> *nameset, int initial_level, int incr, int nlevel, dir_op *dop, file_op *fop, barrier *door)
{
  stats data_d;
  stats data_f;
  data_d.reset();
  data_f.reset();
  for (int l = initial_level; l >= 0 && l < nlevel; l += incr)
    {
      handle_one(op, dop, (*nameset)[l].get_dir_names(), &data_d);
      handle_one(op, fop, (*nameset)[l].get_file_names(), &data_f);

      std::cout << ">>>>>notify and wait for all clients<<<<<" << std::endl;
      if (0 != door->notify())
        {
          return -1;
        }
      door->check();
    }
  std::cout << "op " << std::to_string(op) << " on directories result: average " << std::to_string(data_d.average()) << ", variance " << std::to_string(data_d.variance()) << ", maximum " << std::to_string(data_d.maxvalue()) << std::endl;
  std::cout << "op " << std::to_string(op) << " on files result: average " << std::to_string(data_f.average()) << ", variance " << std::to_string(data_f.variance()) << ", maximum " << std::to_string(data_f.maxvalue()) << std::endl;
  return 0;
}

void fsmetadatabenchmark(std::string top, int nlevel, int client_index, std::vector<std::string> *addresses, int dirs_per_client, int files_per_client)
{
  barrier door(client_index, 12420, addresses);

  dir_op dop;
  file_op fop;

  name_generator namegen1(top, std::string("d"), std::string("f"), client_index, addresses->size(), nlevel, dirs_per_client, files_per_client);
  std::vector<name_set> nameset(nlevel);
  for (int l = 0; l < nlevel; l++)
    {
      if (0 != namegen1.next_level(nameset[l]))
        {
          return;
        }
    }
  measure_op(CREATE, &nameset, 0, 1, nlevel, &dop, &fop, &door);
  measure_op(UPDATE, &nameset, 0, 1, nlevel, &dop, &fop, &door);
  measure_op(DELETE, &nameset, nlevel-1, -1, nlevel, &dop, &fop, &door);
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
