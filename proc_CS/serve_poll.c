#include <stdio.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unp.h>


#define FOPEN_MAX 16
#define BUF_SIZE 30


/**
 *与select最大的不同在于，poll不需要使用宏来进行位操作，而是使用使用一个结构数组，在使用上更加的方便
 *而且不要在每次循环中传入描述符集个内核，从而优化了程序
 *
 *
 *
 *
 * **/


int main(int argc, char *argv[]){

    struct sockaddr_in serv_addr, clin_addr;
    socklen_t clin_addr_sz;
    int sock_l, sock_c;
    char buf[BUF_SIZE];

    if(argc < 2){
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    sock_l = socket(AF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if(bind(sock_l, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
        perror("bind error.\n");
        exit(1);
    }
    
    listen(sock_l, 5);
    
    struct pollfd clients[FOPEN_MAX];
    int maxi;       //记录clinets数组中活跃的描述符数
    int nready;

    clients[0].fd = sock_l;
    clients[0].events = POLLRDNORM;

    for(int i = 1; i < FOPEN_MAX; i++)
        clients[i].fd = -1;

    maxi = 0;

    while(1){
       nready = poll(clients, maxi + 1, INFTIM);
        
       if(clients[0].revents & POLLRDNORM){
            clin_addr_sz = sizeof(clin_addr);
            sock_c = accept(sock_l, (struct sockaddr *)&clin_addr, &clin_addr_sz);
            printf("new clinet: %s\n ", inet_ntoa(clin_addr.sin_addr));
            
            int index;
            for(index = 1; index < FOPEN_MAX; index++){
                if(clients[index].fd < 0){
                    clients[index].fd = sock_c;
                    break;
                }
            }
            if(index == FOPEN_MAX){
                puts("too many clients, connect refuse.");
                continue;
            }
            clients[index].events = POLLRDNORM;
            if(index > maxi)
                maxi = index;
            if(--nready <= 0)
                continue;

       }

       int sockfd;
       int str_len;
       for(int i = 1; i <= maxi; i++){
            if((sockfd = clients[i].fd) < 0)
                continue;
            if(clients[i].revents & (POLLERR | POLLRDNORM)){
                if((str_len = read(sockfd, buf, BUF_SIZE)) < 0){
                    if(errno == ECONNRESET){
                        printf("clinet[%d] aborted connection\n", i);
                        close(sockfd);
                        clients[i].fd = -1;
                    }else
                        perror("read error\n");
                }else if(str_len == 0){
                    printf("clinet[%d] closed connection\n", i);
                }else
                    write(sockfd, buf, str_len);

                if(--nready < 0)
                    break;
            }   
       }
    }

    return 0;
}































