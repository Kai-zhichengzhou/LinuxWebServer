#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "locker.h"


/*Implementation of the threadpool class */
/*Define class as template class for code reuse, the typename T stands for task */
template<typename T>
class threadpool {
public:

    /* thread_number stands for the total numbers threads to be created in the threadpool */
    /* max_requests stands for the max number of requests in requesting queue */
    threadpool(int thread_number = 8, int max_requests = 10000);
    ~threadpool();
    bool append(T* request);

private:
    /* worker function to implement the thread logic */
    /*this function keep taking new task in the queue to process */
    static void* worker(void* arg);
    void run();

private:
    // number of threads
    int m_thread_number;  
       
    // array for threads in threadpool 
    pthread_t * m_threads;

    // max requests number in working queue
    int m_max_requests; 
    
    // working queue
    std::list< T* > m_workqueue;  

    // mutex lock for working queue
    locker m_queuelocker;   

    // indicates if there is task to be processed 
    sem m_queuestat;

    // indicate if stops thread        
    bool m_stop;                    
};

template< typename T >
threadpool< T >::threadpool(int thread_number, int max_requests) : 
        m_thread_number(thread_number), m_max_requests(max_requests), 
        m_stop(false), m_threads(NULL) {

    if((thread_number <= 0) || (max_requests <= 0) ) {
        throw std::exception();
    }

    // initialize the threads array 
    m_threads = new pthread_t[m_thread_number];
    if(!m_threads) {
        throw std::exception();
    }


    //create multiple threads, and set them as detached threads
    for ( int i = 0; i < thread_number; ++i ) {
        printf( "create the %dth thread\n", i);
        if(pthread_create(m_threads + i, NULL, worker, this ) != 0) {
            delete [] m_threads;
            throw std::exception();
        }
        
        if( pthread_detach( m_threads[i] ) ) {
            delete [] m_threads;
            throw std::exception();
        }
    }
}

/* deconstruct the thread pool */
template< typename T >
threadpool< T >::~threadpool() {
    delete [] m_threads;
    m_stop = true;
}

template< typename T >
bool threadpool< T >::append( T* request )
{
    // lock the queue when accessing the working queue for sychronizing 
    m_queuelocker.lock();
    if ( m_workqueue.size() > m_max_requests ) {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

template< typename T >
void* threadpool< T >::worker( void* arg )
{
    threadpool* pool = ( threadpool* )arg;
    pool->run();
    return pool;
}


/* implement the processing logic of the thread */
/* Take the request from the work queue and run the task */
template< typename T >
void threadpool< T >::run() {

    while (!m_stop) {
        m_queuestat.wait();
        m_queuelocker.lock();
        if ( m_workqueue.empty() ) {
            m_queuelocker.unlock();
            continue;
        }
        T* request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if ( !request ) {
            continue;
        }
        request->process();
    }

}

#endif
