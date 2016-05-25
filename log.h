#include <string>

class logger
{
  public:
    static logger& inst();

    void log(std::string *message);
    void log(std::string *format, std::string *message, ...);
    void log(const char *message);
    void log(const char *format, const char *message, ...);

  private:
    logger();
    ~logger();
};
