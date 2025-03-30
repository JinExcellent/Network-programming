#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/select.h>


#define BUFSIZE 50

int main(int argc, char *argv[]){
    struct sockaddr_in sevr_addr, clin_addr;
    int sock_l, sock_c;
    socklen_t clin_addr_sz;       //返回客户端套接字结构的大小

    if(argc < 2){
        printf("Usage: %s <port>\n", argv[0]);
        return 0;
    }
    sock_l = socket(AF_INET, SOCK_STREAM, 0);

    memset(&sevr_addr, 0, sizeof(sevr_addr));
    sevr_addr.sin_family = AF_INET;
    sevr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sevr_addr.sin_port = htons(atoi(argv[1]));

    if(bind(sock_l, (struct sockaddr *)&sevr_addr, sizeof(sevr_addr))){
        perror("bind error\n.");
        exit(1);
    }
    if(listen(sock_l, 5) == -1){
        perror("listen error");
        exit(1);
    }
    /*注意： 要明白这里为何要以“读信号“来监听“监听套接字”
     *
     *监听套接字（通过 socket() 创建 + listen() 启动监听）的核心功能
     *是接受新连接，其工作流程如下：
        1.当客户端发起连接（connect()）时，监听套接字会检测到“连接请求到达”。

        2.此时，监听套接字变为可读状态，但不会直接接收数据，而是需要通过 accept() 提取新连接。
     *
     */

    fd_set reads, cpy_reads;
    FD_ZERO(&reads);
    FD_SET(sock_l, &reads);
    
    int fd_max = sock_l;     //保存最大的文件描述符
    int fd_num;
    struct timeval timeout;     //设置超时    
    int str_len;
    char buf[BUFSIZE];

    while(1){
        cpy_reads = reads;      
        //每次循环初始化，监听所有指定的描述符，如果不进行复制备份，
        //在监听过程中，有些被指定监听的描述符没有就绪会被select函数丢弃。
        //如果是持续监听的话，那么下一次的监听就会缺失这个描述符
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;

        if((fd_num = select(fd_max+1, &cpy_reads, 0, 0, &timeout)) == -1){
            break;
        }
        if(fd_num == 0)
            continue;

        for(int i = 0; i < fd_max + 1; i++){
            if(FD_ISSET(i, &cpy_reads)){
                if(i == sock_l){        //监听套接字部分
                    clin_addr_sz = sizeof(clin_addr);
                    if((sock_c = accept(sock_l, (struct sockaddr *)&clin_addr, &clin_addr_sz)) == -1){
                        perror("accept error\n");
                        continue;
                    }
                    FD_SET(sock_c, &reads);
                    if(fd_max < sock_c)
                        fd_max = sock_c;
                    printf("connected client: %d\n", sock_c);
                }else{                  //数据传输套接字部分
                    str_len = read(i, buf, BUFSIZE);
                    if(str_len == 0){
                        FD_CLR(i, &reads);
                        close(i);
                        printf("closed client: %d\n", i);
                    }else {
                        write(i, buf, str_len);
                    }
                }
            }
        }
    }
    close(sock_l);
    return 0;
}




























