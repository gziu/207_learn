#include "server.h"

int admin_flag;

void create_link(Onlien **head)
{
    create_node(head);
    (*head)->next = NULL;
}

void create_node(Onlien ** new_node)
{
    *new_node = (Onlien *)malloc(sizeof(Onlien));
    is_malloc_ok(*new_node);
}

void is_malloc_ok(Onlien *new_node)
{
    if(NULL == new_node)
    {
        printf("malloc error!\n");
        exit(-1);
    }
}

void insert_node_head(Onlien *head, Onlien * new_node)
{
    new_node->next = head->next;
    head->next = new_node;
}

void release_link(Onlien **head)
{
    Onlien *p = NULL;
    p = *head;

    while(*head != NULL)
    {
        *head = (*head)->next;
        free(p);
        p = *head;
    }
}

//客户端下线将cfd从链表中删除
void delete_node(struct thread_node *cfd_node)  //cfd head
{
    Onlien *p1 = NULL;
    Onlien *p2 = NULL;
    Onlien *head = NULL;  //id cfd next
    int cfd;

    head = cfd_node->head;
    cfd = cfd_node->cfd;

    p1 = head->next;
    p2 = head;

    if(p1 == NULL)
    {
        printf("like is empty!\n");
    }
    else
    {
        //cfd判断下线
        while(p1 != NULL && p1->cfd != cfd)
        {
            p2 = p1;
            p1 = p1->next;
        }

        if(p1 == NULL)
        {
            printf("no such client!\n");
        }
        else
        {
            p2->next = p1->next;
            free(p1);
        }
    }
}

void *chat_func(void *arg)
{   
    struct thread_node c_node;

    c_node = *((struct thread_node *)arg);  //cfd head pdb id

    while(1)
    {
        //聊天
        msg_send_recv(&c_node);
    }

    //关闭当前通信套接口
    close(c_node.cfd);
    //pthread_exit(NULL);
    return NULL;
}



int inquire_port(struct thread_node * cfd_node,char *id)
{
    Onlien *head = NULL;
    head = cfd_node->head;
    Onlien *p = NULL;
    p = head->next;

    if(p == NULL)
    {
        //printf("无用户在线\n");
        return 0;
    }

    while(p != NULL && strcmp(p->id,id) !=0 )
    {
        p = p->next;
    }

    if(p == NULL)
    {
        //printf("该id不在线\n");
        return 0;
    }
    else
    {
        //已经登录
        return p->udp_port;
    }




}



