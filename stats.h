#ifdef __GXX_EXPERIMENTAL_CXX0X__
#include <mutex>
#else
#include <pthread.h>
#endif

#ifndef __STATS_H
#define __STATS_H

class stats
{
  public:
    stats();
    virtual ~stats();

    void reset();

    void process(double data, bool error);

    double average();
    double maxvalue();
    double variance();
    double throughput();
    long total();
    long errors();

  private:
#ifdef __GXX_EXPERIMENTAL_CXX0X__
    std::mutex lock;
#else
    pthread_mutex_t lock;
#endif
    double sum;
    double sum2;
    double maxval;
    long counter;
    long error_counter;
};

#endif
