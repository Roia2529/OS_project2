/*
Lamport's Bakery algorithm implementation:
Write a pthreads program that creates a number of threads 
that repeatedly access a critical section that is synchronized 
using Lamport's Bakery algorithm (Links to an external site.), which we discussed in class.

Your program should take two command line options. 
First, the number of threads. Second, the number of seconds to run for.

Just before terminating, your program should say how many times each thread entered the critical section. 
Make sure starvation does not occur.

To check for violations of mutual exclusion. 
Run your Bakery solution for a while (at least 10 minutes) to make sure mutual exclusion is never violated. 
If it is, you have a bug to fix. in_cs should be a global variable of type volatile int.

To avoid problems with the memory model, always restrict your threads to a single CPU like this:

taskset -c 1 ./problem_1 5 5
*/
/*
NOTE: Compile with "gcc -O2 -std=c99 -Werror -Wall -Wextra -pthread -c problem_1.c"
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#define MEMBAR __sync_synchronize()
 
volatile int stop;
volatile int in_cs=0;

volatile bool *Entering;
volatile int *Number;

int num_of_thread = 0;

int max(void)
{
  int max_value = 0;
  for(int i=0;i<num_of_thread;i++)
  {
    if(Number[i]>max_value)
      max_value = Number[i];
  } 
  return max_value;
}

void lock(int i) {
        Entering[i] = true;
        //MEMBAR;
        Number[i] = 1 + max();
        //MEMBAR;
        Entering[i] = false;
        //MEMBAR;
        for (int j = 0; j < num_of_thread; j++) {
           // Wait until thread j receives its number:
           while(Entering[j])
            { /* nothing */ }
            //MEMBAR;
           // Wait until all threads with smaller numbers or with the same
           // number, but with higher priority, finish their work:
           while(Number[j]!=0 && (Number[j] < Number[i] || (Number[j] == Number[i] && j<i)))
            { /* nothing */ }
       }
}
   
void unlock(int i) {
      //MEMBAR;
       Number[i] = 0;
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
    lock(tid);

    assert(in_cs==0);
    in_cs++;
    assert(in_cs==1);
    in_cs++;
    assert(in_cs==2);
    in_cs++;
    assert(in_cs==3);
    in_cs=0;
    //stop = 0;
    unlock(tid);
  }
  printf("Thread %d End, enter %ld times\n",data->tid,i);
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

  Entering = malloc(NUM_THREADS * sizeof(*Entering));
  Number = malloc(NUM_THREADS * sizeof(*Number));
  /*initialize Lamport's Bakery alg*/
  for (i = 0; i < NUM_THREADS; ++i) {
    Entering[i]=false;
    Number[i] = 0;
  }
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
  //printf("wait\n");

  
  return EXIT_SUCCESS;
}
