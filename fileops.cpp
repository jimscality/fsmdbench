#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include "fileops.h"

int
file_op::do_task(int type, std::string *pathname)
{
  switch(type)
    {
      case CREATE:
      case UPDATE:
        {
          int fd;
          if (-1 == (fd = open(pathname->c_str(), O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)))
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
          if (-1 == unlink(pathname->c_str()))
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
