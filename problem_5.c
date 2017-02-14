/*
The spinlock you wrote for Problem 4 does not make any particular guarantees about fairness. 
Implement another spinlock that is fair in the sense that threads gain access to the critical 
section in the same order in which they begin waiting on it. 
The fair spinlock should implement basically the same interface 
(i.e., taking a pointer to a struct representing the lock) 
but you'll likely need to use a different struct.

Hint: A fair mutex can be implemented Bakery-style using two integers: 
one representing "the customer currently being served" 
and the other representing "the customer who just started waiting in line."

Test your spinlock in the same way that you tested your multicore-safe version of Bakery. 
Hand it in as problem_5.c.
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
 
volatile int stop;
volatile int in_cs=0;

int num_of_thread = 0;

struct spin_lock_t {
  volatile int served; 
  volatile int waiting;
};

struct spin_lock_t slock;

/*
 * atomic_xadd
 *
 * equivalent to atomic execution of this code:
 *
 * return (*ptr)++;
 * 
 */
static inline int atomic_xadd(volatile int *ptr) {
  register int val __asm__("eax") = 1;
  asm volatile("lock xaddl %0,%1"
  : "+r" (val)
  : "m" (*ptr)
  : "memory"
  );  
  return val;
}
/*Implement ticket lock*/
// Acquire a lock
void spin_lock(struct spin_lock_t *s) {
  int waiting = atomic_xadd(&s->waiting);
  while(waiting != s->served){};
}

// Release a lock
void spin_unlock(struct spin_lock_t *s) {
  atomic_xadd(&s->served);
}
 
/* create thread argument struct for thr_func() */
typedef struct _thread_data_t {
  int tid;
  int sleepsecond;
} thread_data_t;

/* thread function */
void *thr_func(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;
 
  printf("hello from thr_func, thread id: %d\n", data->tid);
  int tid = data->tid;
  long int i=1;
  while(stop)
  {
    i++;
    //lock
    spin_lock(&slock);

    assert(in_cs==0);
    in_cs++;
    assert(in_cs==1);
    in_cs++;
    assert(in_cs==2);
    in_cs++;
    assert(in_cs==3);
    in_cs=0;
    
    spin_unlock(&slock);
  }
  printf("Thread %d End, enter %ld times\n",tid,i);
  pthread_exit(NULL);
}

/*main thread function*/
void *main_thr_func(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;
 
  printf("hello from main thread, thread id: %d\n", data->tid);
  stop = 1;
  int NUM_THREADS = data->tid;
  num_of_thread = NUM_THREADS;
  int second = data->sleepsecond;
  int rc = 0;
  int i = 0;
  /* create a thread_data_t argument array */
  pthread_t thr[NUM_THREADS];
  thread_data_t thr_data[NUM_THREADS];

  //create thread with critical section
  //other thread
  for (i = 0; i < NUM_THREADS; ++i) {
    thr_data[i].tid = i;
    if ((rc = pthread_create(&thr[i], NULL, thr_func, &thr_data[i]))) {
      fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
      break;
    }
  }
  /*SLEEP*/
  sleep(second);
  stop = 0;
  /* block until all threads complete */
  for (i = 0; i < NUM_THREADS; ++i) {
    pthread_join(thr[i], NULL);
    //printf("join %d\n",i);
  }
  

  pthread_exit(NULL);
}

 
int main(int argc, char *argv[]) {

  if(argc!=3)
  {  
    fprintf(stderr, "ERROR: Please enter number of thread and time in second\n");
    return EXIT_FAILURE;
  }

  int NUM_THREADS = 0;
  int second = 0;
   /* convert string to int atoi()*/
  NUM_THREADS =  atoi(argv[1]);
  second = atoi(argv[2]);

  /* create threads */
  pthread_t main_thr;

  /* create a thread_data_t argument array */
  thread_data_t main_data;

  //initialize spin_lock_t
  slock.served = 0;
  slock.waiting = 0;

  int rc = 0;
  stop = 1;

  //main thread
  main_data.tid = NUM_THREADS;
  main_data.sleepsecond = second;
  if ((rc = pthread_create(&main_thr, NULL, main_thr_func, &main_data)))
  {
    fprintf(stderr, "error: pthread_create, rc: %d\n", NUM_THREADS);
    return EXIT_FAILURE;
  }    

  pthread_join(main_thr, NULL);

  
  return EXIT_SUCCESS;
}
