#include <chrono>
#include "ops.h"

void timed_task::exec(int type, std::string *pathname, task_result& result)
{
  std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
  result.status_code = do_task(type, pathname);
  std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();

  std::chrono::microseconds delta = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
  result.duration = delta.count();
}
