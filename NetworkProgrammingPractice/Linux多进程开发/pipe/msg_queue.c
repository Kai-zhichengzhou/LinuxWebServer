#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>


//定义消息的结构，用一个结构体
struct msgbuf
{
    long msg_type; //消息类型
    char msg_text[512]; //消息内容
};


int main()
{
    int qid = 0, len = 0;
    key_t key;
    struct msgbuf msg;

    //调用ftok函数来产生标准的key
    if((key = ftok(".", )))

}