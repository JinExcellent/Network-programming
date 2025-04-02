#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>


#define BUF_SIZE 100
#define MAX_CLIN 256


void *handle_clin(void * arg);
void send_mas(char * msg, int len);


pthread_mutex_t mutx;
int clin_count = 0;
int clin_socks[MAX_CLIN];

int main(int argc, char *argv[]){
    struct sockaddr_in serv_addr, clin_addr;
    int sock_l, sock_c;
    socklen_t clin_addr_sz;

    if(argc < 2){
        printf("Usage: %s <port>\n", argv[1]);
        exit(1);
    }
    sock_l = socket(AF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if(bind(sock_l, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
        perror("bind error");
        exit(1);
    }
    listen(sock_l, 5);

    pthread_mutex_init(&mutx, NULL);
    pthread_t t_id;

    while (1) {
        clin_addr_sz = sizeof(clin_addr);
        if((sock_c = accept(sock_l, (struct sockaddr *)&clin_addr, &clin_addr_sz)) == -1){
            perror("accept error");
            continue;
        }

        pthread_mutex_lock(&mutx);
        clin_socks[clin_count++] = sock_c;
        pthread_mutex_unlock(&mutx);

        pthread_create(&t_id, NULL, handle_clin, (void*)&sock_c);
        pthread_detach(t_id);
        printf("connected client IP: %s and port: %d\n", inet_ntoa(clin_addr.sin_addr), clin_addr.sin_port);
                    
    }  
    close(sock_l);
    return 0;
}


void *handle_clin(void * arg){
    int clin_sock = *((int*)arg);
    int str_len = 0;
    char msg[BUF_SIZE];

    while((str_len = read(clin_sock, msg, sizeof(msg))) != 0)
        send_mas(msg, str_len);

    pthread_mutex_lock(&mutx);
    for(int i = 0; i < clin_count; i++){
        if(clin_sock == clin_socks[i]){
            while (i < clin_count - 1) {    //这里使用数组内部元素移动的方式处理需要断开的套接字
                clin_socks[i] = clin_socks[i + 1];
                i++;
            }
            break;
        }
    }
    clin_count--;
    pthread_mutex_unlock(&mutx);
    close(clin_sock);
    return NULL;
}


void send_mas(char * msg, int len){
    pthread_mutex_lock(&mutx);
    for(int i = 0; i < clin_count; i++){
        write(clin_socks[i], msg, len);
    }
    pthread_mutex_unlock(&mutx);
}
























