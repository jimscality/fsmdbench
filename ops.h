#include <string>
#include "taskresult.h"

#ifndef __OPS_H
#define __OPS_H

enum {
  CREATE,
  UPDATE,
  DELETE,
};

class timed_task {
public:
   void exec(int op_type, std::string *pathname, task_result& result);

protected:
   virtual int do_task(int type, std::string *pathname) = 0;
};

extern std::string& get_op_name(int op);
#endif
