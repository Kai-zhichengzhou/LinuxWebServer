#include <unistd.h>
#include<stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>



int main()
{
    //在客户端进程也是要创建一个socket，然后绑定到自己的服务器和port上
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1)
    {
        perror("socket");
        exit(-1);
    }

    //连接服务器端，所以需要定义服务器端的地址
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(9999);
    inet_pton(AF_INET, "212.71.233.112", &serveraddr.sin_addr.s_addr);

    //连接
    int ret = connect(fd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));

    
}