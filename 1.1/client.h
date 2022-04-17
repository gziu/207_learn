#include<sqlite3.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>
#include<arpa/inet.h>
#include<time.h>


#define SERVERPORT 5555

struct msg_node
{
    int action;
    char chat_mode[20];  //聊天方式
    int cfd;            //聊天对象
    char id[100];             //聊天对象
    char msg[100];      //聊天内容
    char name[100];
    char passwd[100];
    int admin_flag;          //管理员标志
    char buffer[1024];
    int file_flag;
    int udp_port;
    char vip[100];
    
};

struct thread_node
{
    int cfd;
    int udp_port;
    int udp_cfd;
};

void *write_thread(void * arg);

void *read_thread(void * arg);

void file_from(int sockfd);


void file_recv(char buffer[]);
