#include "ThreadsWithCounter.H"
#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef NAMED_SEMAPHORES
#define SEMAPHORE_NAME_LEN 12
#define SEMAPTHORE_MAX_ATTEMPTS 100
#endif

void rand_str(char *dest, size_t length);

using namespace std;

class EnlargeArg;

ThreadsWithCounter::ThreadsWithCounter(int iNTHREADS) :
NTHREADS(iNTHREADS),sem_start(0),sem_end(0),thread_ids(0), counter(0) {
    if(NTHREADS>0) {
        pthread_mutex_init(&lock,NULL);
        thread_ids=new pthread_t[NTHREADS];
#ifdef NAMED_SEMAPHORES
        sem_start       = new sem_t*[NTHREADS];                                     // Semaphores
        sem_end         = new sem_t*[NTHREADS];
        sem_start_names = new char*[NTHREADS];                                      // Names of semaphores
        sem_end_names   = new char*[NTHREADS];
        srand ((unsigned int)time(NULL));                                           // Initialize random number generators, will be used to generate random names for semaphores
#else
        sem_start       = new sem_t[NTHREADS];
        sem_end         = new sem_t[NTHREADS];
#endif
        //        pthread_t thread_ids = 0;                                                 // MM: what was this for?
    }
}

ThreadsWithCounter::~ThreadsWithCounter() {
    if(NTHREADS>0) {
        pthread_mutex_destroy(&lock);
#ifdef NAMED_SEMAPHORES
        for(int i=0;i<NTHREADS;i++) {
            if( sem_close(sem_start[i])==-1 ) {
                cout << "\n Couldn't close sem_start[" << i << "]" << endl;
                assert(0);
            }
            if (sem_close(sem_end[i])==-1 ) {
                cout << "\n Couldn't close sem_end[" << i << "]" << endl;
                assert(0);
            }
            if( sem_start_names[i]!=0 ) {
                if( sem_unlink(sem_start_names[i])!=0 ) {
                    cout << "\n Couldn't unlink sem_start[" << i << "]" << endl;
                    assert(0);
                }
                delete [] sem_start_names[i];
                sem_start_names[i] = 0;
            }
            if( sem_end_names[i]!=0 ) {
                if( sem_unlink(sem_end_names[i])!=0 ) {
                    cout << "\n Couldn't unlink sem_end[" << i << "]" << endl;
                    assert(0);
                }
                delete [] sem_end_names[i];
                sem_end_names[i] = 0;
            }
        }
        delete [] sem_start_names;
        delete [] sem_end_names;
#endif
        delete [] sem_start;
        delete [] sem_end;
        delete [] thread_ids;
    }
}

int ThreadsWithCounter::initializeSemaphores() {
    
    bool SemaphoreStartSuccess = false;
    bool SemaphoreEndSuccess = false;
    
#ifdef NAMED_SEMAPHORES
    for(int i=0;i<NTHREADS;i++) {
        sem_start_names[i] = new char [SEMAPHORE_NAME_LEN];
        sem_start_names[i][0] = '/';
        for( int p=1; p<=SEMAPTHORE_MAX_ATTEMPTS; p++) {
            rand_str(&(sem_start_names[i][1]), SEMAPHORE_NAME_LEN-2);
            if( (sem_start[i]=sem_open(sem_start_names[i],O_CREAT | O_EXCL,0644,0))!=SEM_FAILED ) {
                SemaphoreStartSuccess = true;
                break;
            }
        }
        if( !SemaphoreStartSuccess ) {
            cout << "\n Failed to create sem_start[" << i << "]";
            assert(0);
            break;
        }
        sem_end_names[i] = new char [SEMAPHORE_NAME_LEN];
        sem_end_names[i][0] = '/';
        for( int p=1; p<=SEMAPTHORE_MAX_ATTEMPTS; p++) {
            rand_str(&(sem_end_names[i][1]), SEMAPHORE_NAME_LEN-2);
            if( (sem_end[i]=sem_open(sem_end_names[i],O_CREAT | O_EXCL,0644,0))!=SEM_FAILED)    {
                SemaphoreEndSuccess = true;
                break;
            }
        }
        if( !SemaphoreEndSuccess ) {
            cout << "\n Failed to create sem_end[" << i << "]";
            assert(0);
            break;
        }
    }
#else
    for(int i=0;i<NTHREADS;i++) {
        SemaphoreStartSuccess   = (sem_init(&sem_start[i],0,0) == 0);
        SemaphoreEndSuccess     = (sem_init(&sem_end[i],0,0) == 0);
    }
#endif
    return !(SemaphoreStartSuccess & SemaphoreEndSuccess);
}

void ThreadsWithCounter::create(void * (*start_routine)(void*),void* args[]) {
    //int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
    //                        void *(*start_routine) (void *), void *arg);
    
    for(int t=0;t<NTHREADS;t++) {
        int val=pthread_create(&thread_ids[t],NULL,start_routine,args[t]);
        if(val) {
            cerr << "\n Failed to create thread " << t << endl;
            assert(0);
        }
        //semStartPost(t);
    }
}

int ThreadsWithCounter::semStartPost() {
    int val = 0;
    for(int t=0;t<NTHREADS;t++) {
        int val=semStartPost(t);
        if(val!=0) {
            cerr << "\n semStartPost failed for thread " << t << endl;
            return val;
        }
    }
    return val;
}

int ThreadsWithCounter::semEndPost() {
    int val=0;
    for(int t=0;t<NTHREADS;t++) {
        int val=semEndPost(t);
        if(val!=0) {
            cerr << "\n semEndPost failed for thread " << t << endl;
            return val;
        }
    }
    return val;
}

int ThreadsWithCounter::semStartWait() {
    int val=0;
    for(int t=0;t<NTHREADS;t++) {
        int val=semStartWait(t);
        if(val!=0) {
            cerr << "semStartWait failed for thread " << t << endl;
            return val;       }
        
    }
    return val;
}

int ThreadsWithCounter::semEndWait() {
    int val=0;
    for(int t=0;t<NTHREADS;t++) {
        int val=semEndWait(t);
        if(val!=0) {
            cerr << "semEndWait failed for thread " << t << " with value " << val << endl;
            //            return val;
        }
    }
    return val;
}



void ThreadsWithCounter::join() {
    
    void* exit_status;
    for(int t=0;t<NTHREADS;t++) {
        pthread_join(thread_ids[t],&exit_status);
    }
}



void rand_str(char *dest, size_t length) {
    char charset[] = "0123456789"
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    
    while (length-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }
    *dest = '\0';
}





int* randomPermutation(int n)
{
    int* r = new int[n];
    for(int i=0;i<n;++i){
        r[i]=i+1;
    }
    for(int i=0; i<n; ++i){
        int randIdx = rand() % n;
        swap(r[i], r[randIdx]);
    }
    
    return r;
}
