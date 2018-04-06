#include <atomic>
#include <iostream>


using namespace std;

#ifndef RWLOCK_H
#define RWLOCK_H

class writer_queue;
class SNZI;
class node;
class rwlock
{
  private:
  SNZI* reader_tree;
  writer_queue* mcsqueue;
  public:
  atomic_bool readFlag;
  rwlock(int thrd_num);
  ~rwlock();
  void reader_lock(int thread_id);
  void reader_unlock(int thread_id);
  void writer_lock(int thread_id);
  void writer_unlock(int thread_id);
  void set_read_flag(bool flag);
  uint32_t get_I();
};


#endif
