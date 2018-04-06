#include <atomic>
#include <stdio.h>
#include "mcsqueue.h"
#include "rwlock.h"

using namespace std;


writer_queue* the_lock;
//int var = 0;
writer_queue::writer_queue(int n,rwlock* myowner)
{
  qnode* temp = nullptr;
  thr_n = n;
  std::atomic_init(&tail,temp);
  nodelist = new qnode*[n];
  for(int i = 0; i < n; i++)
  {
     nodelist[i] = new qnode(); 
  }
  std::atomic_init(&cs_time,(uint16_t)0);
  std::atomic_init(&curr_time,(uint16_t)0);  // atomic varibale is not neceassry 
  //curr_time = 0;
  owner = myowner;
}


writer_queue::qnode::qnode()
{
  qnode* temp = nullptr;
  std::atomic_init(&locked,false);
  std::atomic_init(&next,temp);
}



void writer_queue::writer_lock(int thread_id)
{
  int temp;
  qnode* cur_node = nodelist[thread_id];
  qnode* pred = (qnode*)atomic_exchange_explicit(&tail,cur_node,memory_order_relaxed);//writer node entering the queue	
  cs_time.fetch_add(1,memory_order_release); 
 // printf("Im thread %d, my timestamp is: %d\n",thread_id,temp);
  if(pred != NULL)
  {
    cur_node->lock_change(true); //set itself to true so it will spin on locked
    pred->next.store(cur_node,memory_order_seq_cst); //set the last node's next to the current node
    while(cur_node->lock_get()); //wait for previous node to finish 
  }
   // printf("we are old:%d\n",thread_id);
  do
  {
    owner->set_read_flag(true);
    while(owner->get_I() != 0); 
    owner->set_read_flag(false);
  }while(owner->get_I() != 0);
  curr_time.fetch_add(1,memory_order_release);     //put this update in critical section
  
}

	

void writer_queue::writer_unlock(int thread_id)
{
  qnode* cur_node = nodelist[thread_id];
  qnode* copy = cur_node;
  qnode* temp = nullptr;
  if(cur_node->next.load(memory_order_acquire) == NULL) //acquire
  {
    
    if(atomic_compare_exchange_strong_explicit(&tail,&copy,temp,memory_order_release,memory_order_relaxed)) //acquire
    {
      owner->set_read_flag(true);
      return;
    }
    while (cur_node->next.load(memory_order_seq_cst) == NULL);	 //acquire
  }
  cur_node->next.load(memory_order_acquire)->lock_change(false); //acquire
  //atomic_store(&canread,false,memory_order_seq_cst);
  cur_node->next.store(NULL,memory_order_relaxed);	 //release
  owner->set_read_flag(true);
}



//lock modification  


void writer_queue::qnode::lock_change(bool flag)
{ 
  atomic_store_explicit(&locked,flag,memory_order_release);  //release
}

bool writer_queue::qnode::lock_get()
{
  return atomic_load_explicit(&locked,memory_order_acquire);   //acquire
}

// end of lock modification



writer_queue::~writer_queue()
{
  for(int i = 0; i < thr_n;i++)
  {
    delete nodelist[i];
  }
  delete[] nodelist;
}

int writer_queue::get_wait_timer()
{
  return cs_time.load(memory_order_acquire);
}



int writer_queue::get_cur_timer()
{
  return curr_time.load(memory_order_acquire);
}



