#include <vector>
#ifdef __GXX_EXPERIMENTAL_CXX0X__
#include <mutex>
#include <condition_variable>
#endif
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
#ifdef __GXX_EXPERIMENTAL_CXX0X__
    std::mutex barrier_lock;
    std::condition_variable barrier_cond;
#else
    pthread_mutex_t barrier_lock;
    pthread_cond_t barrier_cond;
#endif
};

#endif
