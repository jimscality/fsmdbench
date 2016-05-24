#include <math.h>
#include "stats.h"

void stats::reset()
{
  sum = 0.0;
  sum2 = 0.0;
  counter = 0;
  maxval = 0.0;
}

void stats::process(double data)
{
  sum += data;
  sum2 += data*data;
  if (data > maxval)
    {
      maxval = data;
    }
  counter++;
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
