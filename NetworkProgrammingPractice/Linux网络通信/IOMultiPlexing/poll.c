#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/poll.h>



int main()
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in saddr;
    saddr.sin_port = htons(9999);
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;

    //调用Bind
    bind(lfd, (struct sockaddr*) &saddr, sizeof(saddr));

    //listen
    listen(lfd, 8);


    //创建并初始化检测的文件描述符数组
    struct pollfd fds[1024];
    for(int i = 0; i < 1024; i++)
    {
        fds[i].fd = -1; //进行初始化
        fds[i].events = POLLIN; //表示要检测读的事件
    }
    fds[0].fd = lfd;



    int nfds = 0;

    while(1)
    {

        //调用poll（）系统函数
        int ret = poll(fds, nfds + 1, -1);
        if(ret == -1)
        {
            perror("select");
            exit(-1);
        }
        else if (ret == 0)
        {
            continue;
        }else if(ret > 0)
        {
            //说明检测到了有fd对应的缓冲区发生了改变
            //因为检测到了个数
            if(fds[0].revents & POLLIN)
            {
                //表示有新的客户端连接进来
                struct sockaddr_in clientaddr;
                int len  =sizeof(clientaddr);
                int cfd = accept(lfd, (struct sockaddr* )&clientaddr, &len);
                //遍历从1开始，因为0是监听fd
                //这里是从头开始找哪个fd是-1，说明新来的客户端可以用
                for(int i = 1; i < 1024;i++)
                {
                    if(fds[i].fd == -1)
                    {
                        fds[i].fd = cfd;
                        fds[i].events = POLLIN;
                        break;
                    }
                }

                //更新最大的文件描述符的索引
                nfds = nfds > cfd ? nfds : cfd;
            }
            //遍历所有文件描述符
            for(int i = 1 ; i <= nfds; i++)
            {

                //注意在poll函数的调用，
                //这里i诗索引，而我们需要的文件描述符编号在数组里
                //fds[i]。fd
                if(fds[i].revents & POLLIN)
                {
                    //说明客户端发来了数据
                    char buf[1024] = {0};
                    int len = read(fds[i].fd, buf, sizeof(buf));
                    if(len == -1)
                    {
                        perror("read");
                        exit(-1);
                    }
                    else if( len == 0)
                    {
                        //len == 0说明断开了连接
                        printf("client closed\n");
                        //因为断开了connection,so remove the fd
                        close(fds[i].fd);
                        fds[i].fd = -1;
                    }
                    else if(len > 0)
                    {
                        printf("read buf = %s\n", buf);
                        write(fds[i].fd, buf, strlen(buf) + 1);
                    }
                }
            }

        }

    }
    close(lfd);


}