//收发消息
void msg_send_recv(struct thread_node * cfd_node) //id cfd head pdb
{
    int r_len;
    int msg_len;
    int i;
    char id[100];

    struct sockaddr_in dest_addr;

    struct msg_node msg_text;  //msg mode cfd id name passwd

    r_len = recv(cfd_node->cfd,&msg_text,sizeof(msg_text),0);

    if(0 == r_len || -1 == r_len)
    {
        printf("client is offline!\n");
        //下线节点删除  链接套接口 和头结点
        delete_node(cfd_node);
        pthread_exit(NULL);
    }
    //私聊
    else if(strcmp(msg_text.chat_mode,"stoo") == 0)
    {

        if (-1 == inspect_own_onlien(cfd_node))
        {
            //cfd
            char arr[100] = {"你未在线，不能发消息"};
            send(cfd_node->cfd,arr,strlen(arr),0);
            sleep(3);
        }
        else 
        {
            memset(&msg_text, 0, sizeof(msg_text));
            memset(&dest_addr, 0, sizeof(struct sockaddr_in));
            int len = sizeof(struct sockaddr_in);
            recvfrom(cfd_node->udp_cfd, &msg_text, sizeof(msg_text), MSG_NOSIGNAL, 
                                (struct sockaddr *)&dest_addr, &len);  //保存“数据发送方”的ip和端口 并打印

            printf("recv ip = %s\n", inet_ntoa(dest_addr.sin_addr));    //打印数据发送方的ip和端口
            printf("recv port = %d\n", ntohs(dest_addr.sin_port));
            printf("msg = %s\n",msg_text.msg);

            int port = inquire_port(cfd_node,msg_text.id);

            if(port != 0)
            {
                dest_addr.sin_port = htons(port);


                visit_link_one(cfd_node,&msg_text,&dest_addr);
                //printf("**********\n");

                int sed_len = sendto(cfd_node->udp_cfd, msg_text.msg, strlen(msg_text.msg), MSG_NOSIGNAL,     //填写目标ip和端口dest_addr已经保存目标ip与端口号
                                        (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));

                printf("sed_len = %d\n",sed_len);


                if(sed_len == -1)
                {
                    perror("send error");
                }

            }
            else
            {
                char arr[100] = {"该用户不在线"};
                send(cfd_node->cfd,arr,strlen(arr),0);
            }
             

            //int sed_len = sendto(cfd_node->udp_cfd, msg_text.msg, strlen(msg_text.msg), MSG_NOSIGNAL,     //填写目标ip和端口dest_addr已经保存目标ip与端口号
                                    //(struct sockaddr *)&dest_addr, sizeof(struct sockaddr));

            //printf("sed_len = %d\n",sed_len);

            //记录消息导数据库 ，查看对方是否在线
            

            
        }

        

        /* if (-1 == inspect_own_onlien(cfd_node))
        {
            //cfd
            char arr[100] = {"你未在线，不能发消息"};
            send(cfd_node->cfd,arr,strlen(arr),0);
        }
        else
        {
            //将头指针  聊天对象  消息结构体传出
            visit_link_one(cfd_node,&msg_text);
        } */
        
    }
    //群聊
    else if(strcmp(msg_text.chat_mode,"stoa") == 0)
    {
        if (-1 == inspect_own_onlien(cfd_node))
        {
            char arr[100] = {"你未在线，不能发消息"};
            send(cfd_node->cfd,arr,strlen(arr),0);
        }
        else
        {

            memset(&msg_text, 0, sizeof(msg_text));
            memset(&dest_addr, 0, sizeof(struct sockaddr_in));
            int len = sizeof(struct sockaddr_in);
            recvfrom(cfd_node->udp_cfd, &msg_text, sizeof(msg_text), MSG_NOSIGNAL, 
                                (struct sockaddr *)&dest_addr, &len);  //保存“数据发送方”的ip和端口 并打印

            printf("recv ip = %s\n", inet_ntoa(dest_addr.sin_addr));    //打印数据发送方的ip和端口
            printf("recv port = %d\n", ntohs(dest_addr.sin_port));
            printf("msg = %s\n",msg_text.msg);
            //内容 头指针
            visit_link_all(cfd_node,msg_text.msg,dest_addr);
        }
    }
    else
    {
        
        
        //注册
        if(strcmp(msg_text.chat_mode,"register") == 0)
        {
            register_id(cfd_node,&msg_text);
        }
        //登录
        else if(strcmp(msg_text.chat_mode,"user_login") == 0)
        {
            user_login_id(cfd_node,&msg_text);
        }
        //查看在线用户
        else if(strcmp(msg_text.chat_mode,"onlien_user") == 0)
        {
            if (-1 == inspect_own_onlien(cfd_node))
            {
                //cfd
                char arr[100] = {"你未在线，不能查看在线用户，须先登录"};
                send(cfd_node->cfd,arr,strlen(arr),0);
            }
            else
            {
                //查看在线用户
                if(0 == looking_onlien_users(cfd_node))
                {
                    char arr[100] = {"无用户在线"};
                    send(cfd_node->cfd,arr,strlen(arr),0);
                }
                else
                {
                    //printf("显示成功");
                }
            }
        }
        //管理员申请 
        else if(strcmp(msg_text.chat_mode,"administrator") == 0)
        {
            if (-1 == inspect_own_onlien(cfd_node))
            {
                char arr[100] = {"你未在线，不可申请"};
                send(cfd_node->cfd,arr,strlen(arr),0);
                return ;
            }
            
            int i;
            printf("是否给管理员身份:\n");
            printf("1: 是 2: 否\n");
            scanf("%d",&i);
            if(i == 1)
            {
                char arr[100] = {"同意管理员身份申请"};
                send(cfd_node->cfd,arr,strlen(arr),0);
                //将管理员标志置位1  cfd查找
                daminister_user(cfd_node);
                //msg_text.admin = 1;
            }
            if(i == 2)
            {
                char arr[100] = {"不同意管理员身份申请"};
                send(cfd_node->cfd,arr,strlen(arr),0);
                //msg_text.admin = 2;     
            }
            //usleep(3);
            //msg_text.action = 6;
            //send(cfd_node->cfd,"admin",5,0);
            //usleep(3);
            //send(cfd_node->cfd,&msg_text,sizeof(msg_text),0);
        }
        //禁言
        else if(strcmp(msg_text.chat_mode,"forbid_say") == 0)
        {
            int ret = forbid_user_say(cfd_node,msg_text.id);
            if(0 == ret)
            {
                char arr[100] = {"该id用户不在线"};
                send(cfd_node->cfd,arr,strlen(arr),0);
            }
            else if(-1 == ret)
            {
                usleep(1);
                char arr[100] = {"禁言成功"};
                send(cfd_node->cfd,arr,strlen(arr),0);
            }
            else
            {
                char arr[100] = {"对方是管理员，不可禁言"};
                send(cfd_node->cfd,arr,strlen(arr),0);
            }
        }
        //解禁
        else if(strcmp(msg_text.chat_mode,"cancle_forbid_say") == 0)
        {
            int ret = cancle_forbid_chat(cfd_node,msg_text.id);
            if(0 == ret)
            {
                char arr[100] = {"该id用户不在线"};
                send(cfd_node->cfd,arr,strlen(arr),0);
            }
            else if( 1 == ret)
            {
                char arr[100] = {"解禁成功"};
                send(cfd_node->cfd,arr,strlen(arr),0);
            }
            else
            {
                char arr[100] = {"该用户不需要解禁"};
                send(cfd_node->cfd,arr,strlen(arr),0);
            }
        }
        //踢人
        else if(strcmp(msg_text.chat_mode,"Kick_user") == 0)
        {
            kick_user(cfd_node,msg_text.id);
            
        }
        //注销
        else if(strcmp(msg_text.chat_mode,"logoff") == 0)
        {
            
            logoff_account(cfd_node,&msg_text);
            
        }
        //撤销管理员
        else if(strcmp(msg_text.chat_mode,"revoke") == 0)
        {
            if (-1 == inspect_own_onlien(cfd_node))
            {
                char arr[100] = {"你未登录账号"};
                send(cfd_node->cfd,arr,strlen(arr),0);
                return ;
            }

            int i;
            printf("是否撤销管理员身份:\n");
            printf("1: 是 2: 否\n");
            scanf("%d",&i);
            if(i == 1)
            {
                char arr[100] = {"撤销管理员身份成功"};
                send(cfd_node->cfd,arr,strlen(arr),0);
                //将管理员标志置位0  cfd查找
                revoke_daminister_user(cfd_node);
            }
            if(i == 2)
            {
                char arr[100] = {"撤销管理员身份失败"};
                send(cfd_node->cfd,arr,strlen(arr),0);    
            }
        }
        else if(strcmp(msg_text.chat_mode,"FTS") == 0)
        {
            //接收文件
            file_recv(cfd_node,&msg_text);
        }
         else if(strcmp(msg_text.chat_mode,"chatrecord") == 0)
        {
            //查询聊天记录
            inquire_chattable(cfd_node->pdb,cfd_node->cfd,&msg_text,cfd_node);
        }
        else
        {
            //printf("chat mode error!\n");
            char arr[100] = {"选择错误"};
            send(cfd_node->cfd,arr,strlen(arr),0);
        }
    }
}

