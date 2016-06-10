#include <netdb.h>
#include <arpa/inet.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include "communication.h"

channel::channel()
{
  state = CHANNEL_CREATE;
}

void channel::init(int local_idx, const std::vector<std::string> *remote_list)
{
  std::unique_lock<std::mutex> lock(state_lock);
  if (state != CHANNEL_CREATE)
    {
      throw std::exception();
    }
  state = CHANNEL_INIT;
  this->local_index = local_idx;
  this->remote_addresses = *remote_list;
}

int channel::start(RECV_CB cb, void* cb_param)
{
  std::unique_lock<std::mutex> lock(state_lock);
  if (state != CHANNEL_INIT && state != CHANNEL_STOP)
    {
      throw std::exception();
    }
  state = CHANNEL_START;
  this->recv_callback = cb;
  this->recv_param = cb_param;
}

int channel::stop()
{
  std::unique_lock<std::mutex> lock(state_lock);
  if (state != CHANNEL_START)
    {
      throw std::exception();
    }
  state = CHANNEL_STOP;
}

sock_channel::sock_channel()
{
}

void sock_channel::get_addr(struct addrinfo& hint, const char *addr, const char *port, struct sockaddr_in& sockipaddr)
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

void sock_channel::init(int local_idx, const std::vector<std::string> *remote_list)
{
  channel::init(local_idx, remote_list);
  struct addrinfo hints;
  memset(&hints, 0x0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  std::string local_port;
  for (int i = 0; i < remote_addresses.size(); i++)
    {
      struct sockaddr_in peeraddr;
      std::string remoteone = remote_addresses.at(i);
      std::string ip_addr;
      std::string remote_port;

      std::size_t start = 0, end = 0;
      while ((end = remoteone.find(':', start)) != std::string::npos)
        { 
          if (start == 0)
            {
              ip_addr = remoteone.substr(start, end - start);
              remote_port = remoteone.substr(end + 1);
            }
          else
            {
              throw std::exception();
            }
          start = end + 1;
        }
      if (start == 0)
        {
          throw std::exception();
        }

      if (i == local_index)
        {
          local_port = remote_port;
        }
      sock_channel::get_addr(hints, ip_addr.c_str(), remote_port.c_str(), peeraddr);
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

  sock_channel::get_addr(hints, NULL, local_port.c_str(), bindaddr);

  if (-1 == bind(monitor_socket, (struct sockaddr*)&bindaddr, sizeof(struct sockaddr_in)))
    {
      close(monitor_socket);
      perror("unable to bind to monitor socket");
      throw std::exception();
    }
  
}

int sock_channel::get_monitor_socket()
{
  return monitor_socket;
}

int sock_channel::start(RECV_CB cb, void* cb_param)
{
  channel::start(cb, cb_param);
  monitor_thread = std::thread(sock_channel::monitor_static, this);
}

void sock_channel::monitor_static(sock_channel* const cp)
{
  cp->monitor(cp);
}

int sock_channel::stop()
{
  close(monitor_socket);
  monitor_thread.join();
}

int conn_channel::stop()
{
  send_data(local_index, -1);
  sock_channel::stop();
}

int conn_channel::send_data(int peer_index, int data)
{
  struct sockaddr_in *remote = &peer_addresses[peer_index];
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
  while (-1 == connect(s, (struct sockaddr*)remote, sizeof(struct sockaddr_in)))
    {
      if (ECONNREFUSED == errno || EADDRNOTAVAIL == errno)
        {
          std::string msg = std::string("retry connect to ") + std::to_string(s);
          sleep(1);
        }
      else
        {
          close(s);
          perror("unable to connect");
          char buf[INET_ADDRSTRLEN];
          std::cerr << "errno " << std::to_string(errno) << " peer " << inet_ntop(AF_INET, &remote->sin_addr, buf, INET_ADDRSTRLEN) << ":" << std::to_string(ntohs(remote->sin_port)) << std::endl;
          return -1;
        }
    }
  int id = htonl(data);
  send(s, &id, sizeof(int), 0);
  close(s);
  return 0;
}

int conn_channel::broadcast_data(int data)
{
  for (int i = 0; i < peer_addresses.size(); i++)
    {
      send_data(i, data);
    }
}

void conn_channel::monitor(sock_channel* const scp)
{
  conn_channel * const cp = dynamic_cast<conn_channel*>(scp);
  if (-1 == listen(cp->get_monitor_socket(), cp->remote_addresses.size()))
    {
      perror("unable to listen on monitor socket");
      return;
    }

  while (1)
    {
      int s;
      struct sockaddr_in peer_addr;
      socklen_t addr_len = sizeof(struct sockaddr_in);
      if (-1 == (s = accept(cp->get_monitor_socket(), (struct sockaddr*)&peer_addr, &addr_len)))
        {
          std::cout << "accept error" << std::endl;
          return;
        }
      int data;
      int n = recv(s, &data, sizeof(int), 0);
      data = ntohl(data);
      close(s);

      if (0 == memcmp(&cp->peer_addresses[cp->local_index].sin_addr, &peer_addr.sin_addr, sizeof(peer_addr.sin_addr)) && data == -1)
        {
          return;
        }

      cp->recv_callback(data, cp->recv_param);
    }
}
