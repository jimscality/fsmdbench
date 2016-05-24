#include <unordered_set>
#include <netinet/in.h>
#include <vector>

#ifndef __BARRIER_H
#define __BARRIER_H

class barrier
{
  public:
    barrier(int c, int sp, std::vector<std::string> *addresses);
    virtual ~barrier();

    void check();
    int notify();

  private:
    static void* monitor(void *param);

    int client;
    int nclient;
    int start_port;

    std::vector<struct sockaddr_in> peer_addresses;
    struct sockaddr_in my_local_addr;
    int monitor_socket;
    pthread_t monitor_thread;
    pthread_mutex_t monitor_lock;
    pthread_cond_t monitor_cond;
    volatile bool level_done;
    std::unordered_set<int> *current_set;
    std::unordered_set<int> *next_set;
    std::unordered_set<int> seta;
    std::unordered_set<int> setb;
};

#endif
