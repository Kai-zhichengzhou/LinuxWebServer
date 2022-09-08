#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include "locker.h"
#include "threadpool.h"
#include "http_conn.h"

#define MAX_FD 65535 //最大的文件描述符个数
#define MAX_EVENT_NUMBER 10000 //设定监听的最大的事件数量

//添加信号捕捉
void addSignal(int sig, void(handler)(int)){

    //创建一个sigaction
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    //注册信号
    sigaction(sig, &sa, NULL);
}

extern void addfd(int epollfd, int fd, bool one_shot);
extern void removefd(int epollfd, int fd);
//修改文件描述符
extern void modfd(int epollfd, int fd, int ev);

int main(int argc, char* argv[])
{
    if( argc <= 1)
    {
        std :: cout << "按照如下格式运行: " << basename(argv[0]) << " port number\n" << std :: endl;
    }

    //获取端口号
    int port = atoi(argv[1]);

    //这里会涉及一个操作
    //如果一端已经断开连接了，另外一边继续写数据，会有一个sigpie的信号
    //所以要对该信号做处理
    //对SIGPIE信号进行处理
    addSignal(SIGPIPE, SIG_IGN);

    //创建线程池，初始化线程池

    //在这个项目的任务里，处理的任务都是http的连接和请求

    //创建一个线程池的指针
    threadpool<http_conn> * pool = NULL;
    try
    {
        pool = new threadpool<http_conn>;
    }
    catch(...)
    {
        std :: cout << "创建线程池失败，返回程序" << std :: endl;
        return 1;
    }
    

    //因为要处理多个http_conn，也就是要处理若干个连接，对应的就是若干个文件描述符
    //先初始化一个http_conn数组，之后用来存储用户的连接

    http_conn* clients = new http_conn[MAX_FD];

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);//创建一个监听的socket，用来监听是否有连接

    int ret = 0;

    //对监听socket对本地的端口进行绑定
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    int reuse = 1;
    //设定端口复用
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    //进行端口和socket的绑定
    ret = bind(listenfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    //开始监听
    ret = listen(listenfd, 8);


    //接下来就要使用IO多路复用来提升效率
    //IO多路复用提升的效率主要是通过一个监听文件描述符，来同时监听多个客户端的连接和读写事件
    //这里是使用epoll，效率最高，少了很多用户态到内核态的开销
    //通过一个在内核态的epollfd来进行监听所有epoll evetns
    //创建epoll对象，和事件数组
    

    int epollfd = epoll_create(8);
    epoll_event events[MAX_EVENT_NUMBER];

    //将监听的fd添加到epoll对象中
    //调用addfd函数，之后需要去定义这个函数，来完成添加操作
    //因为之后还需要将其他的文件描述符添加，所以封装为一个函数
    addfd(epollfd, listenfd, false);

    //将当前创建的epollfd设置为http_conn的共享epollfd
    http_conn :: epollfd = epollfd;

    while(true)
    {
        //利用epoll来进行监听所有的文件描述符
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);

        for(int i = 0; i < number; ++i)
        {
            //遍历所有的发生更新事件的文件描述符
            //获取有事件到达更新的fd
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd)
            {
                //如果是监听连接的fd有事件到达
                //创建新的连接，并把新连接的fd加入epoll
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr *)& client_address, &client_addrlength);
                
            
                if( connfd < 0)
                {
                    std :: cout << "error " << std :: endl;
                    continue;
                }

                //如果连接的客户端已经超过了最大的文件描述符数量限制，则关闭连接
                if(http_conn :: client_count >= MAX_FD)
                {
                    close(connfd);
                    continue;
                }
                //对于新建的连接，因为涉及到一些状态上的更新，比如总连接数，epoll上的文件描述符等，所以这个通过自己写一个init函数来进行逻辑处理
                //init函数是http_conn的函数， 用来处理相应的连接的逻辑
                clients[connfd].init(connfd, client_address);
                std :: cout << "client conntected" << std :: endl;
            }
            //处理完了新连接的逻辑，服务器还需要去处理读写的事件更新
            else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR) )
            {
                //如果客户端的连接出现了问题，或者说主动中断了连接
                //那么服务器这边也需要关闭相应的文件描述符
                //但并不是简单的关闭文件描述符，还有一些其他需要的状态更新，所以写个关闭连接的函数
                clients[sockfd].close_conn();
            }
            //如果当前检测到的事件是读事件，也就是客户端发送了新的数据过来
            else if(events[i].events & EPOLLIN)
            {
                //读取数据
                //调用http connection函数的读取函数
                //并将这个任务添加到线程池的请求队列
                if(clients[sockfd].read())
                {
                    //将fd的缓冲区数据，读取到封装的http_connection的类里，再将这个对象，作为任务单元，发配给线程池

                    pool->append(clients + sockfd);
                }
                //发生错误，或者对方关闭连接
                else{
                    clients[sockfd].close_conn();
                }
            }
            else if(events[i].events & EPOLLOUT)
            {
                if(!clients[sockfd].write())
                {
                    clients[sockfd].close_conn();
                }
            }

        }

    }

    //释放资源
    close(epollfd);
    close(listenfd);
    delete[] clients;
    delete pool;

    return 0;




}