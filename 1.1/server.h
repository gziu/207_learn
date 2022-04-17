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
#include<time.h>
#include<signal.h>
#include<arpa/inet.h>


#define SERVERPORT 5555

//在线用户链表
struct online_client
{
    char id[100];
    int cfd;
    int udp_cfd;
    //char name[100];
    //char password[100];
    int admin_flag;
    int forbid_flag;  
    int udp_port;

    //UDP地址通信
    //struct sockaddr_in c_addr;
    struct online_client *next;
};

typedef struct online_client Onlien;

struct thread_node
{
    //char id[100];
    int cfd;
    Onlien *head;
    sqlite3 *pdb;
    int udp_cfd;
};

struct msg_node
{
    int action;
    char chat_mode[20];  //聊天方式
    int cfd;            //聊天对象
    char id[100];             //聊天对象
    char msg[100];      //聊天内容
    char name[100];
    char passwd[100];
    int admin_flag;         //管理员标志
    char buffer[1024];
    int file_flag;
    int udp_port;
    char vip[100];
};

void creat_table(sqlite3 *pdb);

void create_link(Onlien **head);

void create_node(Onlien ** new_node);

void is_malloc_ok(Onlien *new_node);

void insert_node_head(Onlien *head, Onlien * new_node);

void release_link(Onlien **head);

void delete_node(struct thread_node *cfd_node);

void *chat_func(void *arg);

void msg_send_recv(struct thread_node * cfd_node);

void visit_link_all(struct thread_node * cfd_node,char msg[],struct sockaddr_in dest_addr);

void visit_link_one(struct thread_node * cfd_node,struct msg_node *msg_one,struct sockaddr_in *dest_addr);

void register_id(struct thread_node * cfd_node,struct msg_node *data);

void insert_record(sqlite3 *pdb,struct msg_node *data);

int find_record_id(sqlite3 *pdb,char *id);

void user_login_id(struct thread_node * cfd_node,struct msg_node *data);

int find_record_passwd(sqlite3 *pdb,char *passwd,char *id);

int inspect_online_user(struct thread_node * cfd_node,struct msg_node *data);

int inspect_own_onlien(struct thread_node *cfd_node);

int looking_onlien_users(struct thread_node *cfd_node);

int forbid_user_say(struct thread_node *cfd_node,char *ID);

int cancle_forbid_chat(struct thread_node *cfd_node,char *ID);

void kick_user(struct thread_node *cfd_node,char * ID);

void daminister_user(struct thread_node *cfd_node);

void logoff_account(struct thread_node *cfd_node,struct msg_node *data);

int delete_onlien_user(struct thread_node *cfd_node,struct msg_node *data);

int delete_record(sqlite3 *pdb,char *id);

void revoke_daminister_user(struct thread_node *cfd_node);

void file_recv(struct thread_node *cfd_node,struct msg_node *data);

void creat_table_chat(sqlite3 *pdb);

void insert_chattable(sqlite3 *pdb,char* fromid, char* toid, char *chatrecord);

void inquire_chattable(sqlite3 *pdb,int cfd,struct msg_node *data,struct thread_node *cfd_node);

char* search_ownid(struct thread_node  *cfd_node);

char * inquire_vip(sqlite3 * pdb, char * id);


int inquire_port(struct thread_node * cfd_node,char *id);


//*********************************************
// 定长线程池实现
struct job                //存放线程函数，和传参
{
    void *(*func)(void *arg); //函数指针
    void *arg;
    struct job *next;
};

struct threadpool
{
    int thread_num;  //已开启线程池已工作线程
    pthread_t *pthread_ids;  // 薄脆线程池中线程id


    struct job *head;
    struct job *tail;  // 任务队列的尾
    int queue_max_num;  //任务队列的最多放多少个
    int queue_cur_num;  //任务队列已有多少个任务

    pthread_mutex_t mutex;
    pthread_cond_t queue_empty;    //任务队列为空
    pthread_cond_t queue_not_emtpy;  //任务队列不为空
    pthread_cond_t queue_not_full;  //任务队列不为满

    int pool_close;   //线程退出标志
};

void * threadpool_function(void *arg);//任务队列取数据 执行线程函数

struct threadpool * threadpool_init(int thread_num, int queue_max_num);

void threadpool_add_job(struct threadpool *pool, void *(*func)(void *), void *arg);//增加任务

void thread_destroy(struct threadpool *pool);
