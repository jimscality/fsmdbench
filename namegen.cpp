#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#ifndef __GXX_EXPERIMENTAL_CXX0X__
#include "util.h"
#endif
#include "namegen.h"

name_generator::name_generator(const std::string top, int c, int nc, int nl, int num_dirs_per_client, int num_files_per_client) :
  top_dir(top), nlevel(nl), client(c), nclient(nc), num_names_per_client(num_dirs_per_client), num_flat_names_per_client(num_files_per_client)
{
  upper_limit = nclient*num_names_per_client;
  segments = (char**)malloc(upper_limit*sizeof(char*));
  char buf[PATH_MAX];
  int k = 0;
  max_len = 0;
  for (int i = 0; i < nclient; i++)
    {
      for (int j = 0; j < num_names_per_client; j++)
        {
          if (PATH_MAX <= snprintf(buf, PATH_MAX, "/d_c%d_i%d", i, j))
            {
              throw std::exception();
            }
          int len = strlen(buf);
          if (max_len < len)
            {
              max_len = len;
            }
          segments[k] = strdup(buf);
          k++;
        }
    }
  max_len++;
  if (PATH_MAX < max_len*nlevel)
    {
      throw std::exception();
    }
  dir_names = (char **)malloc(num_names_per_client*sizeof(char*));
  for (int i = 0; i < num_names_per_client; i++)
    {
      if (max_len <= snprintf(buf, max_len, "/d_c%d_i%d", client, i))
        {
          throw std::exception();
        }
      dir_names[i] = strdup(buf);
    }
  file_names = (char **)malloc(num_flat_names_per_client*sizeof(char*));
  for (int i = 0; i < num_flat_names_per_client; i++)
    {
      if (max_len <= snprintf(buf, max_len, "/f_c%d_i%d", client, i))
        {
          throw std::exception();
        }
      file_names[i] = strdup(buf);
    }
}

name_generator::~name_generator()
{
  for (int i = 0; i < upper_limit; i++)
    {
      free(segments[i]);
    }
  free(segments);
  for (int i = 0; i < num_names_per_client; i++)
    {
      free(dir_names[i]);
    }
  free(dir_names);
  for (int i = 0; i < num_flat_names_per_client; i++)
    {
      free(file_names[i]);
    }
  free(file_names);
}

void name_generator::call_back(char * const path_start, char * const path_end, ACT_FUNC cb_func_d, ACT_FUNC cb_func_f, int op_d, void *param_d, int op_f, void* param_f)
{
  for (int inst = 0; inst < num_names_per_client; inst++)
    {
      char *p1 = path_end;
      char *q = dir_names[inst];
      while (*q != '\0')
        {
          *p1++ = *q++;
        }
      *p1 = '\0';
      cb_func_d(op_d, param_d, path_start);
    }
  for (int inst = 0; inst < num_flat_names_per_client; inst++)
    {
      char *p1 = path_end;
      char *q = file_names[inst];
      while (*q != '\0')
        {
          *p1++ = *q++;
        }
      *p1 = '\0';
      cb_func_f(op_f, param_f, path_start);
    }
}

void name_generator::act_on(int level, ACT_FUNC cb_func_d, ACT_FUNC cb_func_f, int op_d, void *param_d, int op_f, void* param_f)
{
  int counters[level];
  int total_paths = 1;
  for (int i = 0; i < level; i++)
    {
      counters[i] = 0;
      total_paths *= upper_limit;
    }
  char path[PATH_MAX];
  char *seg_start[level];
  int level_s = 0;
  strcpy(path, top_dir.c_str());
  seg_start[level_s] = path + top_dir.size();
  for (int i = 0; i < total_paths; i++)
    {
      char *p = seg_start[level_s];
      for (; level_s < level; level_s++)
        {
          seg_start[level_s] = p;
          char *q = segments[counters[level_s]];
          while (*q != '\0')
            {
              *p++ = *q++;
            }
        }
      call_back(path, p, cb_func_d, cb_func_f, op_d, param_d, op_f, param_f);
      for (int j = level - 1 ; j >= 0; j--)
        {
          level_s--;
          counters[j]++;
          if (counters[j] >= upper_limit)
            {
              counters[j] = 0;
            }
          else
            {
              break;
            }
        }
    }
}
