#include "threadpool.h"


//定义一下threadpool的构造函数
template<typename Task>
threadpool<Task> :: threadpool(int threadNumber, int maxRequests) : _thread_number(threadNumber),
_threads(NULL), _max_requests(maxRequests), _stop(false){

    //先通过列表初始化来初始化成员变量

    //对线程池的创建做一下基本判断，比如数量《=0，或者说任务数量<=0
    if( (threadNumber <=0) || (maxRequests <= 0))
    {
        throw std :: exception();
    }

    //初始化线程池的数组
    //**关键点：对于这个线程数组，是需要所有线程之间可见且共享的，所以创建在heap上
    _threads = new pthread_t[_thread_number];

    //创建thread_number个线程，并且将线程设置为脱离线程
    //这里为什么要设置为脱离线程 -> 当创建线程后，主线程需要对子线程进行资源的回收和释放，如果主线程没有
    //释放，则由系统来释放
    for(int i = 0; i < _thread_number; ++i)
    {
        // std :: cout << "create the " << i << "th thread\n" << std ::endl;

        //调用phread_create
        //**这里注意，因为worker是静态的，static不能访问非静态，所以将this指针穿进去

        if(pthread_create(_threads + i, NULL, worker, this != 0))
        {
            //也就是创建失败
            //删除资源
            delete [] _threads;
            throw std :: exception();
        }

        //要将线程设置为脱离
        if(pthread_detach(_threads[i]) != 0)
        {
            delete [] _threads;
            throw std :: exception();
        }
    }
}

template<typename Task>
threadpool<Task> :: ~threadpool()
{
    delete [] _threads;
    _stop = true;
}


//定义线程池的添加任务的函数
//对于参数Task，处理相关逻辑，将任务添加到workqueue
//但是在添加的时候，需要通过同步工具来保证数据的读写的一致性
template<typename Task>
bool  threadpool<Task> :: append( Task * request)
{
    //当操作工作队列的时候，一定要加锁，因为它被所有线程共享
    //一个线程调用append，先调用lock
    _workqueue_lock.lock();
    
    if(_workqueue.size() > _max_requests)
    {
        //说明不可以向请求队列添加任务
        //要退出当前函数，但要先释放锁
        _workqueue_lock.unlock();
        return false;
    }
    _workqueue.push_back(request);
    _workqueue.unlock();
    //对信号量进行+ 1的操作，说明队列里有任务

    return true;

}

template<typename Task>
void* threadpool<Task> :: worker(void * arg)
{
    //静态的工作函数
    //将void*转换为threadpool*
    threadpool * pool = (threadpool *) arg;

    //线程的执行逻辑
    pool -> run();

    return pool;

}

template<typename Task>
void threadpool<Task> :: run()
{
     //run函数的逻辑：
     //首先调用run函数的是每一个新开的线程，或者说每一个线程池里的线程
     //线程要做的就是1. 需要不停的轮询，因为只有不停的轮询才能获取请求队列里面新到达的任务
     //但是需要注意同步机制，因为在轮询的时候需要判定请求队列是否为空，这时候同步很红药
     while(!_stop)
     {
         //先用信号量的P操作来看是否需要阻塞
         _queue_status.wait();
         //如果不阻塞，则上锁
         _workqueue_lock.lock();
         if(_workqueue.empty())
         {
             //是空的，则需要continue，但是在这之前释放锁
             _workqueue_lock.unlock();
             continue;
         }

         //请求队列不为空，则将请求队列的对头获取
         Task * request = _workqueue.front();
         _workqueue.pop_front();
         _workqueue_lock.unlock();

         //如果没有取到request，则continue
         if(!request) continue;

         request -> process();

     }
}