void file_recv(struct thread_node *cfd_node,struct msg_node *data)
{
    if (-1 == inspect_own_onlien(cfd_node))
    {
        char arr[100] = {"你未在线，不能发文件"};
        send(cfd_node->cfd,arr,strlen(arr),0);
        return ;
    }

    int len;

    Onlien *p = NULL;

    p = cfd_node->head->next;

    //寻找在线用户链表中的cfd与私聊的cfd是否一致
    while(p != NULL && strcmp(p->id, data->id) != 0)
    {
        //printf("****%s\n",p->id);
        p = p->next;
    }

    //判断是否在线
    if(p == NULL)
    {
        //printf("client is not online!\n");
        char arr[100] = {"该用户不在线"};
        send(cfd_node->cfd,arr,strlen(arr),0);
    }
    else
    {
        printf("正在接收中......\n");

        char arr[100] = {"发送文件中"};
        send(p->cfd,arr,strlen(arr),0);
        usleep(1);
        //向客户端发送文件
        send(p->cfd,data->buffer,strlen(data->buffer),0);


    }


}

//注销账户
void logoff_account(struct thread_node *cfd_node,struct msg_node *data)
{
    if (-1 != inspect_online_user(cfd_node,data))
    {
        //id
        char arr[100] = {"注销失败，请先登录该id"};
        send(cfd_node->cfd,arr,strlen(arr),0);
        return ;
    }

    Onlien *p1 = NULL;
    Onlien *head = NULL;  
    head = cfd_node->head;
    p1 = head->next;

    if(p1 == NULL)
    {
        char arr[100] = {"注销失败，无人在线"};
        send(cfd_node->cfd,arr,strlen(arr),0);
    }
    else
    {
        //cfd判断下线
        while(p1 != NULL && strcmp(p1->id,data->id) != 0)
        {
            p1 = p1->next;
        }

        if(p1 == NULL)
        {
            char arr[100] = {"未查询此用户"};
            send(cfd_node->cfd,arr,strlen(arr),0);
        }
        else if(p1->cfd != cfd_node->cfd)
        {
            char arr[100] = {"注销失败，请输入你的id"};
            send(cfd_node->cfd,arr,strlen(arr),0);
        }
        else
        {    
            //char arr[100] = {"注销成功"};
            //send(cfd_node->cfd,arr,strlen(arr),0);

            //从在线链表移除
            if(1 == delete_onlien_user(cfd_node,data))
            {
                //从数据库删除
                if(0 == delete_record(cfd_node->pdb,data->id))
                {
                    char arr[100] = {"注销成功"};
                    send(cfd_node->cfd,arr,strlen(arr),0);
                }
                else
                {
                    char arr[100] = {"注销失败"};
                    send(cfd_node->cfd,arr,strlen(arr),0);
                }
            }   
            else
            {
                //printf("注销失败\n");
                //char arr[100] = {"注销失败"};
                //send(cfd_node->cfd,arr,strlen(arr),0);
            }
            
        }
    } 
}

//从数据库中删除注册信息
int delete_record(sqlite3 *pdb,char *id)
{
    char str[100];
    char *sql = str;
    char *errmsg = NULL;
    

    sprintf(sql, "delete from mytable where id = '%s';",id);

    if(sqlite3_exec(pdb,sql,NULL,NULL,&errmsg) != SQLITE_OK)
    {
        printf("delete record fail! %s \n",errmsg);
        sqlite3_close(pdb);
        return -1;
    }

    return 0;
}

