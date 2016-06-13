#include <netdb.h>
#include <arpa/inet.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <fcntl.h>
#ifndef __GXX_EXPERIMENTAL_CXX0X__
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "util.h"
#endif
#include "communication.h"

channel::channel()
{
  state = CHANNEL_CREATE;
#ifndef __GXX_EXPERIMENTAL_CXX0X__
  pthread_mutex_init(&state_lock, NULL);
#endif
}

channel::~channel()
{
#ifndef __GXX_EXPERIMENTAL_CXX0X__
  pthread_mutex_destroy(&state_lock);
#endif
}

void channel::init(int local_idx, const std::vector<std::string> *remote_list)
{
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  std::unique_lock<std::mutex> lock(state_lock);
#else
  pthread_mutex_lock(&state_lock);
#endif
  if (state != CHANNEL_CREATE)
    {
      throw std::exception();
    }
  state = CHANNEL_INIT;
  this->local_index = local_idx;
  this->remote_addresses = *remote_list;
#ifndef __GXX_EXPERIMENTAL_CXX0X__
  pthread_mutex_unlock(&state_lock);
#endif
}

void channel::destroy()
{
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  std::unique_lock<std::mutex> lock(state_lock);
#else
  pthread_mutex_lock(&state_lock);
#endif
  if (state == CHANNEL_INIT || state == CHANNEL_STOP)
    {
      state = CHANNEL_CREATE;
    }
  else
    {
      throw std::exception();
    }
#ifndef __GXX_EXPERIMENTAL_CXX0X__
  pthread_mutex_unlock(&state_lock);
#endif
}

int channel::start(RECV_CB cb, void* cb_param)
{
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  std::unique_lock<std::mutex> lock(state_lock);
#else
  pthread_mutex_lock(&state_lock);
#endif
  if (state != CHANNEL_INIT && state != CHANNEL_STOP)
    {
      throw std::exception();
    }
  state = CHANNEL_START;
  this->recv_callback = cb;
  this->recv_param = cb_param;
#ifndef __GXX_EXPERIMENTAL_CXX0X__
  pthread_mutex_unlock(&state_lock);
#endif
}

int channel::stop()
{
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  std::unique_lock<std::mutex> lock(state_lock);
#else
  pthread_mutex_lock(&state_lock);
#endif
  if (state != CHANNEL_START)
    {
      throw std::exception();
    }
  state = CHANNEL_STOP;
#ifndef __GXX_EXPERIMENTAL_CXX0X__
  pthread_mutex_unlock(&state_lock);
#endif
}

sock_channel::sock_channel()
{
}

sock_channel::~sock_channel()
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
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  monitor_thread = std::thread(sock_channel::monitor_static, this);
#else
  pthread_create(&monitor_thread, NULL, sock_channel::monitor_static, this);
#endif
}

void sock_channel::destroy()
{
  channel::destroy();
  close(monitor_socket);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  monitor_thread.join();
#else
  pthread_join(monitor_thread, NULL);
#endif
}

int sock_channel::get_monitor_socket()
{
  return monitor_socket;
}

void sock_channel::setnonblocking(int sock)
{
	int opts;

	opts = fcntl(sock,F_GETFL);
	if (opts < 0) {
		perror("fcntl(F_GETFL)");
		exit(EXIT_FAILURE);
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(sock,F_SETFL,opts) < 0) {
		perror("fcntl(F_SETFL)");
		exit(EXIT_FAILURE);
	}
	return;
}


int sock_channel::start(RECV_CB cb, void* cb_param)
{
  channel::start(cb, cb_param);
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
void sock_channel::monitor_static(sock_channel* const cp)
{
  cp->monitor(cp);
}
#else
void * sock_channel::monitor_static(void *cp)
{
  sock_channel* scp = (sock_channel*)cp;
  scp->monitor(scp);
  return 0;
}
#endif

int sock_channel::stop()
{
  channel::stop();
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
#ifdef __GXX_EXPERIMENTAL_CXX0X__
          std::string msg = std::string("retry connect to ") + std::to_string(s);
#else
          std::string msg = std::string("retry connect to ") + to_string(s);
#endif
          sleep(1);
        }
      else
        {
          close(s);
          perror("unable to connect");
          char buf[INET_ADDRSTRLEN];
#ifdef __GXX_EXPERIMENTAL_CXX0X__
          std::cerr << "errno " << std::to_string(errno) << " peer " << inet_ntop(AF_INET, &remote->sin_addr, buf, INET_ADDRSTRLEN) << ":" << std::to_string(ntohs(remote->sin_port)) << std::endl;
#else
          std::cerr << "errno " << to_string(errno) << " peer " << inet_ntop(AF_INET, &remote->sin_addr, buf, INET_ADDRSTRLEN) << ":" << to_string(ntohs(remote->sin_port)) << std::endl;
#endif
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
      if (n == 0)
        {
          return;
        }
      data = ntohl(data);
      close(s);

      if (0 == memcmp(&cp->peer_addresses[cp->local_index].sin_addr, &peer_addr.sin_addr, sizeof(peer_addr.sin_addr)) && data == -1)
        {
          return;
        }

      cp->recv_callback(data, cp->recv_param);
    }
}

