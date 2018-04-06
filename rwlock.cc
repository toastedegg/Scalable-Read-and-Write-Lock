#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <string>
#include "rwlock.h"
#include "mcsqueue.h"
#include "node.h"
#include "SNZI.h"
#include "linuxrwlocks.h"

using namespace std;
using namespace std::chrono;
int var = 0;

rwlock* lock;
rwlock::rwlock(int thrd_num)
{
  readFlag.store(true,memory_order_relaxed);//the writer queue should be empty in the first place
  reader_tree=new SNZI(thrd_num);
  mcsqueue=new writer_queue(thrd_num,this);
}
rwlock::~rwlock()
{
  delete mcsqueue;
  delete reader_tree;
}
void rwlock::reader_lock(int thread_id)
{
  int mystamp = mcsqueue->get_wait_timer();
  int temp1;
  int temp2;
 // printf("Im thread %d, my timestamp is: %d\n",thread_id,mystamp);
  do
  {
     temp2 =mcsqueue->get_wait_timer();
     temp1 = mcsqueue->get_cur_timer();
     if(mystamp > temp1 && mystamp > temp2 && temp2 > temp1)
     break;
  }
  while((mystamp > temp1) || (mystamp+30000 < temp1));
  //add timestamp compare here
  //reader_tree->addReader(thread_id);
  reader_tree->arrive(thread_id);
  while (!readFlag.load(memory_order_seq_cst));
}
void rwlock::reader_unlock(int thread_id)
{
  //reader_tree->readerExit(thread_id);
  reader_tree->depart(thread_id);
}
void rwlock::writer_lock(int thread_id)
{
  mcsqueue->writer_lock(thread_id);
}
void rwlock::writer_unlock(int thread_id)
{
  mcsqueue->writer_unlock(thread_id);
}

void rwlock::set_read_flag(bool flag)
{
  readFlag.store(flag,memory_order_seq_cst);
}

uint32_t rwlock::get_I()
{
  return reader_tree->getI();
}






void unit_test_routine1(void* input)
{
   
   int thread_id = *(int*)input;
   for(int j = 0; j < 1000; j++)
   {
   if(!(thread_id % 50))
   {
     write_lock(&mylock);
   //  printf("writer thread");
     write_unlock(&mylock);
   }
   else
   {
     read_lock(&mylock);
  //   printf("reader thread");
     read_unlock(&mylock);
   }
   } 

}


void unit_test_routine2(void* input)
{
   
   int thread_id = *(int*)input;
   for(int j = 0; j < 1000; j++)
   {
   if(!(thread_id % 50))
   {
     lock->writer_lock(thread_id);
    // printf("writer thread");
     lock->writer_unlock(thread_id);
   }
   else
   {
     lock->reader_lock(thread_id);
   //  printf("reader thread");
     lock->reader_unlock(thread_id);
   }
   } 
}




int main(int argc, char *argv[])
{

  int trn = stoi(argv[1]);
  lock = new rwlock(trn);
  vector<thread> B;
  int temp[trn];
  atomic_init(&mylock.lock, RW_LOCK_BIAS);
//performance test for linuxrwlock
  high_resolution_clock::time_point t1 = high_resolution_clock::now();
  for(int j = 0;j < 10;j++)
  {
    for(int i = 0; i < trn; i++) 
    {	
      temp[i] = i; 
      B.push_back(thread(&unit_test_routine1,&temp[i]));
    }
    for(int i = 0; i < trn; i++)  B[i].join();
    B.clear();
  }
  high_resolution_clock::time_point t2 = high_resolution_clock::now(); 
  


  duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
  printf("it took linuxrwlock %f seconds to finish the test in average\n",time_span.count()/10);

//performance test for our lock
  B.clear();
  t1 = high_resolution_clock::now();
  for(int j = 0;j < 10;j++)
  {
    for(int i = 0; i < trn; i++) 
    {	
      temp[i] = i; 
      B.push_back(thread(&unit_test_routine2,&temp[i]));
    }
    for(int i = 0; i < trn; i++)  B[i].join();
    B.clear();
  }
  t2 = high_resolution_clock::now(); 
  


  time_span = duration_cast<duration<double>>(t2 - t1);
  printf("it took our lock %f seconds to finish the test in average\n",time_span.count()/10);



  delete lock;
  return 0;
}




