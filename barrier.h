#include <unordered_set>
#include <netinet/in.h>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

#ifndef __BARRIER_H
#define __BARRIER_H

class barrier
{
  public:
    barrier(const int num_barriers, const int sp, const int thread_clients, const std::vector<std::string> *remote_addresses);
    virtual ~barrier();

    int notify_and_wait(const int barrier_id, const int reset_barrier_id = 0);

    std::vector<struct sockaddr_in> get_remote_addresses();

  private:
    static void monitor(barrier * const bp);
    void local_notify(int barrier_id);
    int remote_notify(int barrier_id);
    static int send_data(struct sockaddr_in& remote, int data);
    static void get_addr(struct addrinfo& hint, const char *addr, const char *port, struct sockaddr_in& sockipaddr);
    static int id_to_index(int barrier_id);

    int start_port;
    int nclient;
    std::vector<int> barrier_ids;
    std::vector<struct sockaddr_in> peer_addresses;
    struct sockaddr_in my_local_addr;
    int monitor_socket;
    std::mutex monitor_lock;
    std::condition_variable monitor_cond;
    std::thread monitor_thread;
};

#endif
