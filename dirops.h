#include <limits.h>
#include <string>
#include "ops.h"

class dir_op : public timed_task
{
public:

protected:
  int do_task(int type, std::string *pathname);

private:
};
