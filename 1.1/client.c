#include "client.h"

int admin_flag;
int forbid_flag = 0;
pthread_mutex_t mutex;

void *read_thread(void * arg)
{
    char recvline[100];
    char message[100];
    int sockfd;
    int len;
    int udp_cfd;

    int t = 0;

    struct thread_node c_node;
    struct sockaddr_in dest_addr;

    c_node = *((struct thread_node *)arg);

    //int admin;
    //struct msg_node *msg_text;
    //msg_text = (struct msg_node *)malloc(sizeof(struct msg_node));
    
    //sockfd = *((int*)arg);
    sockfd = c_node.cfd;
    udp_cfd = c_node.udp_cfd;

    while(1)
    {
        memset(recvline, 0, sizeof(recvline));
        len = recv(sockfd,recvline,100,0);
        if(len == 0 || len == -1)
        {
            pthread_exit(NULL);
            //exit(-1);
        }
        recvline[len] = '\0';

        if(strcmp(recvline,"登录成功") == 0)
        {
            printf("%s\n",recvline);
            memset(recvline, 0, sizeof(recvline));
            recv(sockfd,recvline,100,0);
            //printf("%s\n",recvline);
            if(strcmp(recvline,"是") == 0)
            {
                printf("登录管理员成功\n");
                admin_flag = 1;
            }
            else
            {
                admin_flag = 0;
            }


        }

        else if(strcmp(recvline,"聊天记录如下:") == 0)
        {
            printf("%s\n",recvline);
            char arr[5][100];
            
            while(1)
            {
                
                memset(arr, 0, sizeof(arr));

                for(int i = 0; i < 5; i++)
                {
                    len = recv(sockfd,arr[i],100,0);
                    //printf("len = %d\n",len);

                    if(strcmp(arr[i],"发送结束") == 0)
                    {
                       t = 1;
                       break;
                    }

                    if(len == 0)
                    {
                        t = 1;
                        break;
                    }
                    if(len == -1)
                    {
                         t = 1;
                        break;
                    }

                    arr[i][len] = '\0';
                }

                if(t == 1)
                {
                    t = 0;
                    break;
                }

                for(int i = 0; i < 5; i++)
                {
                    printf("%-25s",arr[i]);
                }

                printf("\n");
            } 

           
        }

        else if(strcmp(recvline,"私聊") == 0)
        {
            //printf("私聊中\n");
            //printf("sdp_cfd = %d\n",udp_cfd);

            memset(message, 0, sizeof(message));
            int len = sizeof(struct sockaddr_in);
            int recv_len = recvfrom(udp_cfd, message, sizeof(message), MSG_NOSIGNAL, 
                                    (struct sockaddr *)&dest_addr, &len);  //保存“数据发送方”的ip和端口 并打印
            //printf("recv_len = %d\n",recv_len);
            if(recv_len == -1)
            {
                perror("recv error");
            }
            printf("mes = %s\n",message);

        }
        else if(strcmp(recvline,"群聊") == 0)
        {
            //printf("群聊中\n");
            //printf("sdp_cfd = %d\n",udp_cfd);

            memset(message, 0, sizeof(message));
            int len = sizeof(struct sockaddr_in);
            int recv_len = recvfrom(udp_cfd, message, sizeof(message), MSG_NOSIGNAL, 
                                    (struct sockaddr *)&dest_addr, &len);  //保存“数据发送方”的ip和端口 并打印
            //printf("recv_len = %d\n",recv_len);
            if(recv_len == -1)
            {
                perror("recv error");
            }
            printf("mes = %s\n",message);

        }

        else if(strcmp(recvline,"同意管理员身份申请") == 0)
        {
            printf("%s\n",recvline);
            admin_flag = 1;
           
        }
        else if(strcmp(recvline,"撤销管理员身份成功") == 0)
        {
            printf("%s\n",recvline);
            //pthread_mutex_lock(&mutex);
            admin_flag = 0;
            //pthread_mutex_unlock(&mutex);
            //printf("admin_flag = %d\n",admin_flag);
        }
        else if(strcmp("你已经被管理员禁言",recvline) == 0)
        {
            printf("%s\n",recvline);
            pthread_mutex_lock(&mutex);
            forbid_flag = 1;
            //printf("forbid_flag = %d\n",forbid_flag);
            pthread_mutex_unlock(&mutex);
        }
        else if(strcmp("你已经被管理员解禁",recvline) == 0)
        {
            printf("%s\n",recvline);
            pthread_mutex_lock(&mutex);
            forbid_flag = 0;
            //printf("forbid_flag = %d\n",forbid_flag);
            pthread_mutex_unlock(&mutex);
        }
        else if(strcmp("你已经被管理员剔除",recvline) == 0)
        {
            printf("%s\n",recvline);
            admin_flag = 0;
            forbid_flag = 0;
            //printf("admin_flag = %d\n",admin_flag);
            //printf("forbid_flag = %d\n",forbid_flag);
        }
        else if(strcmp("发送文件中",recvline) == 0)
        {
            printf("接受文件中....\n");
            char buffer[1024];
            memset(buffer,0,sizeof(buffer));
            int file_len = recv(sockfd,buffer,1024,0);
            printf("file_len = %d\n",file_len);
            buffer[file_len] = '\0';
            //printf("buffer = %s\n",buffer);
            file_recv(buffer);
            printf("接收文件成功\n");
        }
        else
        {
            //printf("***********\n");
            //recvline[len] = '\0';
            printf("%s\n",recvline);
        }
        
    }
    pthread_exit(NULL);
}

