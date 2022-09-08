#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{

    //有一个父进程，创建5个子进程
    pid_t pid;

    for(int i = 0; i < 5; ++i)
    {
        pid = fork();
        if(pid == 0) //说明当前的进程是子进程
        {
            break;
        }
    }

    if(pid > 0 ){
        //说明是父进程

        while(1)
        {
             sleep(1);
            printf("Parent pid = %d\n", getpid());
            int st;
            // int ret = waitpid(-1, &st, 0); //阻塞
            int ret = waitpid(-1, &st, WNOHANG); //非阻塞


            if(ret == -1) break;

            if(ret == 0)
            {
                //说明还有子进程存在
                continue;

            }else if (ret > 0)
            {

                if(WIFEXITED(st))
                {
                    printf("退出的状态码:%d\n", WEXITSTATUS(st));
                }
                if(WIFSIGNALED(st))
                {
                    printf("被哪个信号中断终止了：%d\n", WTERMSIG(st));
                }

                printf("child die, pid = %d\n", ret);
            }

           
        }
    }

    else if(pid == 0)
    {

        while(1)
        {
            printf("Child pid = %d\n", getpid());
            sleep(1);
        }

        exit(0);

    }

}