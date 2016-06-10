#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <iostream>
#include <signal.h>
#include "barrier.h"

barrier::barrier(const int num_barriers, const int num_participants, channel *cc)
{
  comm_channel = cc;
  nclient = num_participants;
  for (int i = 0; i < num_barriers; i++)
    {
      barrier_ids.push_back(0);
    }
  comm_channel->start(barrier::handle_recv, this);
}

barrier::~barrier()
{
  comm_channel->stop();
}

void barrier::handle_recv(int data, void *param)
{
  barrier *bp = (barrier*)param;

  bp->local_notify(data);
}

int barrier::remote_notify(const int barrier_id)
{
  return comm_channel->broadcast_data(barrier_id);
}

void barrier::local_notify(int barrier_id)
{
  int index = barrier::id_to_index(barrier_id);
  std::unique_lock<std::mutex> lck(barrier_lock);
  if (barrier_ids[index] == nclient)
    {
      barrier_ids[index] = 0;
    }
  barrier_ids[index]++;
  lck.unlock();
  barrier_cond.notify_all();
}

int barrier::notify_and_wait(const int barrier_id)
{
  if (barrier_id < 1 || barrier_id > barrier_ids.size())
    {
      return -1;
    }
  int index = barrier::id_to_index(barrier_id);
  std::unique_lock<std::mutex> lck(barrier_lock);
  if (barrier_ids[index] == nclient)
    {
      barrier_ids[index] = 0;
    }
  lck.unlock();
  // remote_notify needs to be after the above boundary check.
  // otherwise the check may include the effect of this notify.
  remote_notify(barrier_id);
  lck.lock();
  while (barrier_ids[index] < nclient)
    {
      barrier_cond.wait(lck);
    }
  return 0;
}

int barrier::id_to_index(int barrier_id)
{
  return barrier_id - 1;
}
