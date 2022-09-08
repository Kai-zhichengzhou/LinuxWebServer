
/*

       #include <sys/types.h>
       #include <sys/stat.h>
       通过函数： int mkfifo(const char*  pathname, mode_t mode);


       int mkfifo(const char * pathname, mode_t mode);

       参数：
        -pathname: 管道名称的路径
        -mode : 文件的权限，和open函数的mode是一样的，是一个八进制的数字

    返回值：

        如果成功了返回0， 失败了返回 -1，并且设置错误号


*/


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h> 


int main()
{
    int ret = mkfifo("fifo1", 0664);

    if(ret == -1)
    {
        perror("mkfifo");
        exit(0);
    }
}