//注销从在线链表移除
int delete_onlien_user(struct thread_node *cfd_node,struct msg_node *data)
{
    Onlien *p1 = NULL;
    Onlien *p2 = NULL;
    Onlien *head = NULL; 

    head = cfd_node->head;

    p1 = head->next;
    p2 = head;

    if(p1 == NULL)
    {
        char arr[100] = {"无在线用户"};
        send(cfd_node->cfd,arr,strlen(arr),0);
        return 0;
    }
    else
    {
        //cfd判断下线
        while(p1 != NULL && strcmp(p1->id,data->id) != 0)
        {
            p2 = p1;
            p1 = p1->next;
        }

        if(p1 == NULL)
        {
            char arr[100] = {"未查询此用户"};
            send(cfd_node->cfd,arr,strlen(arr),0);
            return 0;
        }
        else
        {
            if( -1 == find_record_passwd(cfd_node->pdb,data->passwd,data->id))
            {
                char arr[100] = {"密码错误,注销失败"};
                send(cfd_node->cfd,arr,strlen(arr),0); 
                return 0;
            }
            else
            {
                p2->next = p1->next;
                //移除在线用户
                free(p1);
                //char arr[100] = {"剔除成功"};
                //send(cfd_node->cfd,arr,strlen(arr),0);
                return 1;
            } 
        }
    }
}

//撤销管理员身份处理
void revoke_daminister_user(struct thread_node *cfd_node)
{
    Onlien *p1 = NULL;
    Onlien *p2 = NULL;
    Onlien *head = NULL;  

    head = cfd_node->head;

    p1 = head->next;

    if(p1 == NULL)
    {
        return ;
    }
    else
    {
        //cfd判断下线
        while(p1 != NULL && p1->cfd != cfd_node->cfd)
        {
            p1 = p1->next;
        }

        if(p1 == NULL)
        {
            return ;
        }
        else
        {
            p1->admin_flag = 0;
            return;
        }
    }
}

//管理员身份同意处理
void daminister_user(struct thread_node *cfd_node)
{
    Onlien *p1 = NULL;
    Onlien *p2 = NULL;
    Onlien *head = NULL;  

    head = cfd_node->head;

    p1 = head->next;

    if(p1 == NULL)
    {
        return ;
    }
    else
    {
        //cfd判断下线
        while(p1 != NULL && p1->cfd != cfd_node->cfd)
        {
            p1 = p1->next;
        }

        if(p1 == NULL)
        {
            return ;
        }
        else
        {
            p1->admin_flag = 1;
            return;
        }
    }
}

//踢人
void kick_user(struct thread_node *cfd_node,char * ID)  //cfd head
{
    Onlien *p1 = NULL;
    Onlien *p2 = NULL;
    Onlien *head = NULL;  
    char *id = ID;

    head = cfd_node->head;

    p1 = head->next;
    p2 = head;

    if(p1 == NULL)
    {
        char arr[100] = {"该用户不在群组，踢人失败"};
        send(cfd_node->cfd,arr,strlen(arr),0);
    }
    else
    {
        //cfd判断下线
        while(p1 != NULL && strcmp(p1->id,id) != 0)
        {
            p2 = p1;
            p1 = p1->next;
        }

        if(p1 == NULL)
        {
            char arr[100] = {"未查询此用户"};
            send(cfd_node->cfd,arr,strlen(arr),0);
        }
        else
        {
            if(p1->admin_flag != 1)
            {
                p2->next = p1->next;
                //踢人
                free(p1);
                char arr[100] = {"剔除成功"};
                send(cfd_node->cfd,arr,strlen(arr),0);
                memset(arr,0,sizeof(arr));
                char *ar;
                ar = "你已经被管理员剔除";
                send(p1->cfd,ar,strlen(ar),0);

            }
            else
            {
                char arr[100] = {"对方是管理员，无权限剔除"};
                send(cfd_node->cfd,arr,strlen(arr),0);
            }
        }
    }
}

//解禁
int cancle_forbid_chat(struct thread_node *cfd_node,char *ID)
{
    Onlien *head = NULL;
    head = cfd_node->head;
    Onlien *p = NULL;
    p = head->next;

    if(p == NULL)
    {
        //printf("无用户在线\n");
        return 0;
    }

    while(p != NULL && strcmp(p->id,ID) !=0 )
    {
        p = p->next;
    }

    if(p == NULL)
    {
        //printf("该id不在线\n");
        return 0;
    }
    else
    {
        //保存禁言标志方便解禁
        if(p->forbid_flag == 1)
        {
            char arr[100] = {"你已经被管理员解禁"};
            p->forbid_flag = 0;
            send(p->cfd,arr,strlen(arr),0);
            return 1;
        }
        else
        {
            //printf("该用户不需要解禁，没有被禁言\n");
            return -1;
        }
    }
}

//禁言
int forbid_user_say(struct thread_node *cfd_node,char *ID)
{
    Onlien *head = NULL;
    head = cfd_node->head;
    Onlien *p = NULL;
    p = head->next;

    if(p == NULL)
    {
        //printf("无用户在线\n");
        return 0;
    }

    while(p != NULL && strcmp(p->id,ID) !=0 )
    {
        p = p->next;
    }

    if(p == NULL)
    {
        //printf("该id不在线\n");
        return 0;
    }
    else
    {
        if(p->admin_flag == 1)
        {
            //printf("对方是管理员，不可禁言");
            return 1;
        }
        else
        {
            //保存禁言标志方便解禁************
            p->forbid_flag = 1;
            char arr[100] = {"你已经被管理员禁言"};
            send(p->cfd,arr,strlen(arr),0);
            return -1;
        }
        
        
    }
}

