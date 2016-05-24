#include <string>
#include <iostream>
#include <fstream>
#include "dirops.h"
#include "fileops.h"
#include "barrier.h"
#include "nameset.h"
#include "namegen.h"
#include "processopts.h"
#include "stats.h"

void handle_one(int op, timed_task *task, std::vector<std::string>& names, stats *data_handler, std::ofstream& data_file)
{
  task_result result;
  for (auto& dn : names)
    {
      task->exec(op, &dn, result);
      if (0 == result.status_code)
        {
          data_handler->process((double)result.duration);
        }
      if (data_file.is_open())
        {
          data_file << "op " << std::to_string(op) << " status " << result.status_code << " in " << result.duration << "(microsec) on " << dn << std::endl;
        }
    }
}

void print_summary(int op, std::string object_type, stats& data)
{
  std::cout << "op " << std::to_string(op) << " on " << object_type << " result: average " << std::to_string(data.average()) <<
            "(microsec), variance " << std::to_string(data.variance()) << "(microsec), maximum " <<
            std::to_string(data.maxvalue()) << "(microsec)" << std::endl;
}

int measure_op(int op, std::vector<name_set> *nameset, int initial_level, int incr, int nlevel, dir_op *dop, file_op *fop, barrier *door, std::ofstream& data_file)
{
  stats data_d;
  stats data_f;
  data_d.reset();
  data_f.reset();
  for (int l = initial_level; l >= 0 && l < nlevel; l += incr)
    {
      handle_one(op, dop, (*nameset)[l].get_dir_names(), &data_d, data_file);
      handle_one(op, fop, (*nameset)[l].get_file_names(), &data_f, data_file);

      if (data_file.is_open())
        {
          data_file << ">>>>>notify and wait for all clients<<<<<" << std::endl;
        }
      if (0 != door->notify())
        {
          return -1;
        }
      door->check();
    }

  print_summary(op, std::string("directories"), data_d);
  print_summary(op, std::string("files"), data_f);
  return 0;
}

void fsmetadatabenchmark(std::string top, int nlevel, int client_index, std::vector<std::string> *addresses, int dirs_per_client, int files_per_client, std::string& data_output)
{
  barrier door(client_index, 12420, addresses);

  dir_op dop;
  file_op fop;

  std::ofstream data_file;
  data_file.open(data_output);

  name_generator namegen1(top, std::string("d"), std::string("f"), client_index, addresses->size(), nlevel, dirs_per_client, files_per_client);
  std::vector<name_set> nameset(nlevel);
  for (int l = 0; l < nlevel; l++)
    {
      if (0 != namegen1.next_level(nameset[l]))
        {
          return;
        }
    }
  measure_op(CREATE, &nameset, 0, 1, nlevel, &dop, &fop, &door, data_file);
  measure_op(UPDATE, &nameset, 0, 1, nlevel, &dop, &fop, &door, data_file);
  measure_op(DELETE, &nameset, nlevel-1, -1, nlevel, &dop, &fop, &door, data_file);

  if (data_file.is_open())
    {
      data_file.close();
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
  std::string data_output;

  if (0 != process_opts(argc, argv, addresses, client_index, target_dir, levels, num_dirs, num_files, data_output))
    {
      return 1;
    }

  fsmetadatabenchmark(target_dir, levels, client_index, &addresses, num_dirs, num_files, data_output);

  return 0;
}
