#include <chrono>
#include <assert.h>
#include "ops.h"

void timed_task::exec(int op_type, std::string *pathname, task_result& result)
{
  std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
  result.status_code = do_task(op_type, pathname);
  std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();

  std::chrono::microseconds delta = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
  result.duration = delta.count();
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
