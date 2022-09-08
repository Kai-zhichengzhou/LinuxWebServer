#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>


class locker
{
public:

    //设计互斥锁的构造函数
    //互斥锁在pthread函数里，是pthread_mutex_lock
    locker()
    {
        if( pthread_mutex_init(&m_mutex, NULL) != 0)
        {
            throw std :: exception();
        }
    }

    ~locker()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    //封装一个上锁的接口
    bool lock()
    {
        return pthread_mutex_lock(&m_mutex) == 0;
    }

    bool unlock()
    {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }

    //设计一个get函数，用来返回locker的mutex lock
    pthread_mutex_t * get()
    {
        return &m_mutex;
    }



private:

    pthread_mutex_t m_mutex;

};

//设计封装一个条件变量类
class cond
{
public:

    cond()
    {
        //创建一个cond的构造函数，用来对pthread_cond进行初始化
        if( pthread_cond_init(&m_cond, NULL) != 0)
        {
            throw std :: exception();
        }

    }
    ~cond()
    {
        pthread_cond_destroy(&m_cond) == 0;
    }

    //设计一个wait函数的接口，用来让抢占锁的线程来看cond是否满足，不满足的话挂起线程
    bool wait(pthread_mutex_t* mutex_lock)
    {
        int ret = 0;
        ret = pthread_cond_wait(&m_cond, mutex_lock);
        return ret == 0;
    }

    bool timewait(pthread_mutex_t* mutex_lock, timespec t)
    {
        int ret = 0;
        ret = pthread_cond_timedwait(&m_cond, mutex_lock, &t);
        return ret == 0;
    }

    //设计signal和广播的接口，用来让条件变量满足的时候，通知所有挂起的线程去抢占mutex lock
    bool signa()
    {
        return pthread_cond_signal(&m_cond) == 0;
    }
    bool broadcast()
    {
        return pthread_cond_broadcast(&m_cond) == 0;
    }

private:

    pthread_cond_t m_cond;
};


//设计并封装一个信号量类，通过对信号量的PV操作，来达到同步的机制

class sem
{
public:

    sem()
    {
        //信号量的初始化
        //sem_init的第二个参数，也就是PSHARED，如果是0，代表同一进程下的线程之间共享，如果非0， 代表不同进程分享
        if(sem_init(&m_sem, 0, 0) != 0)
        {
            throw std :: exception();
        }
    }

    //指定初始值的构造函数
    sem(int num)
    {
        if( sem_init(&m_sem, 0, num) != 0)
        {
            throw std :: exception();
        }
    }
    ~sem()
    {
        sem_destroy(&m_sem);

    }
    //等待信号量
    bool wait()
    {
        return sem_wait(&m_sem) == 0;
    }
    
    //增加信号量
    bool post()
    {
        return sem_post(&m_sem) == 0;
    }



private:

    sem_t m_sem;


};




#endif


