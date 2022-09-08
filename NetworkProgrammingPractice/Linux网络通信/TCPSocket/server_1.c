#include <unistd.h>
#include<stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>


//建立服务器

int main()
{


    //创建一个tcp socket，来监听连接
    //对应的是一个文件描述符
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1)
    {
        perror("socket");
        exit(-1);
    }

    //创建好socket以后，需要绑定，绑定到当前的某个端口
    struct sockaddr_in serveraddr;
    //需要将本地的字节序转成网络字节序
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(9999);
    //还需要将本地的ip地址转换成网络字节序
    inet_pton(AF_INET, "212.71.233.112", &serveraddr.sin_addr.s_addr);
    int ret = bind(lfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    if(ret == -1)
    {
        perror("bind");
        exit(-1);
    }

    //绑定以后，要调用listen监听函数，使得fd开始监听
    ret = listen(lfd, 8);

    //调用阻塞的函数accept来接收客户端的连接
    //初始化一个clientaddr用来存储客户端地址
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    int cfd = accept(lfd, (struct sockaddr *)&clientaddr, &len);

    if(cfd == -1)
    {
        perror("accept");
        exit(-1);
    }

    //为了验证连接，这里可以输出一下客户端的信息
    char clientIP[16];
    //从网络字节序转换为本机
    inet_ntop(AF_INET, (struct sockaddr *)&clientaddr, clientIP, sizeof(clientIP));
    unsigned short client_port = ntohs(clientaddr.sin_port);
    printf("Client IP is %s, with Port : %d", clientIP, client_port);

    





}