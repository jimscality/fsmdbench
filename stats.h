#include <mutex>

#ifndef __STATS_H
#define __STATS_H

class stats
{
  public:
    void reset();

    void process(double data, bool error);

    double average();
    double maxvalue();
    double variance();
    double ops();
    long errors();

  private:
    std::mutex lock;
    double sum;
    double sum2;
    double maxval;
    long counter;
    long error_counter;
};

#endif
