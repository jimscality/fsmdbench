#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include "dirops.h"

int dir_op::do_task(int type, const char *pathname)
{
  switch (type)
    {
      case CREATE:
        {
          if (-1 == mkdir(pathname, S_IRWXU | S_IRWXG | S_IRWXO))
            {
              return errno;
            }
          return 0;
        }
      case UPDATE:
        {
          int fd;
          if (-1 == (fd = open(pathname, O_DIRECTORY)))
            {
              return errno;
            }
          if (-1 == close(fd))
            {
              return errno;
            }
          return 0;
        }
      case DELETE:
        {
          if (-1 == rmdir(pathname))
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
