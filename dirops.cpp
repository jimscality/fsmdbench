#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include "dirops.h"

int dir_op::do_task(int type, std::string *pathname)
{
  switch (type)
    {
      case CREATE:
        {
          if (-1 == mkdir(pathname->c_str(), S_IRWXU | S_IRWXG | S_IRWXO))
            {
              return errno;
            }
          return 0;
        }
      case UPDATE:
        {
          int fd;
          if (-1 == (fd = open(pathname->c_str(), O_DIRECTORY)))
            {
              return errno;
            }
          if (-1 == close(fd))
            {
              return errno;
            }
          return 0;
        }
      default:
        {
          assert(0);
        }
    }
  return -1;
}
