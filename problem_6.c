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
 
volatile int inCircle = 0;
volatile int Total = 0;

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
  //printf("%d\n", s->waiting);
  int waiting = atomic_xadd(&s->waiting);
  //printf("%d\n", s->waiting);
  while(waiting != s->served){};
}

// Release a lock
void spin_unlock(struct spin_lock_t *s) {
  atomic_xadd(&s->served);
}
 
/* create thread argument struct for thr_func() */
typedef struct _thread_data_t {
  int tid;
  int count;
} thread_data_t;


/* thread function */
void *thr_func(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;
  
  double x=0;
  double y=0;
  int in_circle = 0;
  int total_ran = data->count;

  while(total_ran)
  {
    //generate random coordinate in the retangle
    x=((double)rand()/RAND_MAX)*2-1; //(-1,1)
    y=((double)rand()/RAND_MAX)*2-1; //(-1,1)
    if((x*x+y*y)<=1)
    {
      in_circle++;
    }  
    total_ran--;
  }
  spin_lock(&slock);
  //update result
  inCircle += in_circle;
  Total += data->count;

  spin_unlock(&slock);

  pthread_exit(NULL);
}

/*main thread function*/
void *main_thr_func(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;
 
  //printf("Hello from Main Thread");
  int NUM_THREADS = data->tid;
  num_of_thread = NUM_THREADS;
  int count = data->count;
  int rc = 0;
  int i = 0;
  /* create a thread_data_t argument array */
  pthread_t thr[NUM_THREADS];
  thread_data_t thr_data[NUM_THREADS];

  //create thread with critical section
  //other thread
  for (i = 0; i < NUM_THREADS; ++i) {
    thr_data[i].tid = i;
    thr_data[i].count = count;
    if ((rc = pthread_create(&thr[i], NULL, thr_func, &thr_data[i]))) {
      fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
      break;
    }
  }
  
  /* block until all threads complete */
  for (i = 0; i < NUM_THREADS; ++i) {
    pthread_join(thr[i], NULL);
  }
  //output result
  printf("points in Circle: %d\n", inCircle);
  printf("points generated: %d\n", Total);
  fprintf(stderr, "pi is approximately equal to: %f\n", 4.0*inCircle/(double)Total);

  pthread_exit(NULL);
}

 
int main(int argc, char *argv[]) {

  if(argc!=3)
  {  
    fprintf(stderr, "ERROR: Please call this function with two input variables\n");
    fprintf(stderr, "One is how many threads created, the other is the number of points a thread generated\n");
    fprintf(stderr, "Ex. ./problem_6 10 50\n");
    return EXIT_FAILURE;
  }

  int NUM_THREADS = 0;
  int count = 0;
   /* convert string to int atoi()*/
  NUM_THREADS =  atoi(argv[1]);
  count = atoi(argv[2]);

  /* create threads */
  pthread_t main_thr;

  /* create a thread_data_t argument array */
  thread_data_t main_data;

  slock.served = 0;
  slock.waiting = 0;

  int rc = 0;

  //main thread
  main_data.tid = NUM_THREADS;
  main_data.count = count;
  if ((rc = pthread_create(&main_thr, NULL, main_thr_func, &main_data)))
  {
    fprintf(stderr, "error: pthread_create, rc: %d\n", NUM_THREADS);
    return EXIT_FAILURE;
  }    

  pthread_join(main_thr, NULL);

  
  return EXIT_SUCCESS;
}
