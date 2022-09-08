#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include "locker.h"
#include <sys/uio.h>

//声明一个http连接的类， 也就是任务类
class http_conn
{
public:

    
    http_conn(){}
    ~http_conn(){}


public:
    //定义一些任务类的静态变量
    static const int FILENAME_LEN = 200; //文件名大小
    static const int READ_BUFFER_SIZE = 2048; //读缓冲区的大小
    static const int WRITE_BUFFER_SIZE = 1024; //写缓冲区的大小
public:

    void init(int sockfd, const sockaddr_in& addr);

    //声明一个用于关闭当前http conn的连接的函数，并且释放掉该释放的恶资源
    void close_conn();

    void process();

    //声明读取函数，就是http_connection对象去文件描述符读取数据，并且封装为任务
    //声明非阻塞读

    bool read();
    //声明非阻塞写
    bool write();



public:
    //所有的http链接都会共享一个epoll，所以用静态的方式来创建
    static int epollfd;
    //所有的客户端的数量
    static int client_count; 

    //通信的socket，也就是对应的文件描述符
    int sockfd;

    //通信的socket地址
    sockaddr_in address;

//定义一些私有的变量
private:

    // void init();

    void unmap();
private:

    char _read_buf[READ_BUFFER_SIZE]; //http类的读缓冲区
    int _read_idx; // 标识读缓冲区已经读入的客户端数据的最后一个字节的下一个位置
    int _checked_idx;  //当前正在分析的字符在读缓冲区的位置


    bool _linger;   //http请求是否要求保持连接


    //写缓冲区的成员变量定义
    char m_write_buf[WRITE_BUFFER_SIZE];

    int m_write_idx;   //写缓冲区中待发送的字节数
    char* m_file_address;        //客户发来的是http request，会有请求的目标文件，比如html，这是客户请求的目标的文件被mmap到内存中的起始位置
    struct stat m_file_stat;   //用一个结构体stat来表示目标文件的状态。通过这个结构体我们可以判断文件是否存在，是否为目录，并且获取文件大小
    struct iovec m_iv[2];    //采用writev来执行写操作，所以定义下面两个成员，_iv_count表示被写内存块的数量
    int m_iv_count;

    int bytes_to_send; //讲要发送的数据的字节数
    int bytes_have_send;


};







#endif