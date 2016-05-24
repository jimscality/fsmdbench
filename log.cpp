#include <syslog.h>
#include "log.h"

logger& logger::inst()
{
  static logger instance;

  return instance;
}

logger::logger()
{
  openlog("fsmdbenchmark", 0, LOG_USER);
}

logger::~logger()
{
  closelog();
}

void logger::log(std::string *message)
{
  syslog(LOG_NOTICE, "%s", message->c_str());
}
