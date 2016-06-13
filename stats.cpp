#include <math.h>
#include "stats.h"

stats::stats()
{
#ifndef __GXX_EXPERIMENTAL_CXX0X__
  pthread_mutex_init(&lock, NULL);
#endif
}

stats::~stats()
{
#ifndef __GXX_EXPERIMENTAL_CXX0X__
  pthread_mutex_destroy(&lock);
#endif
}

void stats::reset()
{
  sum = 0.0;
  sum2 = 0.0;
  counter = 0;
  maxval = 0.0;
  error_counter = 0;
}

void stats::process(double data, bool error)
{
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  std::unique_lock<std::mutex> lck(lock);
#else
  pthread_mutex_lock(&lock);
#endif
  if (error)
    {
      error_counter++;
    }
  else
    {
      sum += data;
      sum2 += data*data;
      if (data > maxval)
        {
          maxval = data;
        }
      counter++;
    }
#ifndef __GXX_EXPERIMENTAL_CXX0X__
  pthread_mutex_unlock(&lock);
#endif
}

double stats::average()
{
  return counter == 0 ? -1.0 : sum/counter;
}

double stats::maxvalue()
{
  return maxval;
}

double stats::variance()
{
  double avg = this->average();
  return counter < 2 ? -1 : sqrt(sum2/counter - avg*avg);
}

double stats::ops()
{
  return sum > 0.0 ? counter*1000000.0/sum : 0.0;
}

long stats::errors()
{
  return error_counter;
}
