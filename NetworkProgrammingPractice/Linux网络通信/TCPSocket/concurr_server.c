//TCP通信服务器并发： 多进程实现TCP服务器并发

#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
int main
{
    //创建socket
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1)
    {
        perror("socket");
        exit(-1);
    }

    //然后进行绑定
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htnos(9999);
    saddr.sin_addr.s_addr = INADDR_ANY;
    int ret = bind(lfd, (struct sockaddr* )&saddr, sizeof(saddr));
    if(ret == -1)
    {
        perror("bind");
        exit(-1);
    }


    //开始监听
    ret = listen(lfd, 8);
    if(ret == -1)
    {
        perror("listen");
        exit(-1);
    }

    while(1)
    {
        //等待客户端连接
        //循环调用accept

        然后fork子进程
    }


    //监听成功

}