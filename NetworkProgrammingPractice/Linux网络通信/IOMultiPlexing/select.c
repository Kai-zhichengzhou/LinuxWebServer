#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>



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

    //创建一个fd_set的集合， and stores the file descriptor
    //fd_set在select.h里面
    fd_set rdset, tmp;

    FD_ZERO(&rdset);
    FD_SET(lfd, &rdset);
    int maxfd = lfd;


    while(1)
    {

        tmp = rdset;
        //调用select系统函数，让kernel check any fd has data
        int ret = select(maxfd + 1, &tmp, NULL, NULL, NULL);
        //if set the timeval for select, after the timeval ,if still no detecting
        //continue running
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
            if(FD_ISSET(lfd, &tmp))
            {
                //表示有新的客户端连接进来
                struct sockaddr_in clientaddr;
                int len  =sizeof(clientaddr);
                int cfd = accept(lfd, (struct sockaddr* )&clientaddr, &len);
                //要将新的连接的文件描述符加入到RDSET的集合中
                FD_SET(cfd, &rdset);
                //并且加入之后，要更新maxfd
                maxfd = maxfd > cfd  ? maxfd : cfd;

            }

            //遍历所有文件描述符
            for(int i = lfd + 1 ; i <= maxfd; i++)
            {
                if(FD_ISSET(i, &tmp))
                {
                    //说明客户端发来了数据
                    char buf[1024] = {0};
                    int len = read(i, buf, sizeof(buf));
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
                        close(i);
                        FD_CLR(i, &rdset);
                    }
                    else if(len > 0)
                    {
                        printf("read buf = %s\n", buf);
                        write(i, buf, strlen(buf) + 1);
                    }
                }
            }

        }

    }


}