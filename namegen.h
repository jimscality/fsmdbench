#include <string>
#include <vector>
#include "nameset.h"

#ifndef __NAMEGEN_H
#define __NAMEGEN_H

typedef void (*ACT_FUNC)(int, void *, const char*);

class name_generator
{
  public:
    name_generator(const std::string top, int c, int nc, int nl, int num_dirs_per_client, int num_files_per_client);
    virtual ~name_generator();

    void act_on(int level, ACT_FUNC cb_func_d, ACT_FUNC cb_func_f, int op_d, void *param_d, int op_f, void *param_f);

  private:
    void call_back(char * const path_start, char * const path_end, ACT_FUNC cb_func_d, ACT_FUNC cb_func_f, int op_d, void *param_d, int op_f, void* param_f);

    std::string top_dir;
    int nlevel;
    int client;
    int nclient;
    int num_names_per_client; 
    int num_flat_names_per_client; 

    int upper_limit;
    char** segments;
    char** dir_names;
    char** file_names;
    int max_len;
};

#endif
