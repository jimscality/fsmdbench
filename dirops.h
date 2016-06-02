#include <limits.h>
#include <string>
#include "ops.h"

#ifndef __DIROPS_H
#define __DIROPS_H

class dir_op : public timed_task
{
public:

protected:
  int do_task(int type, std::string *pathname);

private:
};

#endif
