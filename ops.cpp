#ifdef __GXX_EXPERIMENTAL_CXX0X__
#include <chrono>
#else
#include <time.h>
#endif
#include <assert.h>
#include "ops.h"

void timed_task::exec(int op_type, std::string *pathname, task_result& result)
{
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
  result.status_code = do_task(op_type, pathname);
  std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();

  std::chrono::microseconds delta = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
  result.duration = delta.count();
#else
  struct timespec t1;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  result.status_code = do_task(op_type, pathname);
  struct timespec t2;
  clock_gettime(CLOCK_MONOTONIC, &t2);
  result.duration = (t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_nsec - t1.tv_nsec)/1000;
#endif
}

std::string& get_op_name(int op)
{
  switch(op)
  {
    case CREATE:
      static std::string create_str("CREATE");
      return create_str;
    case UPDATE:
      static std::string update_str("UPDATE");
      return update_str;
    case DELETE:
      static std::string delete_str("DELETE");
      return delete_str;
    default:
      assert(0);
  }
}
