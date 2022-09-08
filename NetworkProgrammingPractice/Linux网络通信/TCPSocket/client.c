#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{
    //创建socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1)
    {
        perror("socket");
        exit(-1);
    }


    //2.连接服务器端
    struct sockaddr_in serveraddr;
    //定义服务器端的协议族
    serveraddr.sin_family = AF_INET;
    //将服务器的ip地址转换为网路字节序存储在socket地址对象里面
    inet_pton(AF_INET, "212.71.233.112", &serveraddr.sin_addr.s_addr);
    //设置要连接的服务器socket的端口号
    serveraddr.sin_port = htons(9999);

    //客户端调用connect函数来连接服务器
    int ret = connect(fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    if(ret == -1)
    {
        perror("connect");
        exit(-1);
    }

    //3. 通信

    //给服务器写数据
    char * data = "Hello I am client";
    write(fd, data, strlen(data));


    //从服务器收数据
    char recvBuf[1024] = {0};
    while(1) {

        char * data = "hello,i am client";
        // 给客户端发送数据
        write(fd, data , strlen(data));

        sleep(1);
        
        int len = read(fd, recvBuf, sizeof(recvBuf));
        if(len == -1) {
            perror("read");
            exit(-1);
        } else if(len > 0) {
            printf("recv server data : %s\n", recvBuf);
        } else if(len == 0) {
            // 表示服务器端断开连接
            printf("server closed...");
            break;
        }

    }




    //关闭
    close(fd);



}