#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <list>
#include <vector>
#include <iostream>
#include <pthread.h>
#include  "locker.h" 

//声明并定义一个cpp的线程池类，定义为模版类是为了代码复用，T是任务类
template<typename Task>
class threadpool
{
public:

    //定义线程池的构造函数
    threadpool(int threadNumber = 8, int maxRequests = 10000);
    ~threadpool();

    //声明一个append函数（为什么 ->  append函数是我们往线程池的任务队列添加任务
    bool append(Task* request);

private:

    //声明且定义当前线程池，工作线程需要运行的函数以及业务逻辑
    //线程池会不断从请求队列里取出任务并且分配给线程，执行工作函数

    //必须要是静态的函数
    static void* worker(void * arg);

    void run();
private:

    //定义线程池需要的哪些成员
    //1.线程池的大小,也就是线程的数量

    int _thread_number;

    //描述线程池的数组， 因为是一个线程池， 所以需要一个容器来存储所有的线程（或者说线程号）
    pthread_t* _threads;

    //请求队列中，或者说handler的集合，最多允许的，等待处理的请求的数量
    //也就是请求队列的容量
    int _max_requests;

    //请求队列，或者说handler的队列
    //用链表来存储
    std :: list<Task*> _workqueue;

    //之前封装了同步工具，而同步工具主要就是用在请求队列上，保证请求队列的任务发配不会有错误
    locker _workqueue_lock;

    //用一个信号量来维护请求队列，来看是否有request需要处理，可以根据sem的值来决定是否阻塞
    sem _queue_status;

    //是否要结束线程的一个bool值
    bool _stop;
};


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
        std :: cout << "create the " << i << "th thread\n" << std ::endl;

        //调用phread_create
        //**这里注意，因为worker是静态的，static不能访问非静态，所以将this指针穿进去

        if(pthread_create(_threads + i, NULL, worker, this )!= 0)
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
    _workqueue_lock.unlock();
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






#endif