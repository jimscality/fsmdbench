#include <vector>
#include <mutex>
#include <condition_variable>
#include "communication.h"

#ifndef __BARRIER_H
#define __BARRIER_H

class barrier
{
  public:
    barrier(const int num_barriers, const int num_participants, channel *cc);
    virtual ~barrier();

    int notify_and_wait(const int barrier_id);

  private:
    static void handle_recv(int data, void * param);
    void local_notify(int barrier_id);
    int remote_notify(int barrier_id);
    static int id_to_index(int barrier_id);

    channel *comm_channel;
    int nclient;
    std::vector<int> barrier_ids;
    std::mutex barrier_lock;
    std::condition_variable barrier_cond;
};

#endif
