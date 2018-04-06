#include <atomic>
#include <stdio.h>

#ifndef MCSQUEUE_H
#define MCSQUEUE_H

class rwlock;

class writer_queue
{
  private:
  class qnode  
  {
    public:
    qnode();
    void lock_flip();
    void lock_change(bool flag);
    bool lock_get();
    std::atomic<qnode*> next;
    private:
    std::atomic<bool> locked;
   // std::atomic<boolean> locked_reader;
  };
  std::atomic<qnode*> tail;
  std::atomic<uint16_t> cs_time;   //the timestamp used to track how many writer are waiting/in the queue.
  std::atomic<uint16_t> curr_time;  //the timestamp used to mark the last finished/currently working writer processs.
  qnode** nodelist;
  public:
  int thr_n;
  writer_queue(int n,rwlock* myowner);
  ~writer_queue();
  void writer_lock(int thread_id);
  void writer_unlock(int thread_id);
  int get_wait_timer();
  int get_cur_timer();
  rwlock* owner;  
};



#endif
 







	
