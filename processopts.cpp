#include <string>
#include <vector>
#include <iostream>
#include <unistd.h>
#include "processopts.h"

static void
print_usage()
{
  std::cout << "Usage: fsmdbench -t <test directory> [-l <number of levels>] [-d <number of directories per level>] [-f <number of files per level>] [-c <threads per server>] [-p port] [-a <comma separated list of server addresses and ports>] [-o <file name for detailed output>] [-h]" << std::endl;
}

std::string get_remote_address(const std::string& str)
{
  std::size_t start = 0, end = 0;
  int count = 0;
  while ((end = str.find(':', start)) != std::string::npos)
    {
      start = end + 1;
      count++;
    }
  if (count == 0)
    {
      return str + ":" + std::to_string(12420);
    }
  else if (count == 1)
    {
      return str;
    }
  throw std::exception();
}

int
process_opts(int argc, char **argv, std::vector<std::string>& addresses, int& port, int& clients, std::string& target_dir, int& levels, int& num_dirs, int& num_files, std::string& data_output)
{
  // some default values
  levels = 2;
  clients = 1;
  num_dirs = 5;
  num_files = 3;
  port = 12420;
  int c;
  while (-1 != (c = getopt(argc, argv, "a:p:c:t:l:d:f:o:h")))
    {
      switch(c)
        {
          case 'a':
            {
              std::string text(optarg);
              std::size_t start = 0, end = 0;
              while ((end = text.find(',', start)) != std::string::npos) {
                try
                  {
                    std::string addr = get_remote_address(text.substr(start, end - start));
                    addresses.push_back(addr);
                  }
                catch (std::exception e)
                  {
                    std::cout << "Invalid option for -a" << std::endl;
                    print_usage();
                    return -1;
                  }
                start = end + 1;
              }
              addresses.push_back(text.substr(start));
            }
            break;
          case 'p':
            port = std::stoi(std::string(optarg));
            break;
          case 'c':
            clients = std::stoi(std::string(optarg));
            break;
          case 't':
            target_dir = optarg;
            break;
          case 'l':
            levels = std::stoi(std::string(optarg));
            break;
          case 'd':
            num_dirs = std::stoi(std::string(optarg));
            break;
          case 'f':
            num_dirs = std::stoi(std::string(optarg));
            break;
          case 'o':
            data_output = optarg;
            break;
          case 'h':
          case '?':
          default:
            print_usage();
            break;
        }
    }
  if (0 == target_dir.size())
    {
      std::cout << "Must specify target directory in option -t" << std::endl;
      print_usage();
      return -1;
    }
  if (levels < 1)
    {
      std::cout << "Invalid for option -l" << std::endl;
      print_usage();
      return -1;
    }
  if (0 == addresses.size())
    {
      addresses.push_back(std::string("127.0.0.1"));
    }
  if (clients < 1)
    {
      std::cout << "Invalid option for -c" << std::endl;
      print_usage();
      return -1;
    }
  
  std::cout << "Benchmark setup" << std::endl;
  std::cout << "  Threads per server: " << clients << std::endl;
  std::cout << "  Number of server(s): " << addresses.size() << std::endl;
  std::cout << "  Server(s): ";
  for (auto& s : addresses)
    {
      std::cout << s << " ";
    }
  std::cout << std::endl;
  std::cout << "  bind to local port: " << std::to_string(port) << std::endl;
  std::cout << "  Number of directory levels: " << levels << std::endl;
  std::cout << "  Number of directores per level: " << num_dirs << std::endl;
  std::cout << "  Number of files per level: " << num_files << std::endl;
  std::cout << "  Benchmark target directory: " << target_dir << std::endl;
  return 0;
}
