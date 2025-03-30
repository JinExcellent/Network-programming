#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>

#define EPOLL_SIZE 50
#define BUFSIZE 4


void SetNonblocking_mode(int);

int main(int argc, char *argv[]){
    
    struct sockaddr_in serv_addr , clin_addr;
    socklen_t clin_addr_sz;
    int sock_l, sock_c;

    if(argc < 2){
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    
    sock_l = socket(AF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if(bind(sock_l, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1){
        perror("bind error\n");
        exit(1);
    }
    
    if(listen(sock_l, 5) == -1){
        perror("listen error\n");
        exit(1);
    }
    
    struct epoll_event *ptr_events;
    struct epoll_event event;   //用于设定描述符结构的temp变量
    int epfd;
    int ep_cnt;     //记录发生的事件数

    epfd = epoll_create(EPOLL_SIZE);
    //用于epoll_wait函数，记录触发事件的结构体，而且该函数的第二个参数所接受的参数需要动态分配
    ptr_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE); 

    SetNonblocking_mode(sock_l);   /***          ***/

    event.events = EPOLLIN;
    event.data.fd = sock_l;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sock_l, &event);

    int str_len;        //记录读数据的字节数
    char buf[BUFSIZE];

    while (1) {
        ep_cnt = epoll_wait(epfd, ptr_events, EPOLL_SIZE, -1);
        if(ep_cnt == -1){
            perror("epoll wait error\n");
            exit(1);
        }
        /*
         *这句主要用于直观的感受水平触发模式：(这是epoll的默认模式。同样，select函数也是水平触发模式)

         *可以更改变量BUFSIZE，使其每次读操作的字节数少于缓冲区的字节数
         *这里我做的实验时设置为4，在客户端输入16个字符后，除去建立连接显示的一次“epoll_wait”，还会再显示五次。
         *为何我输入了16个字符（每次读取4个字符）会显示五次呢？ 
         *答：因为终端输入的是原始字节流（不想c中，字符串用‘\0’结尾 ）到输入缓冲区，
         *    在输入完字符后会按下回车，缓冲区的数据被发送到对端的接受缓冲区（这其中包含\n换行符）
         *
         *例如：输入16个aaaa aaaa aaaa aaaa(这里的空格不在输入序列中，只是为了识别)
         *      那么，服务器端读取的方式就为：
         *      1.aaaa  2.aaaa   3.aaaa  4.aaaa 5.\n    (可以重新只输入15个字符来验证这个换行符的存在)
         * **/
        puts("epoll_wait");
        for(int i = 0; i < ep_cnt; i++){
            if(ptr_events[i].data.fd == sock_l){
                clin_addr_sz = sizeof(clin_addr);
                sock_c = accept(sock_l, (struct sockaddr *)&clin_addr, &clin_addr_sz);
                SetNonblocking_mode(sock_c);  /******          *****/

                event.events = EPOLLIN | EPOLLET;  /*******   *******/
                event.data.fd = sock_c;
                epoll_ctl(epfd, EPOLL_CTL_ADD, sock_c, &event);
                printf("connected client: %d\n", sock_c);
            }else{
                while(1) {
                    str_len = read(ptr_events[i].data.fd, buf, BUFSIZE);
                    if(str_len == 0){
                        epoll_ctl(epfd, EPOLL_CTL_DEL, ptr_events[i].data.fd, NULL);
                        close(ptr_events[i].data.fd);
                        printf("closed client: %d\n", ptr_events[i].data.fd);
                        break;
                    }else if(str_len < 0){
                        if(errno == EAGAIN)
                            break;
                    }else{
                        write(ptr_events[i].data.fd, buf, str_len);
                    }
                }
            }
        }
        
    }
    close(sock_l);
    close(epfd);
    return 0;
}



void SetNonblocking_mode(int fd){
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}



























