#include "client.h"

extern pthread_mutex_t mutex;

int main(int argc,char **argv)
{
    if(argc != 2)
    {
        printf("need input udp port != 5555 \n");
        exit(0);
    } 

    int sockfd;
    int ret;
    int udp_sockfd;

    pthread_mutex_init(&mutex,NULL);

    struct sockaddr_in s_addr;
    struct sockaddr_in c_addr;

    struct sockaddr_in udp_addr;

    struct thread_node cfd_node;

    pthread_t tid_read;
    pthread_t tid_write;
    pthread_t tid_file;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error!\n");
        exit(1);
    }

    printf("sockfd successfully!\n");

    printf("tcp cfd = %d\n",udp_sockfd);

    if ((udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket error!");
        exit(1);
    }

    printf("udp socket successfully!\n");

    printf("mu udp cfd = %d\n",udp_sockfd);

    int opt = 1;
    setsockopt(udp_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); //设置套接字可以重复使用端口号

    //udp 5556
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(atoi(argv[1]));
    udp_addr.sin_addr.s_addr = inet_addr("192.168.33.136");

    //tcp  5555
    bzero(&s_addr,sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(SERVERPORT);
    s_addr.sin_addr.s_addr = inet_addr("192.168.33.136");

    if (bind(udp_sockfd, (struct sockaddr *)&udp_addr, sizeof(struct sockaddr)) < 0)  //绑定ip和端口号
    {
        perror("bind error!");
        exit(1);
    }

    printf("udp bind successfully!\n");

    if(connect(sockfd,(struct sockaddr *)&s_addr,sizeof(struct sockaddr_in)) != 0)  
    {
        perror("connect error!");
        exit(1);
    }

    cfd_node.cfd = sockfd;
    cfd_node.udp_cfd = udp_sockfd;
    cfd_node.udp_port = atoi(argv[1]);

    ret = pthread_create(&tid_read,NULL,(void *)read_thread,(void *)&cfd_node);

    ret = pthread_create(&tid_write,NULL,(void *)write_thread,(void *)&cfd_node);

    //ret = pthread_create(&tid_file,NULL,(void *)file_recv,(void*)&sockfd);

    pthread_detach(tid_write);
    pthread_join(tid_read,NULL);
    //pthread_join(tid_write,NULL);
    //pthread_join(tid_file,NULL);
    //pthread_detach(tid_file);



    close(sockfd);
    pthread_mutex_destroy(&mutex);

    return 0;
}