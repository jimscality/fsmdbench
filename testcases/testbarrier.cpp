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
      if (i % 10000 == 0)
        {
          std::cout << "gate 1 iter " << std::to_string(i) << ": " << std::this_thread::get_id() << std::endl;
        }
      b->notify_and_wait(2);
      if (i % 10000 == 0)
        {
          std::cout << "gate 2 iter " << std::to_string(i) << ": " << std::this_thread::get_id() << std::endl;
        }
    }
}

int main(int argc, char ** argv)
{
  int n = 2;
  if (argc == 2)
    {
      n = std::stoi(argv[1]);
    }
  std::cout << "number of participants: " << std::to_string(n) << std::endl;
  std::vector<std::string> addresses;
  for (int i = 0; i < n; i++)
    {
      addresses.push_back(std::string("127.0.0.1:") + std::to_string(12420+i));
    }
//  std::vector<conn_channel> c_list(n);
  std::vector<persistence_channel> c_list(n);
  for (int i = 0; i < n; i++)
    {
      c_list[i].init(i, &addresses);
    }
  std::vector<barrier*> b_list;
  for (int i = 0; i < n; i++)
    {
      barrier *b = new barrier(2, n, &c_list[i]);
      b_list.push_back(b);
    }
  std::vector<std::thread*> t_list;
  for (int i = 0; i < n; i++)
    {
      std::thread *t = new std::thread(test, b_list[i]);
      t_list.push_back(t);
    }
  for (int i = 0; i < n; i++)
    {
      t_list[i]->join();
    }
  for (int i = 0; i < n; i++)
    {
      delete t_list[i];
      delete b_list[i];
      c_list[i].destroy();
    }
}
