#include <string>
#include "processopts.h"
#include "execworkload.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> addresses;
  int local_index;
  int clients;
  std::string target_dir;
  int levels;
  int num_dirs;
  int num_files;
  std::string data_output;

  if (0 != process_opts(argc, argv, addresses, local_index, clients, target_dir, levels, num_dirs, num_files, data_output))
    {
      return 1;
    }

  exec_workload ew(target_dir, levels, &addresses, local_index, num_dirs, num_files, clients, data_output);

  ew.exec_benchmark();

  return 0;
}
