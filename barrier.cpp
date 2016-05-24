#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <iostream>
#include "barrier.h"
#include "log.h"

barrier::barrier(int c, int sp, std::vector<std::string> *addresses) : client(c), start_port(sp)
{
  nclient = addresses->size();
  monitor_lock = PTHREAD_MUTEX_INITIALIZER;
  monitor_cond = PTHREAD_COND_INITIALIZER;
  level_done = false;
  current_set = &seta;
  next_set = &setb;

  struct addrinfo hints;
  memset(&hints, 0x0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  struct addrinfo *myaddrinfo;
  if (0 != getaddrinfo(addresses->at(c).c_str(), std::to_string(start_port + nclient + client).c_str(), &hints, &myaddrinfo))
    {
      throw std::exception();
    }
  assert(sizeof(struct sockaddr_in) == myaddrinfo->ai_addrlen);
  memcpy(&my_local_addr, myaddrinfo->ai_addr, sizeof(struct sockaddr_in));
  freeaddrinfo(myaddrinfo);

  for (int i = 0; i < nclient; i++)
    {
      struct addrinfo *peerinfo;
      if (0 != getaddrinfo(addresses->at(i).c_str(), std::to_string(start_port + i).c_str(), &hints, &peerinfo))
        {
          throw std::exception();
        }
      assert(sizeof(struct sockaddr_in) == myaddrinfo->ai_addrlen);
      struct sockaddr_in peeraddr;
      memcpy(&peeraddr, peerinfo->ai_addr, sizeof(struct sockaddr_in));
      peer_addresses.push_back(peeraddr);
      freeaddrinfo(peerinfo);
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
  if (0 != getaddrinfo(NULL, std::to_string(start_port + client).c_str(), &hints, &bindaddrinfo))
    {
      throw std::exception();
    }
  assert(sizeof(struct sockaddr_in) == bindaddrinfo->ai_addrlen);
  memcpy(&bindaddr, bindaddrinfo->ai_addr, sizeof(struct sockaddr_in));
  freeaddrinfo(bindaddrinfo);
  if (-1 == bind(monitor_socket, (struct sockaddr*)&bindaddr, sizeof(struct sockaddr_in)))
    {
      close(monitor_socket);
      perror("unable to bind to monitor socket");
      throw std::exception();
    }

  if (0 != pthread_create(&monitor_thread, NULL, barrier::monitor, this))
    {
      throw std::exception();
    }
}

barrier::~barrier()
{
  close(monitor_socket);
  pthread_mutex_destroy(&monitor_lock);
  pthread_cond_destroy(&monitor_cond);
}

void* barrier::monitor(void *param)
{
  barrier *bp = (barrier*)param;

  if (-1 == listen(bp->monitor_socket, bp->nclient))
    {
      close(bp->monitor_socket);
      perror("unable to listen on monitor socket");
      return (void*)-1;
    }

  while (1)
    {
      int s;
      struct sockaddr_in peer_addr;
      socklen_t addr_len = sizeof(struct sockaddr_in);
      if (-1 == (s = accept(bp->monitor_socket, (struct sockaddr*)&peer_addr, &addr_len)))
        {
          close(bp->monitor_socket);
          perror("unable to accept on monitor socket");
          return (void*)-1;
        }
      close(s);
      pthread_mutex_lock(&bp->monitor_lock);
      if (0 == bp->current_set->count(peer_addr.sin_port))
        {
          bp->current_set->insert(peer_addr.sin_port);
        }
      else if (0 == bp->next_set->count(peer_addr.sin_port))
        {
          bp->next_set->insert(peer_addr.sin_port);
        }
      else
        {
          assert(0);
        }
      std::string msg = std::string("received done ") + std::to_string(bp->current_set->size()) + std::string("/") + std::to_string(bp->nclient);
      logger::inst().log(&msg);
      if (bp->current_set->size() == bp->nclient)
        {
          bp->level_done = true;
          if (0 != pthread_cond_broadcast(&bp->monitor_cond))
            {
              assert(0);
            }
        }
      pthread_mutex_unlock(&bp->monitor_lock);
    }
}

void barrier::check()
{
  pthread_mutex_lock(&monitor_lock);
  while (!level_done)
    {
      pthread_cond_wait(&monitor_cond, &monitor_lock);
    }
  auto tmp_set = current_set;
  current_set = next_set;
  next_set = tmp_set;
  next_set->clear();
  level_done = false;
  pthread_mutex_unlock(&monitor_lock);
}

int barrier::notify()
{
  for (auto& peer : peer_addresses)
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
      if (-1 == bind(s, (struct sockaddr*)&my_local_addr, sizeof(struct sockaddr_in)))
        {
          close(s);
          perror("unable to bind to connect socket");
          throw std::exception();
        }
      while (-1 == connect(s, (struct sockaddr*)&peer, sizeof(struct sockaddr_in)))
        {
          if (ECONNREFUSED == errno)
            {
              std::string msg = std::string("retry connect to ") + std::to_string(s);
              logger::inst().log(&msg);
              sleep(1);
            }
          else
            {
              close(s);
              perror("unable to connect");
              return -1;
            }
        }
      close(s);
      std::string msg = std::string("connected to ") + std::to_string(s);
      logger::inst().log(&msg);
    }
  return 0;
}
