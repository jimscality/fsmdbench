#include <syslog.h>
#include "log.h"

logger& logger::inst()
{
  static logger instance;

  return instance;
}

logger::logger()
{
  openlog("fsmdbench", LOG_PID, LOG_USER);
}

logger::~logger()
{
  closelog();
}

void logger::log(std::string *message)
{
  syslog(LOG_NOTICE, "%s", message->c_str());
}

void logger::log(std::string *format, std::string *message, ...)
{
  syslog(LOG_NOTICE, format->c_str(), message->c_str());
}

void logger::log(const char *message)
{
  syslog(LOG_NOTICE, "%s", message);
}

void logger::log(const char *format, const char *message, ...)
{
  syslog(LOG_NOTICE, format, message);
}
