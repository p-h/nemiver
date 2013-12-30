#include <pthread.h>
#include <cstdlib>
#include <cassert>
#include <string>
#include <iostream>

static const int NUM_ITER = 10000;
static const int NUM_THREADS = 5;
// pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
// pthread_cond_t c = PTHREAD_COND_INITIALIZER;


struct thread_info
{
    pthread_t tid;
    int tnum;

    thread_info ()
    : tid (0), tnum (0)
    {
    }
};
thread_info threads[NUM_THREADS];
using namespace std;

void*
thread_func (void *arg)
{
    assert (arg != 0);
    thread_info *ti = static_cast<thread_info*> (arg);
    __attribute__((unused)) pthread_t tid = ti->tid;
    return NULL;
}

int
main ()
{
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads[i].tnum = i;
        if (pthread_create (&threads[i].tid,
                            NULL,
                            &thread_func,
                            &threads[i])) {
            cerr << "Failed to create thread id number: "<< i << endl;
            exit (EXIT_FAILURE);
        }
    }
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join (threads[i].tid, NULL);
    }
    return 0;
}

