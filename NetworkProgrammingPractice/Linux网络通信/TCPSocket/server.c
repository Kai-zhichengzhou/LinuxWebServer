//TCP通信的服务器端
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>

//实现服务器端的TCP Socket
int main()
{



    //先通过socket()函数先创建socket(用于监听的socket)
    //socket函数通过类型参数们也就是协议类型以及具体的协议参数protocol来创建socket（文件描述符）
    //返回一个文件描水符，失败则返回-1
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1)
    {
        perror("socket");
        exit(-1);
    }
     



    //2.绑定socket到具体的ip和端口，也叫命名socket
    //先创建一个socket地址，之后再对这个socket地址进行绑定
    struct sockaddr_in saddr;
    //对初始化的socket address进行设置
    saddr.sin_family = AF_INET;
    //将当前的ip地址，用ip地址转换函数，通过inet的函数将地址转换为网络字节序的格式，然后存储
    //在socket address的sin_addr属性的s_addr里
    inet_pton(AF_INET, "212.71.233.112",&saddr.sin_addr.s_addr);
    //将当前主机需要绑定到当前socket的端口号port number进行到网络字节序的转换，然后存储
    saddr.sin_port = htons(9999);

    //上述的socket地址已经设置好了，然后作为参数将socket地址的地址传入bind()函数，将这个对象与文件描述符进行绑定
    int ret = bind(lfd, (struct sockaddr* )&saddr, sizeof(saddr));
    if(ret == -1)
    {
        perror("bind");
        exit(-1);
    }


    //3. 当成功将socket绑定完成后，要开始进行监听，也就是调用listen函数
    ret = listen(lfd, 8);
    if(ret == -1)
    {
        perror("listen");
        exit(-1);
    }

    //4.监听完成以后，需要调用阻塞的accept函数来接收客户端的连接
    //先初始化一个客户端的地址，用来存储连接的客户端地址信息
    //注意accept函数的address和addresslen参数都是地址
    struct sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);
    int cfd  = accept(lfd, (struct sockaddr* ) &clientAddr,  &len);

    if(cfd == -1)
    {
        perror("accept");
        exit(-1);
    }

    //5.输出客户端的信息
    //创建一个char数组，来输出客户端的ip信息
    char clientIP[16];
    inet_ntop(AF_INET, &clientAddr.sin_addr.s_addr, clientIP, sizeof(clientIP));
    unsigned short clientPort = ntohs(clientAddr.sin_port);
    printf("client ip is %s, port is  %d \n", clientIP, clientPort);

    //6. 
    //获取客户端的数据
    char recvBuf[1024] = {0};
    //调用read函数将客户端发送的数据读取到char数组
    while(1){

    
    int num = read(cfd, recvBuf, sizeof(recvBuf));
    if(num == -1)
    {
        perror("read");
        exit(-1);
    }
    else if(num> 0)
    {
        printf("recv client data :%s\n", recvBuf);
    }
    else if(num== 0)
    {
        //当读取到的数据长度为0，说明客户端断开连接
        printf("client closed");
    }


    //给客户端发送数据
    char * data = "hello I am server";
    write(cfd, "hello I am server", strlen(data));
    }
    close(lfd);
    close(cfd);

}