//查看在线用户
int looking_onlien_users(struct thread_node *cfd_node)
{
    Onlien *head = NULL;
    head = cfd_node->head;
    Onlien *p = NULL;
    p = head->next;

    char ID[100];

    if(p == NULL)
    {
        //printf("无用户在线\n");
        return 0;
    }
    char arr[100] = {"在线用户如下 显示id:\n"};
    send(cfd_node->cfd,arr,strlen(arr),0);

    while(p != NULL )
    {
        memset(ID,0,sizeof(ID));
        strcpy(ID,p->id);
        send(cfd_node->cfd,ID,strlen(ID),0);
        p = p->next;
        usleep(2);
    }

    return 1;

}

//检查自己是否重复登录
int inspect_online_user(struct thread_node * cfd_node,struct msg_node *data)
{
    Onlien *head = NULL;
    head = cfd_node->head;
    Onlien *p = NULL;
    p = head->next;

    if(p == NULL)
    {
        //printf("无用户在线\n");
        return 0;
    }

    while(p != NULL && strcmp(p->id,data->id) !=0 )
    {
        p = p->next;
    }

    if(p == NULL)
    {
        //printf("该id不在线\n");
        return 0;
    }
    else
    {
        //已经登录
        return -1;
    }


}

//用户登录
void user_login_id(struct thread_node * cfd_node,struct msg_node *data)
{
    //检查自己是否登录
    if (-1 == inspect_online_user(cfd_node,data))
    {
        //id
        char arr[100] = {"你已经在线，无需登录"};
        send(cfd_node->cfd,arr,strlen(arr),0);
        return ;
    }

    Onlien *new_node;
    Onlien *head = cfd_node->head;

    data->cfd = cfd_node->cfd;

    if( -1 != find_record_id(cfd_node->pdb,data->id))
    {
        char arr[100] = {"用户id不存在，请重新登录"};
        send(data->cfd,arr,strlen(arr),0); 
        return;
    }
    else
    {
        if( -1 == find_record_passwd(cfd_node->pdb,data->passwd,data->id))
        {
            char arr[100] = {"密码错误，请重新登录"};
            send(data->cfd,arr,strlen(arr),0); 
            return;
        }
        else
        {
            char arr[100] = {"登录成功"};
            send(data->cfd,arr,strlen(arr),0); 
            usleep(1);

            char *vip;


            vip = inquire_vip(cfd_node->pdb,data->id);
            send(data->cfd,vip,strlen(vip),0); 

            //创建新节点
            create_node(&new_node);   //id name passwd cfd next
            //id cfd
            strcpy(new_node->id, data->id);
            new_node->cfd = data->cfd;
            new_node->forbid_flag = 0;
            new_node->admin_flag = 0;
            new_node->udp_cfd = cfd_node->udp_cfd;

            new_node->udp_port = data->udp_port;
            //printf("link udpcfd = %d\n",cfd_node->udp_cfd);
            printf("---udp_port--- = %d\n",data->udp_port);
            
            //头插
            insert_node_head(head,new_node);
        }
    }
}

char * inquire_vip(sqlite3 * pdb, char * id)
{

    char str[100];
    char *sql = str;
    char *errmsg = NULL;
    char **result;
    int row_count;
    int column_count;
    int flag;

    sprintf(sql, "select *from mytable where id = '%s';",id);

    if(SQLITE_OK != sqlite3_get_table(pdb,sql,&result,&row_count,&column_count,&errmsg))
    {
        printf("find record fail! %s \n",errmsg);
        sqlite3_free_table(result);
        sqlite3_close(pdb);
        exit(-1);
    }

    //printf("This is find record!\n");
    //printf("row = %d, column = %d\n",row_count,column_count);

    for(int i = column_count; i < column_count * (row_count + 1); i++)
    {
        if(i  == column_count * (row_count + 1) - 1 )
        {
            char * vip = result[i];
            sqlite3_free_table(result);
            return vip;
        }
    }
   
}


//检查密码
int find_record_passwd(sqlite3 *pdb,char *passwd,char *id)
{
    char str[100];
    char *sql = str;
    char *errmsg = NULL;
    char **result;
    int row_count;
    int column_count;
    int flag;

    sprintf(sql, "select *from mytable where id = '%s';",id);

    if(SQLITE_OK != sqlite3_get_table(pdb,sql,&result,&row_count,&column_count,&errmsg))
    {
        printf("find record fail! %s \n",errmsg);
        sqlite3_free_table(result);
        sqlite3_close(pdb);
        exit(-1);
    }

    //printf("This is find record!\n");
    //printf("row = %d, column = %d\n",row_count,column_count);

    for(int i = column_count; i < column_count * (row_count + 1); i++)
    {
        if(i  == column_count * (row_count + 1) - 2 )
        {
            if(strcmp(result[i],passwd) != 0)
            {
                flag = 1;
                break;
            }
            else
            {
                //密码正确
                flag = 0;
            }
        }
    }
    if(flag == 1)
    {
        //printf("密码错误\n"); 
        sqlite3_free_table(result);
        return -1;
    }
    else if(flag == 0)
    {
        //printf("密码正确\n");
        sqlite3_free_table(result);
        return 0;
    }
    //sqlite3_free_table(result);
    return 0;
}

