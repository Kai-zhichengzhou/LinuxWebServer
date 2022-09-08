

/*


    pipe()系统调用
    int pipe(int pipefd[2]);
        function: create a 匿名管道，用来进程间通信
        参数: int pipefd[2],数组是传出参数
            pipefd[0] read end 
            pipefd[1] write end

        return:
            类似操作文件
            如果成功返回0，失败返回 -1

    注意：匿名管道只能用于具有关系的进程通信

    必须注意：管道默认是阻塞的，比如在父进程里面，如果是读取数据，调用read，会等到管道里有数据了才会读取到

*/


//子进程发送数据给父进程
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main()
{

    //在fork之前创建管道

    //创建管道
    int pipefd[2];
    int ret = pipe(pipefd);
    //根据ret的结果来判定管道是否创建成功
    if(ret == 1)
    {
        perror("pipe");
        exit(0); //退出当前进程
    }

    //创建好管道以后，开始fork()
    pid_t pid = fork();

    if(pid > 1){
        //父进程
        //父进程来向pipe里面写数据
        char* str = "Nice to meet you son";
        //pipe的写端是pipefd[1]；
        write(pipefd[1], str, strlen(str));
        sleep(2);
        char* str1 = "How are you bro ?";
        write(pipefd[1], str1, strlen(str1));

    } 
    else if(pid == 0)
    {
        //子进程
        //子进程从pipefd[0]的读端读数据
        //读到子进程的buffer里
        char buffer[1024] = {0};
        int len = read(pipefd[0], buffer, sizeof(buffer));
        printf("Child recv : %s, pid : %d\n", buffer, getpid());
        sleep(2);
        len = read(pipefd[0], buffer, sizeof(buffer));
         printf("Child recv : %s, pid : %d\n", buffer, getpid());

    }
     

     return 0;

}
