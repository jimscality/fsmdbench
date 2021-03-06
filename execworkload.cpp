#include <string>
#include <iostream>
#include <fstream>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#ifndef __GXX_EXPERIMENTAL_CXX0X__
#include "util.h"
#endif
#include "barrier.h"
#include "nameset.h"
#include "processopts.h"
#include "execworkload.h"

exec_workload::exec_workload(std::string top, int nl, std::vector<std::string> *addrs, int local_idx, int dirs, int files, int nclient, std::string out)
  : target_dir(top), nlevel(nl), addresses(addrs), local_index(local_idx), dirs_per_client(dirs), files_per_client(files), num_client_threads(nclient), output_path(out)
{
  comm_channel = new persistence_channel();
  comm_channel->init(local_idx, addrs);
  door = new barrier(2, num_client_threads*addresses->size(), comm_channel);
}

exec_workload::~exec_workload()
{
  delete door;
  comm_channel->destroy();
  delete comm_channel;
}

void exec_workload::print_summary(int op, std::string object_type, stats& data)
{
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  std::cout << get_op_name(op) << " " << object_type << " result:" << std::endl
            << "   throughput(ops/s):  " << std::to_string(data.throughput()) << std::endl
            << "   latency (micro sec) " << std::endl
            << "              average: " << std::to_string(data.average()) << std::endl
            << "             variance: " << std::to_string(data.variance()) << std::endl
            << "              maximum: " << std::to_string(data.maxvalue()) << std::endl
            << "   errors: " << std::to_string(data.errors()) << "/" << std::to_string(data.total()) << std::endl
            << std::endl;
#else
  std::cout << get_op_name(op) << " " << object_type << " result:" << std::endl
            << "   throughput(ops/s):  " << to_string(data.throughput()) << std::endl
            << "   latency (micro sec) " << std::endl
            << "              average: " << to_string(data.average()) << std::endl
            << "             variance: " << to_string(data.variance()) << std::endl
            << "              maximum: " << to_string(data.maxvalue()) << std::endl
            << "   errors: " << to_string(data.errors()) << "/" << to_string(data.total()) << std::endl
            << std::endl;
#endif
}

void exec_workload::handle_one_dir(int op, void *param, const char *path)
{
  exec_workload *ew = (exec_workload*)param;
  ew->handle_one(op, &ew->dop, path, &ew->data_d);
}

void exec_workload::handle_one_file(int op, void *param, const char *path)
{
  exec_workload *ew = (exec_workload*)param;
  ew->handle_one(op, &ew->fop, path, &ew->data_f);
}

void exec_workload::handle_one(int op, timed_task *task, const char *path, stats *data_handler)
{
  task_result result;
  task->exec(op, path, result);
  data_handler->process((double)result.duration, result.status_code != 0);
  if (data_file.is_open())
    {
      data_file << "op " << get_op_name(op) << " status " << result.status_code << " in " << result.duration << "(microsec) on " << path << std::endl;
    }
}

void exec_workload::measure_op_oneclient(int op, int initial_level, int incr, int client_thread_index, name_generator &namegen)
{
  if (0 == client_thread_index)
    {
      data_d.reset();
      data_f.reset();
    }
  for (int l = initial_level; l >= 0 && l < nlevel; l += incr)
    {
      door->notify_and_wait(1);
      namegen.act_on(l, handle_one_dir, handle_one_file, op, this, op, this);
      door->notify_and_wait(2);
    }
  if (0 == client_thread_index)
    {
      print_summary(op, std::string("directories"), data_d);
      print_summary(op, std::string("files"), data_f);
    }
}

void *exec_workload::bench_oneclient(void *td)
{
  struct ew_tdata *etd = (struct ew_tdata *) td;
  exec_workload *ew = etd->ewp;
  const int client_thread_index = etd->client_idx;
  delete etd;

  int client_index = ew->get_client_index(client_thread_index);
  int total_clients = ew->num_client_threads * ew->addresses->size();

  name_generator namegen(ew->target_dir, client_index, total_clients, ew->nlevel, ew->dirs_per_client, ew->files_per_client);

  ew->measure_op_oneclient(CREATE, 0, 1, client_thread_index, namegen);
  ew->measure_op_oneclient(UPDATE, 0, 1, client_thread_index, namegen);
  ew->measure_op_oneclient(DELETE, ew->nlevel-1, -1, client_thread_index, namegen);

  return 0;
}

void exec_workload::exec_benchmark()
{
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  data_file.open(output_path);
#else
  data_file.open(output_path.c_str());
#endif

  std::cout << std::endl;
  
  for (int i = 0; i < num_client_threads; i++)
    {
#ifdef __GXX_EXPERIMENTAL_CXX0X__
      client_threads.push_back(std::thread(bench_oneclient, this, i));
#else
      pthread_t tid;
      struct ew_tdata *td = new ew_tdata();
      td->ewp = this;
      td->client_idx = i;
      pthread_create(&tid, NULL, bench_oneclient, td);
      client_threads.push_back(tid);
#endif
    }

  for (int i = 0; i < num_client_threads; i++)
    {
#ifdef __GXX_EXPERIMENTAL_CXX0X__
      client_threads[i].join();
#else
      pthread_join(client_threads[i], NULL);
#endif
    }

  if (data_file.is_open())
    {
      data_file.close();
    }
}

int exec_workload::get_client_index(const int client_thread_index)
{
  return client_thread_index + local_index*num_client_threads;
}