//注册
void register_id(struct thread_node * cfd_node,struct msg_node *data)
{
    //Onlien *p = NULL;
    //p = cfd_node->head;
    //往数据库插入数据

    //记录cfd 传回注册成功
    data->cfd = cfd_node->cfd;
    //注册前查找是否被注册
    if( -1 == find_record_id(cfd_node->pdb,data->id))
    {
        char arr[100] = {"id已存在,请重新注册"};
        send(data->cfd,arr,strlen(arr),0); 
        return;
    }
    else
    {
        //插入注册数据
        strcpy(data->vip,"否");
        insert_record(cfd_node->pdb,data);
    }
}

//查找id是否注册
int find_record_id(sqlite3 *pdb,char *id)
{
    char str[100];
    char *sql = str;
    char *errmsg = NULL;
    char **result;
    int row_count;
    int column_count;
    int flag = 0;

    sql = "select *from mytable;";
    //sprintf(sql, "select *from mytable where id = '%s';",id);

    if(SQLITE_OK != sqlite3_get_table(pdb,sql,&result,&row_count,&column_count,&errmsg))
    {
        printf("find record fail! %s \n",errmsg);
        sqlite3_free_table(result);
        sqlite3_close(pdb);
        exit(-1);
    }

    printf("正在查询是否已经注册!\n");
    //printf("row = %d, column = %d\n",row_count,column_count);
    for(int i = column_count; i < column_count * (row_count + 1); i++)
    {
        if(i % column_count == 0)
        {
            if(strcmp(result[i],id) == 0)
            {
                //printf("id = %s\n",result[i]);
                flag = 1;
                break;
            }
        }
    }
    if(flag == 1)
    {
        //printf("已经被注册\n"); 
        sqlite3_free_table(result);
        return -1;
    }
    sqlite3_free_table(result);
}

//插入数据库数据
void insert_record(sqlite3 *pdb,struct msg_node *data)
{
    char str[100];
    char *sql = str;
    char *errmsg = NULL;

    //需要先分配空间，insert语句,传字符串单引号, 传整型不需要
    sprintf(sql,"insert into mytable (id,name,passwd,vip) values ('%s','%s',%s,'%s');"
                                            ,data->id,data->name,data->passwd,data->vip);

    if(SQLITE_OK != sqlite3_exec(pdb,sql,NULL,NULL,&errmsg))
    {
        printf("insert record fail! %s \n",errmsg);
        sqlite3_close(pdb);
        exit(-1);
    }

    char arr[100] = {"注册成功"};
    send(data->cfd,arr,strlen(arr),0);
}

//检查自己是否在线，可以发送消息
 int inspect_own_onlien(struct thread_node *cfd_node)
 {
    Onlien *head = NULL;
    head = cfd_node->head;
    Onlien *p = NULL;
    p = head->next;

    if(p == NULL)
    {
        //printf("无用户在线\n");
        return -1;
    }

    while(p != NULL && p->cfd != cfd_node->cfd)
    {
        p = p->next;
    }

    if(p == NULL)
    {
        //printf("自己不在线\n");
        return -1;
    }
    else
    {
        //printf("自己在线");
        return 0;
    }
 }

//群聊
void visit_link_all(struct thread_node * cfd_node,char msg[],struct sockaddr_in dest_addr)
{
    int msg_len;
    Onlien *p = NULL;
    Onlien * head = cfd_node->head;
    p = head->next;
    msg_len = strlen(msg);
    char * myid;
    int udp_cfd;
    udp_cfd = cfd_node->udp_cfd;

    while(p != NULL)
    {
        myid = search_ownid(cfd_node);
        if(myid != NULL)
        {
            insert_chattable(cfd_node->pdb,myid,p->id,msg);
            printf("记录成功\n");
        }

        char arr[100] = {"群聊"};
        send(p->cfd,arr,strlen(arr),0);
        usleep(2);

        dest_addr.sin_port = htons(p->udp_port);

        int sed_len = sendto(cfd_node->udp_cfd, msg, strlen(msg), MSG_NOSIGNAL,     //填写目标ip和端口dest_addr已经保存目标ip与端口号
                                (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));

        printf("sed_len = %d\n",sed_len);


        if(sed_len == -1)
        {
            perror("send error");
        }
        
        //发送给每个客户端
        //send(p->cfd,msg,msg_len,0);
        p = p->next;
    }
}



