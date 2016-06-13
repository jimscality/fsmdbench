#include <string>
#include <vector>
#ifdef __GXX_EXPERIMENTAL_CXX0X__
#include <thread>
#else
#include <pthread.h>
#include "../util.h"
#endif
#include <iostream>
#include "../barrier.h"

#ifdef __GXX_EXPERIMENTAL_CXX0X__
void test(barrier *b)
{
#else
void *test(void *param)
{
  barrier *b = (barrier*)param;
#endif
  for (int i = 0; i < 100000; i++)
    {
      b->notify_and_wait(1);
      if (i % 10000 == 0)
        {
#ifdef __GXX_EXPERIMENTAL_CXX0X__
          std::cout << "gate 1 iter " << std::to_string(i) << ": " << std::this_thread::get_id() << std::endl;
#else
          std::cout << "gate 1 iter " << to_string(i) << ": " << pthread_self() << std::endl;
#endif
        }
      b->notify_and_wait(2);
      if (i % 10000 == 0)
        {
#ifdef __GXX_EXPERIMENTAL_CXX0X__
          std::cout << "gate 2 iter " << std::to_string(i) << ": " << std::this_thread::get_id() << std::endl;
#else
          std::cout << "gate 2 iter " << to_string(i) << ": " << pthread_self() << std::endl;
#endif
        }
    }
}

int main(int argc, char ** argv)
{
  int n = 2;
  if (argc == 2)
    {
#ifdef __GXX_EXPERIMENTAL_CXX0X__
      n = std::stoi(argv[1]);
#else
      n = stoi(argv[1]);
#endif
    }
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  std::cout << "number of participants: " << std::to_string(n) << std::endl;
#else
  std::cout << "number of participants: " << to_string(n) << std::endl;
#endif
  std::vector<std::string> addresses;
  for (int i = 0; i < n; i++)
    {
#ifdef __GXX_EXPERIMENTAL_CXX0X__
      addresses.push_back(std::string("127.0.0.1:") + std::to_string(12420+i));
#else
      addresses.push_back(std::string("127.0.0.1:") + to_string(12420+i));
#endif
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
#ifdef __GXX_EXPERIMENTAL_CXX0X__
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
#else
  std::vector<pthread_t> t_list;
  for (int i = 0; i < n; i++)
    {
      pthread_t tid;
      pthread_create(&tid, NULL, test, b_list[i]);
      t_list.push_back(tid);
    }
  for (int i = 0; i < n; i++)
    {
      pthread_join(t_list[i], NULL);
    }
#endif
  for (int i = 0; i < n; i++)
    {
#ifdef __GXX_EXPERIMENTAL_CXX0X__
      delete t_list[i];
#endif
      delete b_list[i];
      c_list[i].destroy();
    }
}