void *write_thread(void * arg)
{
    char sendline[100];
    struct msg_node msg_text;
    char message[100];

    int choice;
    int sockfd;
    int udp_cfd;
    int udp_port;

    struct sockaddr_in dest_addr;  //目标地址

    struct thread_node c_node;
    c_node = *((struct thread_node *)arg);

    //sockfd = *((int *)arg);
    sockfd = c_node.cfd;
    udp_cfd = c_node.udp_cfd;
    udp_port = c_node.udp_port;
    

    while(1)
    {
        printf(" -------------------------- \n");
        printf("|请执行你的操作:           |\n");
        printf("|        1.群 聊(保存记录) |\n");
        printf("|        2.私 聊(保存记录) |\n");
        printf("|        3.注 册           |\n");
        printf("|        4.登 录           |\n");
        printf("|        5.查看在线人数    |\n");
        printf("|        6.请求管理员身份  |\n");
        printf("|        7.禁 言           |\n");
        printf("|        8.解 禁           |\n");
        printf("|        9.踢 人           |\n");
        printf("|        10.撤销管理员身份 |\n");
        printf("|        11.注 销 账 号    |\n");
        printf("|        12.文 件 传 输    |\n");
        printf("|        13.查看本地记录   |\n");
        printf(" -------------------------- \n");

        scanf("%d",&choice);
        getchar();

        if(1 == choice)
        {
            if(forbid_flag != 1)
            {
                /* printf("请输入消息:\n");
                memset(sendline, 0, sizeof(sendline));
                
                scanf("%s",sendline);
                //fgets(sendline,100,stdin);
                //群聊
                strcpy(msg_text.chat_mode,"stoa");
                strcpy(msg_text.msg,sendline);
                send(sockfd,&msg_text,sizeof(msg_text),0); 
                usleep(2); */
           
           

                strcpy(msg_text.chat_mode,"stoa");
                send(sockfd,&msg_text,sizeof(msg_text),0); 
                usleep(2);


                printf("请输入消息:\n");
                memset(message, 0, sizeof(message));
                scanf("%s", message);
            
                strcpy(msg_text.msg,message);

                memset(&dest_addr, 0, sizeof(struct sockaddr_in));
                dest_addr.sin_family = AF_INET;                     //指定发送目标的端口和ip地址 填写目标ip和端口
                dest_addr.sin_port = htons(5555);
                dest_addr.sin_addr.s_addr = inet_addr("192.168.33.136");
                int len = sendto(udp_cfd, &msg_text, sizeof(msg_text), MSG_NOSIGNAL,
                    (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));
                //printf("len = %d\n",len);
                if(len == -1)
                {
                    perror("sendto error:");
                }
            

            }
            else
            {
                printf("你已经被禁言\n");
            }
        }
        else if(2 == choice)
        {
            if(forbid_flag != 1)
            {
                /* printf("请输入消息:\n");
                memset(sendline, 0, sizeof(sendline));
                scanf("%s",sendline);
                //私聊
                strcpy(msg_text.chat_mode,"stoo");
                printf("请输入对象ID:\n");
                scanf("%s",msg_text.id);  
                //将发送的消息存在结构体
                strcpy(msg_text.msg,sendline);
                send(sockfd,&msg_text,sizeof(msg_text),0); */ 

                strcpy(msg_text.chat_mode,"stoo");
                send(sockfd,&msg_text,sizeof(msg_text),0); 
                usleep(2);

                printf("请输入消息:\n");
                memset(message, 0, sizeof(message));
                scanf("%s", message);
                printf("请输入对象ID:\n");
                scanf("%s",msg_text.id);
                //strcpy(msg_text.chat_mode,"stoo");
                strcpy(msg_text.msg,message);

                memset(&dest_addr, 0, sizeof(struct sockaddr_in));
                dest_addr.sin_family = AF_INET;                     //指定发送目标的端口和ip地址 填写目标ip和端口
                dest_addr.sin_port = htons(5555);
                dest_addr.sin_addr.s_addr = inet_addr("192.168.33.136");
                int len = sendto(udp_cfd, &msg_text, sizeof(msg_text), MSG_NOSIGNAL,
                    (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));
                //printf("len = %d\n",len);
                if(len == -1)
                {
                    perror("sendto error:");
                }
                
                
                
                /* memset(message, 0, sizeof(message));
                int len = sizeof(struct sockaddr_in);
                int recv_len = recvfrom(udp_cfd, message, sizeof(message), MSG_NOSIGNAL, 
                                     (struct sockaddr *)&dest_addr, &len);  //保存“数据发送方”的ip和端口 并打印
                printf("recv_len = %d\n",recv_len);
                printf("mes = %s\n",message); */
            
            }
            else
            {
                printf("你已经被禁言\n");
            }
           
        }
        else if(3 == choice)
        {
            srand((unsigned)time(NULL));
            int id = rand() % 1000;
            sprintf(msg_text.id, "%d", id);
            printf("请记住你的id:%s\n",msg_text.id);
        
            //printf("你的id:\n");
            //scanf("%s",msg_text.id);
            printf("你的姓名:\n");
            scanf("%s",msg_text.name);
            printf("密码:\n");
            scanf("%s",msg_text.passwd);
            strcpy(msg_text.chat_mode,"register");
            send(sockfd,&msg_text,sizeof(msg_text),0); 
        }
        else if(4 == choice )
        {
            printf("你的id:\n");
            scanf("%s",msg_text.id);
            printf("密码:\n");
            scanf("%s",msg_text.passwd);
            msg_text.udp_port = udp_port;
            strcpy(msg_text.chat_mode,"user_login");
            send(sockfd,&msg_text,sizeof(msg_text),0); 
        }
        else if(5 == choice)
        {
            //查看在线用户
            strcpy(msg_text.chat_mode,"onlien_user");
            send(sockfd,&msg_text,sizeof(msg_text),0);
        }
        else if(6 == choice)
        {
            if(admin_flag == 1)
            {
                printf("你已经是管理员\n");
            }
            else
            {
                //msg_text.admin = 1;
                strcpy(msg_text.chat_mode,"administrator");
                send(sockfd,&msg_text,sizeof(msg_text),0);
                printf("等待服务器响应\n");
            } 
        }
        else if(7 == choice)
        {
            if(admin_flag == 1)
            {
                //msg_text.admin_flag = admin_flag;
                
                printf("需要禁言用户的id\n");
                scanf("%s",msg_text.id);
                strcpy(msg_text.chat_mode,"forbid_say");
                send(sockfd,&msg_text,sizeof(msg_text),0);
            }
            else
            {
                printf("你不是管理员or你未登录\n");
            }
            
        }
        else if(8 == choice)
        {
            if(admin_flag == 1)
            {
                printf("需要解禁言用户的id\n");
                scanf("%s",msg_text.id);
                strcpy(msg_text.chat_mode,"cancle_forbid_say");
                send(sockfd,&msg_text,sizeof(msg_text),0);
            }
            else
            {
                printf("你不是管理员or你未登录\n");
            }
            
        }
        else if(9 == choice)
        {
            if(admin_flag == 1)
            {
                printf("需要剔除用户的id\n");
                scanf("%s",msg_text.id);
                strcpy(msg_text.chat_mode,"Kick_user");
                send(sockfd,&msg_text,sizeof(msg_text),0);
            }
            else
            {
                printf("你不是管理员or你未登录\n");
            }
        }
        else  if(10 == choice)
        {
            if(admin_flag == 0)
            {
                printf("你还不是管理员or你未登录账号\n");
            }
            else
            {
                strcpy(msg_text.chat_mode,"revoke");
                send(sockfd,&msg_text,sizeof(msg_text),0);
                printf("等待服务器响应");
            } 
           
        }
        else if(11 == choice)
        {
            strcpy(msg_text.chat_mode,"logoff");
            printf("请输入你的id:\n");
            scanf("%s",msg_text.id);
            printf("请输入密码:\n");
            scanf("%s",msg_text.passwd);
            send(sockfd,&msg_text,sizeof(msg_text),0);
        }
        else if(12 == choice)
        {
            //文件传输
            file_from(sockfd);
            //sleep(20);
        }
        else if(13 == choice)
        {
            printf("正在前往服务器\n");
            printf("查看本地记录\n");
            printf("请输入聊天对象的id\n");
            scanf("%s",msg_text.id);

            strcpy(msg_text.chat_mode,"chatrecord");
            send(sockfd,&msg_text,sizeof(msg_text),0);
        }
        else
        {
            printf("选择失误，重新做出选择!\n");
            //exit(0);
        }

        //避免选择冲突
        sleep(1);
    }

}

