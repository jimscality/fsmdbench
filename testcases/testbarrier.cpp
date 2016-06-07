#include <string>
#include <vector>
#include <thread>
#include <iostream>
#include "../barrier.h"

void test(void *param)
{
  barrier *b = (barrier*)param;
  for (int i = 0; i < 100000; i++)
    {
      b->notify_and_wait(1);
//      std::cout << "gate 1 iter " << std::to_string(i) << " port " << b->get_port() << ": " << std::this_thread::get_id() << std::endl;
      b->notify_and_wait(2);
//      std::cout << "gate 2 iter " << std::to_string(i) << " port " << b->get_port() << ": " << std::this_thread::get_id() << std::endl;
    }
}

int main(int argc, char ** argv)
{
  std::vector<std::string> addresses;
  addresses.push_back("127.0.0.1:12420");
  addresses.push_back("127.0.0.1:12421");
  barrier testdoor1(2, 12420, 1, &addresses);
  barrier testdoor2(2, 12421, 1, &addresses);
  std::thread thread1(test, &testdoor1);
  std::thread thread2(test, &testdoor2);
  thread1.join();
  thread2.join();
}