//私聊
void visit_link_one(struct thread_node * cfd_node,struct msg_node *msg_one,struct sockaddr_in *dest_addr)
{
    int len;

    Onlien *p = NULL;

    p = cfd_node->head->next;

    len = strlen(msg_one->msg);

    //寻找在线用户链表中的cfd与私聊的cfd是否一致
    while(p != NULL && strcmp(p->id, msg_one->id) != 0)
    {
        //printf("****%s\n",p->id);
        p = p->next;
    }

    //判断是否在线
    if(p == NULL)
    {
        //printf("id is not online!\n");
        char arr[100] = {"该用户不在线"};
        send(cfd_node->cfd,arr,strlen(arr),0);
    }
    else
    {   
        //printf("id is online!\n");
        char* myid = search_ownid(cfd_node);
        if(myid != NULL)
        {
            insert_chattable(cfd_node->pdb,myid,p->id,msg_one->msg);
            printf("记录成功\n");
        }
       
        char arr[100] = {"私聊"};
        send(p->cfd,arr,strlen(arr),0);
        usleep(2);

        printf("msg = %s\n",msg_one->msg);
        printf("p->udp = %d\n",p->udp_cfd);

       /*  int sed_len = sendto(cfd_node->udp_cfd, msg_one->msg, strlen(msg_one->msg), MSG_NOSIGNAL,     //填写目标ip和端口dest_addr已经保存目标ip与端口号
                            (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));

        printf("sed_len = %d\n",sed_len);


        if(sed_len == -1)
        {
            perror("send error");
        } */


        //send(p->cfd,msg_one->msg,len,0);
    }
}

//遍历数据库聊天记录
void inquire_chattable(sqlite3 *pdb,int cfd,struct msg_node *data,struct thread_node *cfd_node)
{
    char *id = data->id;

    char *myid = search_ownid(cfd_node);

    if(myid == NULL)
    {
        return;
    }
    printf("myid = %s\n",myid);
    printf("id = %s\n",id);



    char str[100];
    char *sql = str;
    char **result = NULL;
    int row;
    int column;
    char *errmsg = NULL;

    sql = "select *from chat_table;";

    if(SQLITE_OK == sqlite3_get_table(pdb,sql,&result,&row,&column,&errmsg))
    {
        printf("聊天内容如下!\n");
        printf("row = %d column = %d\n",row,column);

        char arr[100] = {"聊天记录如下:"};
        send(cfd,arr,strlen(arr),0);
        usleep(2);

        //for(int i = 0; i < 10; i++)
        for(int i = 0; i < (row + 1) * column; i++)
        {
            //int j = i;
             if(i < column)
            {
                send(cfd,result[i],strlen(result[i]),0);
                //i++;
                usleep(1);
            }
            else if(strcmp(result[i],myid) == 0 && strcmp(result[i+1],id) == 0)
            {
                //printf("******\n");
                for(int k = 0; k < 5; k++)
                {
                    send(cfd,result[i],strlen(result[i]),0);
                    i++;
                    usleep(1);
                }
                i--;
            } 

           /*  printf("%-25s",result[i]);
            if(i < 5)
            {
                printf("   ");
            }

            if((i + 1) % column == 0)
            {
                printf("\n");
            } */
            
        }


        char dd[100] = {"发送结束"};
        send(cfd,dd,strlen(dd),0);
    }
    sqlite3_free_table(result);
}

//保存记录
void insert_chattable(sqlite3 *pdb,char *fromid, char *toid,char *chatrecord)
{
    char str[1024];
    char *sql = str;
    char *errmsg = NULL;

    //需要先分配空间，insert语句,传字符串单引号, 传整型不需要
    sprintf(sql,"insert into chat_table (发送方,接收方,记录,日期,时间) values ('%s','%s','%s',date('now'),time('now','localtime'));"
                                            ,fromid,toid,chatrecord);

    if(SQLITE_OK != sqlite3_exec(pdb,sql,NULL,NULL,&errmsg))
    {
        printf("insert record fail! %s \n",errmsg);
        sqlite3_close(pdb);
        printf("记录失败\n");
        exit(-1);
    }
}

//搜查自己的id，保存聊天记录用
char* search_ownid(struct thread_node  *cfd_node)
{
    Onlien *head = NULL;
    head = cfd_node->head;
    Onlien *p = NULL;
    p = head->next;

    if(p == NULL)
    {
        //printf("无用户在线\n");
        return NULL;
    }

    while(p != NULL && p->cfd != cfd_node->cfd)
    {
        p = p->next;
    }

    if(p == NULL)
    {
        //printf("没查到自己id\n");
        return NULL;
    }
    else
    {
        //printf("自己在线");
        return p->id;
    }
}


//创建表
void creat_table(sqlite3 *pdb)
{
    char * sql = NULL;
    char *errmsg = NULL;
    
    //id name passed
    sql = "create table if not exists mytable (id text primary key,name text,passwd text, vip text);";

    if(sqlite3_exec(pdb,sql,NULL,NULL,&errmsg) != SQLITE_OK)
    {
        printf("create table fail! %s \n",errmsg);
        sqlite3_close(pdb);
        exit(-1);
    }
}

void creat_table_chat(sqlite3 *pdb)
{
    char * sql = NULL;
    char *errmsg = NULL;
    
    //id name passed
    sql = "create table if not exists chat_table (发送方 text,接收方 text,记录 text,日期 date,时间 time);";

    if(sqlite3_exec(pdb,sql,NULL,NULL,&errmsg) != SQLITE_OK)
    {
        printf("create table fail! %s \n",errmsg);
        sqlite3_close(pdb);
        exit(-1);
    }
}

//***************************************************
//线程池