void file_from(int sockfd)
{

    char sendline[100];
    struct msg_node msg_text;
    
    int from_fd;
    int bytes_read;
    char *from_ptr;
    char filename_path[1024];
    strcpy(msg_text.chat_mode,"FTS");

    printf("请输入传输对象的id:\n");
    scanf("%s",msg_text.id);
    

    printf("请输入文件路径\n");
    scanf("%s",filename_path);

    if((from_fd = open(filename_path,O_RDONLY)) == -1)
    {
        perror("open error!\n");
        printf("没发现此文件\n");
        exit(1);
    }

    //from_ptr = msg_text.buffer;

    while(1)
    {
        //************
      
        memset(msg_text.buffer,0,sizeof(msg_text.buffer));
        //printf("buffer = %s\n",msg_text.buffer);
        bytes_read = read(from_fd,msg_text.buffer,1024);

        if((bytes_read == -1))
        {
            perror("read error!\n");
            exit(1);
        }
        if(bytes_read == 0)
        {
            break;
        }

        send(sockfd,&msg_text,sizeof(msg_text),0);
        sleep(3);
    }

    close(from_fd);


    /* bytes_read = read(from_fd,from_ptr,1024);

    if((bytes_read == -1))
    {
        perror("read error!\n");
        exit(1);
    }

    send(sockfd,&msg_text,sizeof(msg_text),0); */
}

void file_recv(char buffer[])
{
    int to_fd;
    char filename_path[1024];
    char * to_ptr;
    int bytes_write;

    //printf("buffer = %s\n",buffer);

    //printf("请先输入选择\n");
    //printf("请输入保存的文件路径\n");
    //scanf("%s",filename_path);
    //printf("file_path = %s\n",filename_path);
    
    //if((to_fd = open("a.txt",O_WRONLY | O_CREAT | O_APPEND, 0655)) == -1)
    if((to_fd = open("a.c",O_APPEND | O_CREAT | O_WRONLY  , 0655)) == -1)
    {
        perror("open error!\n");
        exit(1);
    }

    //lseek(to_fd,0,SEEK_END);

    //printf("*****\n");

    to_ptr = buffer;

    bytes_write = write(to_fd,to_ptr,strlen(buffer));

    if((bytes_write == -1))
    {
        perror("write error!\n");
        exit(1);
    }

    close(to_fd);

}

