#include <vector>
#include <string>
#ifdef __GXX_EXPERIMENTAL_CXX0X__
#include <thread>
#include <mutex>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#ifndef __COMMUNICATION_H
#define __COMMUNICATION_H

typedef void (*RECV_CB)(int,void*);

enum {CHANNEL_CREATE, CHANNEL_INIT, CHANNEL_START, CHANNEL_STOP};

class channel
{
  public:
    channel();
    virtual ~channel();

    virtual void init(int local_idx, const std::vector<std::string> *remote_list);
    virtual void destroy();
    virtual int start(RECV_CB cb, void* cb_param);
    virtual int stop();
    virtual int send_data(int peer_index, int data) = 0;
    virtual int broadcast_data(int data) = 0;

  protected:
    int local_index;
    std::vector<std::string> remote_addresses;
    RECV_CB recv_callback;
    void *recv_param;

  private:
    int state;
#ifdef __GXX_EXPERIMENTAL_CXX0X__
    std::mutex state_lock;
#else
    pthread_mutex_t state_lock;
#endif
};

class sock_channel : public channel
{
  public:
    sock_channel();
    virtual ~sock_channel();

    virtual void init(int local_index, const std::vector<std::string> *remote_list);
    virtual void destroy();
    virtual int start(RECV_CB cb, void* cb_param);
    virtual int stop();
  protected:
    static void get_addr(struct addrinfo& hint, const char *addr, const char *port, struct sockaddr_in& sockipaddr);
    virtual void monitor(sock_channel* const cp) = 0;
    int get_monitor_socket();
    void setnonblocking(int sock);

    std::vector<struct sockaddr_in> peer_addresses;
  private:
#ifdef __GXX_EXPERIMENTAL_CXX0X__
    static void monitor_static(sock_channel* const cp);
#else
    static void * monitor_static(void * cp);
#endif

    int monitor_socket;
#ifdef __GXX_EXPERIMENTAL_CXX0X__
    std::thread monitor_thread;
#else
    pthread_t monitor_thread;
#endif
};

class conn_channel : public sock_channel
{
  public:
    int stop();
    int send_data(int peer_index, int data);
    int broadcast_data(int data);

  private:
    void monitor(sock_channel * const cp);
};

class persistence_channel : public sock_channel
{
  public:
    int start(RECV_CB cb, void* cb_param);
    int stop();
    int send_data(int peer_index, int data);
    int broadcast_data(int data);

  private:
    void monitor(sock_channel * const cp);

    std::vector<int> send_sockets;
    std::vector<int> recv_sockets;
};
#endif
