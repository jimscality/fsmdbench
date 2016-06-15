#include <string>
#include <fstream>
#ifdef __GXX_EXPERIMENTAL_CXX0X__
#include <thread>
#endif
#include "ops.h"
#include "stats.h"
#include "nameset.h"
#include "dirops.h"
#include "fileops.h"
#include "barrier.h"
#include "namegen.h"

#ifndef __EXECWORKLOAD_H
#define __EXECWORKLOAD_H

class exec_workload {
  public:
    exec_workload(std::string top, int nl, std::vector<std::string> *addrs, int port, int dirs, int files, int nclient, std::string out);
    virtual ~exec_workload();

    void exec_benchmark();

  private:
    void handle_one(int op, timed_task *task, const char* path, stats *data_handler);
    static void handle_one_dir(int op, void *param, const char *path);
    static void handle_one_file(int op, void *param, const char *path);
    void measure_op_oneclient(int op, int initial_level, int incr, int client_thread_index, name_generator &namegen);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
    static void bench_oneclient(exec_workload *ew, const int client_thread_index);
#else
    static void *bench_oneclient(void *td);
#endif
    void print_summary(int op, std::string object_type, stats& data);
    int get_client_index(const int client_thread_index);

    std::string target_dir;
    int local_index;
    int nlevel;
    std::vector<std::string> *addresses;
    int dirs_per_client;
    int files_per_client;
    int num_client_threads;
    std::string output_path;
    channel * comm_channel;
    barrier * door;

    dir_op dop;
    file_op fop;
    std::ofstream data_file;
    stats data_d;
    stats data_f;
#ifdef __GXX_EXPERIMENTAL_CXX0X__
    std::vector<std::thread> client_threads;
#else
    struct ew_tdata
    {
      exec_workload *ewp;
      int client_idx;
    };
    std::vector<pthread_t> client_threads;
#endif
};

#endif
