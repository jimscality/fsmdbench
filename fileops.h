#include <limits.h>
#include <string>
#include "ops.h"

class file_op : public timed_task
{
public:

protected:
  int do_task(int type, std::string *pathname);

private:
};
