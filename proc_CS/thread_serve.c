#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>





pthread_mutex_t mutx;


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

    while (1) {
        clin_addr_sz = sizeof(clin_addr);
        sock_c = accept(sock_l, (struct sockaddr *)&clin_addr, &clin_addr_sz);
                    
    }


    return 0;
}




























