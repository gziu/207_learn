#include "server.h"

int main(int argc, char **argv)
{
    int sockfd;
    int cfd;
    int n;
    int udp_sockfd;

    sqlite3 *pdb;
    if(sqlite3_open("chat.db",&pdb) != SQLITE_OK)
    {
        printf("open db fail! %s \n",sqlite3_errmsg(pdb));
        exit(-1);
    }

    //创建数据库表
    creat_table(pdb);
    creat_table_chat(pdb);


    struct sockaddr_in s_addr;
    struct sockaddr_in c_addr;

    struct sockaddr_in src_addr;
    

    socklen_t c_len;
    pthread_t tid;

    Onlien *head;      //id cfd next
    Onlien *new_node;

    struct thread_node cfd_node; //cfd head id pdb

    create_link(&head);

    //结构体指向链表
    cfd_node.head = head;
    //结构体指向数据库
    cfd_node.pdb = pdb;

    struct msg_node data;

    strcpy(data.id,"100");
    strcpy(data.name,"guo");
    strcpy(data.passwd,"123");
    strcpy(data.vip,"是");

    if(-1 != find_record_id(pdb,data.id))
    {
        insert_record(pdb,&data);
    }

    //管理员设置
    printf("管理员创建成功，账号100，密码123\n");


    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error!\n");
        exit(1);
    }

    printf("sockfd successfully!\n");

    if ((udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket error!");
        exit(1);
    }

    printf("udp socket successfully!\n");
    

    

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); //设置套接字可以重复使用端口号

    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(SERVERPORT);
    s_addr.sin_addr.s_addr = inet_addr("192.168.33.136");

    if (bind(sockfd, (struct sockaddr *)&s_addr, sizeof(struct sockaddr_in)) != 0)
    {
        perror("bind error!");
        exit(1);
    }

    printf("bind successfully!\n");

    if (bind(udp_sockfd, (struct sockaddr *)&s_addr, sizeof(struct sockaddr)) < 0)
    {
        perror("bind error!");
        exit(1);
    }

    printf("udp bind successfully!\n");

    if (listen(sockfd, 20) != 0)
    {
        perror("listen error!");
        exit(1);
    }

    printf("listen successfully!\n");

    //线程池初始化
    struct threadpool *pool = threadpool_init(10, 100);

    while(1)
    {
        c_len = sizeof(struct sockaddr_in);
        //每个客户端的通信描述符 连接成功
        cfd = accept(sockfd,(struct sockaddr *)&c_addr, &c_len);
        if (cfd < 0)
        {
            perror("accept error!");
            exit(1);
            //return -1;
        }
        //printf("cfd = %d\n",cfd);

        cfd_node.cfd = cfd;  //cfd head id
        cfd_node.udp_cfd = udp_sockfd;
        printf("server main cfd = %d\n",cfd);

        printf("server main udpcfd = %d\n",udp_sockfd);

        
        //创建新节点
        //create_node(&new_node);   //id name passwd cfd next
        //new_node->cfd = cfd;
        
        //头插
        //insert_node_head(head,new_node);

        //创建线程  将cfd与head传给线程函数
        // pthread_create(&tid,NULL,(void *)chat_func,(void *)&cfd_node);
          

        //往线程池 任务队列 放任务
        threadpool_add_job(pool, (void *)chat_func,(void *)&cfd_node);


    }

    close(sockfd);

    release_link(&head);

    thread_destroy(pool);

    return 0;
}