int persistence_channel::start(RECV_CB cb, void* cb_param)
{
  sock_channel::start(cb, cb_param);
  for (int i = 0; i < peer_addresses.size(); i++)
    {
      struct sockaddr_in *remote = &peer_addresses[i];
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
#ifdef __GXX_EXPERIMENTAL_CXX0X__
              std::string msg = std::string("retry connect to ") + std::to_string(s);
#else
              std::string msg = std::string("retry connect to ") + to_string(s);
#endif
              sleep(1);
            }
          else
            {
              close(s);
              perror("unable to connect");
              char buf[INET_ADDRSTRLEN];
#ifdef __GXX_EXPERIMENTAL_CXX0X__
              std::cerr << "errno " << std::to_string(errno) << " peer " << inet_ntop(AF_INET, &remote->sin_addr, buf, INET_ADDRSTRLEN) << ":" << std::to_string(ntohs(remote->sin_port)) << std::endl;
#else
              std::cerr << "errno " << to_string(errno) << " peer " << inet_ntop(AF_INET, &remote->sin_addr, buf, INET_ADDRSTRLEN) << ":" << to_string(ntohs(remote->sin_port)) << std::endl;
#endif
              return -1;
            }
        }
      send_sockets.push_back(s);
    }
  return 0;
}

int persistence_channel::stop()
{
  sock_channel::stop();
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  for (auto s : send_sockets)
    {
#else
  for (int i = 0; i < send_sockets.size(); i++)
    {
      int s = send_sockets[i];
#endif
      close(s);
    }
}

int persistence_channel::send_data(int peer_index, int data)
{
  int s = send_sockets[peer_index];
  int id = htonl(data);
  return send(s, &id, sizeof(int), 0);
}

int persistence_channel::broadcast_data(int data)
{
  for (int i = 0; i < peer_addresses.size(); i++)
    {
      send_data(i, data);
    }
}

void persistence_channel::monitor(sock_channel* const scp)
{
  persistence_channel * const cp = dynamic_cast<persistence_channel*>(scp);
  if (-1 == listen(cp->get_monitor_socket(), cp->remote_addresses.size()))
    {
      perror("unable to listen on monitor socket");
      return;
    }

  fd_set fds;

  while (1)
    {
      FD_ZERO(&fds);
      FD_SET(cp->get_monitor_socket(), &fds);

      int max_sock = cp->get_monitor_socket();
#ifdef __GXX_EXPERIMENTAL_CXX0X__
      for (int s : cp->recv_sockets)
        {
#else
      for (int i = 0; i < cp->recv_sockets.size(); i++)
        {
          int s = cp->recv_sockets[i];
#endif
          if (s > max_sock)
            {
              max_sock = s;
            }
          FD_SET(s, &fds);
        }

      if (-1 == select(max_sock + 1, &fds, NULL, NULL, NULL))
        {
          return;
        }
      if (FD_ISSET(cp->get_monitor_socket(), &fds))
        {
          int s;
          struct sockaddr_in peer_addr;
          socklen_t addr_len = sizeof(struct sockaddr_in);
          if (-1 == (s = accept(cp->get_monitor_socket(), (struct sockaddr*)&peer_addr, &addr_len)))
            {
              std::cout << "accept error" << std::endl;
              return;
            }
          setnonblocking(s);
          cp->recv_sockets.push_back(s);
        }
#ifdef __GXX_EXPERIMENTAL_CXX0X__
      for (int s : cp->recv_sockets)
        {
#else
      for (int i = 0; i < cp->recv_sockets.size(); i++)
        {
          int s = cp->recv_sockets[i];
#endif
          if (FD_ISSET(s, &fds))
            {
              int data;
              int n = recv(s, &data, sizeof(int), 0);
              if (n == -1 || n == 0)
                {
                  continue;
                }

              data = ntohl(data);
              cp->recv_callback(data, cp->recv_param);
            }
        }
    }
}