void * threadpool_function(void *arg) //任务队列取数据 执行任务
{
    
    struct threadpool *pool = (struct threadpool *)arg;
    struct job *pjob = NULL;

    while (1)
    {
        //我访问时别人不能让问  访问任务队列10个一个一个来
        pthread_mutex_lock(&(pool->mutex));

        while(pool->queue_cur_num == 0)
        {
            //当目前任务队列没有任务  等待任务队列不为空的条件被置位(添加任务处成功过来唤醒)
            pthread_cond_wait(&(pool->queue_not_emtpy), &(pool->mutex));

            //线程要结束时 退出
            if (pool->pool_close == 1)
            {
                pthread_exit(NULL);
            }
        }

        pjob = pool->head;   //将对头任务拿出去处理
        pool->queue_cur_num--;  //任务数量减一个

        if (pool->queue_cur_num != pool->queue_max_num)
        {
            //如果任务不满   不满条件唤醒
            pthread_cond_broadcast(&(pool->queue_not_full));
        }
        
        if (pool->queue_cur_num == 0)
        {
            //当前任务队列没有任务
            pool->head = pool->tail = NULL;
            //当任务队列为空时 唤醒空条件，去销毁线程池
            pthread_cond_broadcast(&(pool->queue_empty));
        }
        else
        {
            pool->head = pjob->next; //处理完一个 队头向后移动一个
        }
        
        pthread_mutex_unlock(&(pool->mutex));

        (*(pjob->func))(pjob->arg);//让线程执行任务队列里的任务
        free(pjob);//执行完释放
        pjob = NULL;
    }
}

struct threadpool * threadpool_init(int thread_num, int queue_max_num)
{
    struct threadpool *pool = (struct threadpool *)malloc(sizeof(struct threadpool));
    // malloc

    pool->queue_max_num = queue_max_num;
    pool->queue_cur_num = 0;
    pool->pool_close = 0; //线程退出标志 0不退出
    pool->head = NULL;
    pool->tail = NULL;

    pthread_mutex_init(&(pool->mutex), NULL);
    pthread_cond_init(&(pool->queue_empty), NULL);
    pthread_cond_init(&(pool->queue_not_emtpy), NULL);
    pthread_cond_init(&(pool->queue_not_full), NULL);

    pool->thread_num = thread_num;
    pool->pthread_ids = (pthread_t *)malloc(sizeof(pthread_t) * thread_num);//乘 线程数量
    // malloc

    for (int i = 0; i < pool->thread_num; i++)
    {
        //创建线程
        pthread_create(&pool->pthread_ids[i], NULL, (void *)threadpool_function, (void *)pool);
    }

    return pool;
}

void threadpool_add_job(struct threadpool *pool, void *(*func)(void *), void *arg)
{
    pthread_mutex_lock(&(pool->mutex));
    //队列满
    while (pool->queue_cur_num == pool->queue_max_num)
    {
        //阻塞等待 不满条件发生
        //队列任务满 不得添加
        pthread_cond_wait(&pool->queue_not_full, &(pool->mutex));
    }
    
    //定义函数链表
    struct job *pjob = (struct job *)malloc(sizeof(struct job));
    //malloc
    
    pjob->func = func;
    pjob->arg = arg;
    pjob->next = NULL;
    
    // pjob->func(pjob->arg);
    if (pool->head == NULL)   //队列为空
    {
        pool->head = pool->tail = pjob;     //队头队尾指向链表
        //唤醒  告诉别人任务队列不为空
        pthread_cond_broadcast(&(pool->queue_not_emtpy));
    }
    else      //队尾向后移1个
    {
        pool->tail ->next = pjob;
        pool->tail = pjob;
    }

    pool->queue_cur_num++;  //队列任务加1
    pthread_mutex_unlock(&(pool->mutex));
}

void thread_destroy(struct threadpool *pool)
{
    pthread_mutex_lock(&(pool->mutex));

    while (pool->queue_cur_num != 0)
    {
        //等任务队列为空 才能销毁 阻塞等待 空条件
         pthread_cond_wait(&(pool->queue_empty),&(pool->mutex));
    }

    pthread_mutex_unlock(&(pool->mutex));

    //为空 唤醒不满条件  看有没有阻塞的线程
    pthread_cond_broadcast(&(pool->queue_not_full));
    //pthread cond broadcast(&( pool->queue_not_empty));

    //任务队列为空时  置为1 告诉其他线程要退出了
    pool->pool_close = 1;

    //回收线程资源
    for (int i = 0; i < pool->thread_num; i++)
    {
        //每次都唤醒  不唤醒 阻塞无法执行 线程释放
        pthread_cond_broadcast(&(pool->queue_not_emtpy));
        // pthread_cancel(pool->pthread_ids[i]); //有系统调用，才能销毁掉；有bug
        printf("thread exit!\n");
        pthread_join(pool->pthread_ids[i], NULL);
    }

    pthread_mutex_destroy(&(pool->mutex));
    pthread_cond_destroy(&(pool->queue_empty));
    pthread_cond_destroy(&(pool->queue_not_emtpy));
    pthread_cond_destroy(&(pool->queue_not_full));

    free(pool->pthread_ids);

    //再次，释放每个节点
    struct job *temp;
    while(pool->head != NULL)
    {
        temp = pool->head;
        pool->head = temp->next;
        free(temp);
    }

    free(pool);

    printf("destroy finish!\n");
}






