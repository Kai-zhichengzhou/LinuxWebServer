#include <sys/types.h> /*文件预处理，包含waitpid，kill，raise等函数库*/
#include <unistd.h> /*文件预处理，包含进程控制函数库 */
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//定义一个结构体

typedef struct {
    char name[4];
    int age;
}person;


int main(int argc, char ** argv)
{
    pid_t pid;
    int i = 0; 
    person* p_map;
    char temp;


    //调用mmap函数来创造匿名内存映射
    p_map = (person * )mmap(NULL, sizeof(person) * 10,PROT_READ | PROT_WRITE,  MAP_SHARED | MAP_ANONYMOUS, -1, 0 );

    pid = fork();

    if(pid < 0)
    {
        perror("process");
        exit(-1);
    }
    else if(pid == 0)
    {
        //子进程
        sleep(2);
        for(i = 0; i < 5; i++){

        
            printf("子进程读取：第%d个人的年龄是: %d\n", i + 1, (*(p_map + i)).age);
            printf("子进程读取：第%d个人的年龄是: %s\n", i + 1, (*(p_map + i)).name);
        }
        munmap(p_map, sizeof(person) * 10); //解除内存映射关系
        exit(0);
    }
    else{
        temp = 'a';
        for(i = 0; i < 5; ++i)
        {
            temp += 1;
            memcpy((*(p_map + i)).name, &temp, 2);
            (*(p_map + i)).age = 20 + i;

        }
    }
    sleep(5);
    printf("父进程读取：%d\n", (*p_map).age);
    printf("解除内存映射....\n");
    munmap(p_map, sizeof(person) * 10);
    printf("解除内存映射成功\n");
}