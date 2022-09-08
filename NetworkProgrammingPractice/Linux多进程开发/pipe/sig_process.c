#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>

//定义信号处理方式
//进程对特定的信号注册相应的信号处理函数
void fun_ctrl_c()
{
    //自定义信号处理函数
    printf("\t 你按了Ctrl + C哦 : \n");
    printf("\t 信号处理函数： 有什么要处理，在处理函数中编写逻辑！！\n");

    printf(" \t 此例子不处理， 重新恢复SIGINT系统默认处理\n");

    (void) signal(SIGINT, SIG_DFL); //重新恢复SIGINT信号的系统默认处理


}


int main()
{
    (void) signal(SIGINT, fun_ctrl_c); //如果按了ctrl + C， 调用func_ctrl_c函数
    printf("主程序： 程序进入一个无限循环\n");

    while(1)
    {
        printf("这是一个无限循环（要退出请按Ctrl + C） ！\n");
        sleep(3);
    }

    exit(0);

}