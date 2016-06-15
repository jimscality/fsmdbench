#include <limits.h>
#include <string>
#include "ops.h"

#ifndef __FILEOPS_H
#define __FILEOPS_H

class file_op : public timed_task
{
public:

protected:
  int do_task(int type, const char *pathname);

private:
};

#endif
