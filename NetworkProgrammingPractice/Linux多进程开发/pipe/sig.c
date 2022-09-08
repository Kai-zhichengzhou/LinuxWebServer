#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{
    pid_t pid;

    pid = fork();

    //创建子进程
    int ret;
    if(pid < 0)
    {
        perror("创建子进程失败");
        exit(1);
    }

    else if(pid == 0)
    {
        //代表了子进程
        //为了使子进程不在父进程发出信号前结束，子进程要发送sigstop，使得自己暂停
        raise(SIGSTOP); //调用raise函数。raise函数发送信号给进程或自身
        exit(0);
    }

    else
    {
        printf("父进程");
        if((waitpid(pid, NULL, WNOHANG) == 0))
        {
            //调用KIll函数，发送SIGKILL的信号来结束子进程
            if(ret = kill(pid, SIGKILL) == 0)
            {
                printf("用kill函数的返回值是：%d, 发出的SIGKILL信号结束的pid: %d\n", ret, pid);

            }
            else{
                perror("Kill函数执行失败");
            }
        }
    }





}