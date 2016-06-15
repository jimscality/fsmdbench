#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#ifndef __GXX_EXPERIMENTAL_CXX0X__
#include <errno.h>
#endif
#include "fileops.h"

int
file_op::do_task(int type, const char *pathname)
{
  switch(type)
    {
      case CREATE:
      case UPDATE:
        {
          int fd;
          if (-1 == (fd = open(pathname, O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)))
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
          if (-1 == unlink(pathname))
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
