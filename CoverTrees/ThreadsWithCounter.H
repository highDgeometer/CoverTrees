#ifndef THREADSWITHCOUNTER_H
#define THREADSWITHCOUNTER_H

#include <semaphore.h>
#include "Types.H"
#include <pthread.h>
//#ifdef NAMED_SEMAPHORES
//#include <dispatch/dispatch.h>
//#endif

class EnlargeArg;

class ThreadsWithCounter {
private:
    int counter;
    int NTHREADS;
    pthread_t* thread_ids;
#ifdef NAMED_SEMAPHORES
    sem_t** sem_start;
    sem_t** sem_end;
    char**   sem_start_names;
    char**   sem_end_names;
#else
    sem_t* sem_start;
    sem_t* sem_end;
#endif
    pthread_mutex_t lock;
public:
    ThreadsWithCounter(int);
    ~ThreadsWithCounter();
    int getNThreads() const     { return NTHREADS;                  }
#ifdef NAMED_SEMAPHORES
    sem_t* getSemStart(int i)   { return sem_start[i];              }
    sem_t* getSemEnd(int i)     { return sem_end[i];                }
#else
    sem_t* getSemStart(int i)   { return &sem_start[i]; }
    sem_t* getSemEnd(int i) { return &sem_end[i]; }
#endif
    int Lock()                  { return pthread_mutex_lock(&lock); }
    int unLock()                { return pthread_mutex_unlock(&lock); }
#ifdef NAMED_SEMAPHORES
    int semStartPost(int tid)   { return sem_post(sem_start[tid]);  }
    int semEndPost(int tid)     { return sem_post(sem_end[tid]);    }
    int semStartWait(int tid)   { return sem_wait(sem_start[tid]);  }
    int semEndWait(int tid)     { return sem_wait(sem_end[tid]);    }
#else
    int semStartPost(int tid)   { return sem_post(&sem_start[tid]); }
    int semEndPost(int tid)     { return sem_post(&sem_end[tid]);   }
    int semStartWait(int tid)   { return sem_wait(&sem_start[tid]); }
    int semEndWait(int tid)     { return sem_wait(&sem_end[tid]);   }
#endif
    int semStartPost();
    int semEndPost  ();
    int semStartWait();
    int semEndWait  ();
    
    int getCounter()            { return counter;                   }
    void setCounter(int icounter) { counter=icounter;               }
    void decrementCounter()     { counter--;                        }
    int decrementCounterByAtMost( int icounter ) {
        int val;
        if( counter>=icounter ) {
            val = icounter;
        } else {
            val = counter;
        }
        counter -= val;
        return val;
    }
    
    int initializeSemaphores();
    
    void create(void* (*)(void*),void*[]);
    void join();
    
};






#endif
