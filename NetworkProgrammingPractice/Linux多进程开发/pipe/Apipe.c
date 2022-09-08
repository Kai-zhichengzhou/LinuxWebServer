#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main()
{
    //用父进程发送信息给父子进程
    //在fork之前先创建管道
    int pipefd[2];
    //调用系统调用来创建匿名管道
    int ret = pipe(pipefd);
    if(ret == -1)
    {
        perror("pipe");
        exit(0);
    }

    pid_t pid = fork();
    if(pid > 1)
    {
        //父进程去向管道写数据
        char* data = "Nice To Meet you Son";
        
        //pipe的写端是pipefd[1], 读端是pipefd[0]；
        write(pipefd[1], data, strlen(data));
        sleep(2);

        //继续写
        char* data2 = "How are you bro";
        write(pipefd[1], data2, strlen(data2));

    }
    else if(pid == 0)
    {
        //子进程
        //子进程从pipefd[0]读数据，读到子进程的buffer
        char buffer[824] = {0};
        int len = read(pipefd[0], buffer, sizeof(buffer));
        printf("The child process receive: %s, pid : %d\n", buffer, getpid());
        // sleep(2);
        bzero(buffer, 824);
        len = read(pipefd[0], buffer, sizeof(buffer));

        printf("The child process receive: %s, pid : %d\n", buffer, getpid());

        
    }


    return 0;

}