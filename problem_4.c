/*
* Write a multicore-safe spinlock for the x86-64.
* Your spinlock code should not be very long or complicated. 
* If it is, you are probably doing something wrong.
* Hint: Put a single int into struct spin_lock_t and consider the lock to be held 
  when the value is 1 and to be free when the value is 0.
* Test your spinlock in the same way that you tested your multicore-safe version of Bakery. 
* Hand in the full program as problem_4.c.
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
//#define mFence asm volatile ("mfence" : : : "memory")
 
volatile int stop;
volatile int in_cs=0;

int num_of_thread = 0;

struct spin_lock_t {
  volatile int value; // This needs to be volatile for it to work
};

struct spin_lock_t slock;

/*
 * atomic_cmpxchg
 * 
 * equivalent to atomic execution of this code:
 *
 * if (*ptr == old) {
 *   *ptr = new;
 *   return old;
 * } else {
 *   return *ptr;
 * }
 *
 */
static inline int atomic_cmpxchg(volatile int *ptr, int old, int new){
  int ret;
  asm volatile ("lock cmpxchgl %2,%1"
    : "=a" (ret), "+m" (*ptr)     
    : "r" (new), "0" (old)      
    : "memory");         
  return ret;                            
}

// Acquire a lock
void spin_lock(struct spin_lock_t *s) {
  while(atomic_cmpxchg(&s->value, 0, 1));
}

// Release a lock
void spin_unlock(struct spin_lock_t *s) {
  s->value = 0;
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
    //stop = 0;
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
