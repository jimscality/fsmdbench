#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <signal.h>
#include "barrier.h"
#include "log.h"

barrier::barrier(const int num_barriers, const int sp, const int thread_clients, const std::vector<std::string> *remote_addresses) : start_port(sp)
{
  nclient = thread_clients * remote_addresses->size();
  for (int i = 0; i < num_barriers; i++)
    {
      barrier_ids.push_back(0);
    }

  struct addrinfo hints;
  memset(&hints, 0x0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  barrier::get_addr(hints, "127.0.0.1", std::to_string(start_port).c_str(), my_local_addr);

  for (int i = 0; i < remote_addresses->size(); i++)
    {
      struct sockaddr_in peeraddr;
      barrier::get_addr(hints, remote_addresses->at(i).c_str(), std::to_string(start_port).c_str(), peeraddr);
      peer_addresses.push_back(peeraddr);
    }

  if (-1 == (monitor_socket = socket(AF_INET, SOCK_STREAM, 0)))
    {
      perror("unable to create monitor socket");
      throw std::exception();
    }
  int reuse = 1;
  if (-1 == setsockopt(monitor_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)))
    {
      close(monitor_socket);
      perror("unable to set reuse option");
      throw std::exception();
    }

  hints.ai_flags = AI_PASSIVE;
  struct addrinfo *bindaddrinfo;
  struct sockaddr_in bindaddr;

  barrier::get_addr(hints, NULL, std::to_string(start_port).c_str(), bindaddr);

  if (-1 == bind(monitor_socket, (struct sockaddr*)&bindaddr, sizeof(struct sockaddr_in)))
    {
      close(monitor_socket);
      perror("unable to bind to monitor socket");
      throw std::exception();
    }

  monitor_thread = std::thread(barrier::monitor, this);
}

barrier::~barrier()
{
  barrier::send_data(my_local_addr, -1);
  close(monitor_socket);
  monitor_thread.join();
}

void barrier::monitor(barrier * const bp)
{
  if (-1 == listen(bp->monitor_socket, bp->nclient))
    {
      perror("unable to listen on monitor socket");
      return;
    }

  while (1)
    {
      int s;
      struct sockaddr_in peer_addr;
      socklen_t addr_len = sizeof(struct sockaddr_in);
      if (-1 == (s = accept(bp->monitor_socket, (struct sockaddr*)&peer_addr, &addr_len)))
        {
          std::cout << "accept error" << std::endl;
          return;
        }
      int id;
      int n = recv(s, &id, sizeof(int), 0);
      int barrier_id = ntohs(id);
      close(s);

      if (barrier_id >= 1 && barrier_id <= bp->barrier_ids.size())
        {
          bp->local_notify(barrier_id);
        }
      else
        {
          if (0 == memcmp(&bp->my_local_addr.sin_addr, &peer_addr.sin_addr, sizeof(peer_addr.sin_addr)))
            {
              return;
            }
        }
    }
}

int barrier::send_data(struct sockaddr_in& remote, int data)
{
  int s;
  if (-1 == (s = socket(AF_INET, SOCK_STREAM, 0)))
    {
      perror("unable to create connect socket");
      return -1;
    }
  int reuse = 1;
  if (-1 == setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)))
    {
      close(s);
      perror("unable to set reuse option for connect");
      return -1;
    }
  while (-1 == connect(s, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)))
    {
      if (ECONNREFUSED == errno || EADDRNOTAVAIL == errno)
        {
          std::string msg = std::string("retry connect to ") + std::to_string(s);
          logger::inst().log(&msg);
          sleep(1);
        }
      else
        {
          close(s);
          perror("unable to connect");
          char buf[INET_ADDRSTRLEN];
          std::cerr << "errno " << std::to_string(errno) << " peer " << inet_ntop(AF_INET, &remote.sin_addr, buf, INET_ADDRSTRLEN) << ":" << std::to_string(ntohs(remote.sin_port)) << std::endl;
          return -1;
        }
    }
  int id = htons(data);
  send(s, &id, sizeof(int), 0);
  close(s);
  std::string msg = std::string("connected to ") + std::to_string(s);
  logger::inst().log(&msg);
  return 0;
}

int barrier::remote_notify(const int barrier_id)
{
  for (auto& peer : peer_addresses)
    {
      barrier::send_data(peer, barrier_id);
    }
  return 0;
}

void barrier::local_notify(int barrier_id)
{
  std::unique_lock<std::mutex> lck(monitor_lock);
  barrier_ids[barrier::id_to_index(barrier_id)]++;
  monitor_cond.notify_all();
}

int barrier::notify_and_wait(const int barrier_id, const int reset_barrier_id)
{
  if (barrier_id < 1 || barrier_id > barrier_ids.size())
    {
      return -1;
    }
  remote_notify(barrier_id);
  std::unique_lock<std::mutex> lck(monitor_lock);
  while (barrier_ids[barrier::id_to_index(barrier_id)] < nclient)
    {
      monitor_cond.wait(lck);
    }
  if (reset_barrier_id >= 1 && reset_barrier_id <= barrier_ids.size()
      && barrier_ids[barrier::id_to_index(reset_barrier_id)] == nclient)
    {
      barrier_ids[barrier::id_to_index(reset_barrier_id)] = 0;
    }
  return 0;
}

std::vector<struct sockaddr_in> barrier::get_remote_addresses()
{
  return peer_addresses;
}

void barrier::get_addr(struct addrinfo& hint, const char *addr, const char *port, struct sockaddr_in& sockipaddr)
{
  struct addrinfo *info;
  if (0 != getaddrinfo(addr, port, &hint, &info))
    {
      throw std::exception();
    }
  assert(sizeof(struct sockaddr_in) == info->ai_addrlen);
  memcpy(&sockipaddr, info->ai_addr, sizeof(struct sockaddr_in));
  freeaddrinfo(info);
}

int barrier::id_to_index(int barrier_id)
{
  return barrier_id - 1;
}
