#include <vector>
#include <string>

#ifndef __PROCESSOPTS_H
#define __PROCESSOPTS_H

extern int process_opts(int argc, char **argv, std::vector<std::string>& addresses, int& clients, std::string& target_dir, int& levels, int& num_dirs, int& num_files, std::string& data_output);

#endif
