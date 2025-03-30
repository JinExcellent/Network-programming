#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>


#define NAME_SIZE 20
#define BUF_SIZE 50

char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];



void * send_msg(void * arg);
void * recv_msg(void * arg);

int main(int argc, char * argv[]){
    struct sockaddr_in serv_addr;
    int sock_c;

    if(argc < 4){
        printf("Usage: %s <IP> <port> <name>\n", argv[0]);
        exit(1);
    }
    
    sprintf(name, "[%s]:", argv[3]);
    sock_c = socket(AF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if(connect(sock_c, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
        perror("connect error\n");
        exit(1);
    }
    
    pthread_t send_thread, recv_thread;
    void *thread_return;

    pthread_create(&send_thread, NULL, send_msg, (void *)&sock_c);
    pthread_create(&recv_thread, NULL, recv_msg, (void *)&sock_c);
    pthread_join(send_thread, &thread_return);
    pthread_join(recv_thread, &thread_return);
    close(sock_c);
    
    return 0;
}


void * send_msg(void * arg){
    int sock = *((int *)arg);
    char name_msg[NAME_SIZE + BUF_SIZE];
    
    while (1) {
        fgets(msg, BUF_SIZE, stdin);
        if(!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")){
            close(sock);
            exit(0);
        }
        sprintf(name_msg, "%s %s", name, msg);
        write(sock, name_msg, strlen(name_msg));
    }
}


void * recv_msg(void * arg){
    int sock = *((int *)arg);
    char name_msg[NAME_SIZE + BUF_SIZE];
    int str_len;

    while (1) {
        str_len = read(sock,name_msg, NAME_SIZE + BUF_SIZE - 1);
        if(str_len == -1){
            return (void *)-1;
        }
        name_msg[str_len] = 0;
        fputs(name_msg, stdout);
    }

    return NULL;
}


















