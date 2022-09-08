#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/epoll.h>

int main()
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1)
    {
        perror("socket");
        exit(-1);
    }

    
    //绑定
    struct sockaddr_in serveraddr;
    serveraddr.sin_family =AF_INET;
    serveraddr.sin_port = htons(8824);
    inet_pton(AF_INET, "212.71.233.112", &serveraddr.sin_addr.s_addr);
    int ret = bind(lfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if(ret == -1)
    {
        perror("bind");
        exit(-1);
    }

    ret = listen(lfd, 8);


    //调用epoll create创建epoll实例
    int epfd = epoll_create(100);

    
    
}