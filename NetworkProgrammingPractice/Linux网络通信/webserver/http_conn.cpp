#include "http_conn.h"


//epoll oneshot ->为了使一个线程操作一个socket
//向epoll中添加需要监听的文件描述符


int http_conn :: epollfd = 0;
int http_conn :: client_count = 0;
void http_conn :: process()
{
    
}
void addfd(int epollfd, int fd, bool one_shot)
{
    //先创建一个epoll event，用来配置事件
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLRDHUP;

    if(one_shot)
    {
        event.events | EPOLLONESHOT;
    }

    //通过调用epoll_ctl函数，将当前fd添加到epollfd里面
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);

}
void removefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

//修改文件描述符， 重置socket上EPOLLONESHOT事件，以确保下一次可读时，
//EPOLLIN事件能被触发
void modfd(int epollfd, int fd, int ev)
{
    //创建一个epoll event
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);

}

//定义一下，当有新连接的时候，http connection的init函数，也就是如果新建一个连接，怎么做处理
//初始化连接，外部调用初始化socket地址
void http_conn :: init(int sockfd, const sockaddr_in& addr)
{
    //原本http conn是初始化在clients的数组里，也就是所有信息都是没有配置的
    //那么init实际就是初始化配置

    //将接收到的sockfd和客户端地址赋值给address属性
    this -> sockfd = sockfd;
    this -> address = addr;

    //端口复用
    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    //同时要将负责新的连接的fd添加到epoll里，让epoll去监听
    addfd(epollfd, sockfd, true);

    client_count++;
    //稍后去定义一个init函数，实际就是将连接的所有信息初始化为0
    // init();
}

//定义一下关闭http连接的函数逻辑

void http_conn::close_conn()
{
    if(sockfd != -1)
    {
        //关闭连接，要先将文件描述符从epoll里面移除，不再需要监听
        removefd(epollfd, sockfd);
        sockfd = -1;
        //客户-1
        client_count--;

    }
}


//如何去读取数据呢
//循环读取客户数据，直到没有数据可读或者对方关闭连接
bool http_conn::read()
{
    if(_read_idx >= READ_BUFFER_SIZE)
    {
        //如果当前缓冲区读取的客户端数据位置已经超出了缓冲区大小
        return false;
    }

    int bytes_read = 0;

    while(true)
    {
        //从缓冲区里+readidx的索引未知开始保存数据，讲读取到的数据保存到http的缓存区
        bytes_read = recv(sockfd, _read_buf + _read_idx, READ_BUFFER_SIZE - _read_idx, 0);

        //根据读取到的字节进行判定
        if(bytes_read == -1)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                //说明没有数据
                break;
            }
            return false;
        }
        else if (bytes_read == 0)
        {
            //对方关闭连接
            return false;
        }

        _read_idx += bytes_read; //更新当前http缓冲区读取到的字节数据
    }
    return true;

}


//对内存映射区执行munmap操作
void http_conn :: unmap()
{
    //涉及到文件地址
    if(m_file_address)
    {
        munmap(m_file_address, m_file_stat.st_size);
        m_file_address = 0;
    }
}
//写http响应的函数，就是服务器根据http的请求回应给客户端的内容
bool http_conn:: write()
{


    int temp = 0;

    //先检测当下http_conn连接要发送的字节，为0说明不发
    if(bytes_to_send == 0)
    {
        //要修改当前http_conn连接对应的文件描述符,其实就是重置
        modfd(epollfd, sockfd, EPOLLIN);
        // init();
        return true;
    }

    while(1)
    {
        //分散写
        temp = writev(sockfd, m_iv, m_iv_count);
        if(temp <= -1)
        {
            //如果tcp写缓冲没有空间，则等待下一轮EPOLLOUT事件
            //在此期间，服务器无法立即接受同一客户的下一个请求
            if(errno == EAGAIN)
            {
                modfd(epollfd, sockfd, EPOLLOUT);
                return true;
            }
            unmap();
            return false;

        }

        //更新发送了多少数据的字节
        bytes_have_send += temp;
        //减少待发送的字节
        bytes_to_send -= temp;

        if (bytes_have_send >= m_iv[0].iov_len)
        {
            m_iv[0].iov_len = 0;
            m_iv[1].iov_base = m_file_address + (bytes_have_send - m_write_idx);
            m_iv[1].iov_len = bytes_to_send;
        }
        else
        {
            m_iv[0].iov_base = m_write_buf + bytes_have_send;
            m_iv[0].iov_len = m_iv[0].iov_len - temp;
        }

        if (bytes_to_send <= 0)
        {
            // 没有数据要发送了
            unmap();
            modfd(epollfd, sockfd, EPOLLIN);

            if (_linger)
            {
                // init();
                return true;
            }
            else
            {
                return false;
            }
        }

    }



    

}
