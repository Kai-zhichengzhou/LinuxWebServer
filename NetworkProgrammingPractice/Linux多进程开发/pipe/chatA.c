#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
int main()
{
    //1. 判断有名管道文件是否存在
    int ret  = access("fifo1", F_OK);
    if(ret == -1)
    {
        //说明文件不存在
        printf("管道不存在，创建对应的有名管\n");
        mkfifo("fifo1", 0664);
        
    }

    //2.创建管道2
    ret = access("fifo2", F_OK);
    if(ret == -1)
    {
        //文件不存在
        printf("管道不存在，创建对应的有名管\n");
        ret = mkfifo("fifo2", 0664);

    }

    //管道创建好了以后，以只写的方式打开管道1，以只读的方式打开管道2
    int fdw = open("fifo1", O_WRONLY); //返回值是文件描述符

    printf("打开管道fifo1成功，等待写入....\n");

    //以只读的方式打开管道fifo2
    int fdr = open("fifo2", O_RDONLY);

    printf("打开管道fifo2成功，等待读取...\n");
    
    char buf[128];
    //4.循环读写数据
    while(1)
    {
        //获取输入
        memset(buf, 0, 128);

        fgets(buf, 128, stdin);

        //写数据
       ret =  write(fdw, buf, strlen(buf));
       if(ret == -1)
       {
           perror("write");
           exit(0);
       }


       //5.读数据
       memset(buf, 0, 128);
       ret = read(fdr, buf, 128);
        if(ret <=0) // 0代表对面把管道关闭了
       {
           perror("read");
           exit(0);
       }
       printf("buf : %s\n", buf);


    }
    close(fdr);
    close(fdw);


}