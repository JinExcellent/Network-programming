#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


#define BUFSIZE 1024



/*非分离I/O客户端*/


int main(int argc, char *argv[]){
    int sock;
    struct sockaddr_in sevr_addr;
    char message[BUFSIZE];
    int str_len;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    memset(&sevr_addr, 0, sizeof(sevr_addr));
    sevr_addr.sin_family = AF_INET; 
    sevr_addr.sin_addr.s_addr = inet_addr(argv[1]);
    sevr_addr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr*)&sevr_addr, sizeof(sevr_addr)) == -1){
        perror("connect error\n");
        exit(1);
    }else
        puts("Connected....");
    while (1) {
        fputs("input message(Q to quit):", stdout);

        fgets(message, BUFSIZ, stdin);
        if(!strcmp(message, "q\n") || !strcmp(message, "Q\n")){
            puts("quit");
            break;
        }

        str_len = write(sock, message, strlen(message));

        int recv_len = 0;
        int recv_cnt = 0;

        while(recv_len < str_len){
            recv_cnt = read(sock, &message[recv_len], BUFSIZ - 1);    
            if(recv_cnt)
                perror("read error\n");
            recv_len += recv_cnt;
        }
        message[recv_len] = 0;
        printf("massage from server: %s", message);
    }
    close(sock);

    return 0;
}





















