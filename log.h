#include <string>

class logger
{
  public:
    static logger& inst();

    void log(std::string *message);

  private:
    logger();
    ~logger();
};
