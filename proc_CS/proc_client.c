#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>


#define BUFSIZE 30

/*这是一个I/O分离式的客户端**/

void write_routine(int , char *);
void read_routine(int , char *);


int main(int argc, char *argv[]){
    int sock;
    struct sockaddr_in serv_addr;

    if(argc < 3){
        printf("Usage %s <ip> <port> \n", argv[0]);
        exit(1);
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
        perror("connected error");
        exit(1);
    }

    pid_t pid = fork();
    char massage[BUFSIZ];
/*
 *注意：
 *此处可能认为程序会出现先读后写（即fork后主进程抢占处理机）的情况，
 *这是错误的认识，因为write/read本身为i/o操作，在进程调用io操作时，若无法获得需要的数据，系统内核会使该进程阻塞。直到数据到达后，被唤醒放入就绪队列，竞争cpu
 *
 *在对管道进行io操作时，也是此类情况
 *
 *
 * */
    if(pid == 0){
        write_routine(sock, massage);
    }else{
       read_routine(sock, massage); 
    }

    close(sock);
    return 0;
}



void write_routine(int sock, char *buf){
    while(1){
        fgets(buf, BUFSIZ, stdin);
        if(!strcmp(buf, "q\n") || !strcmp(buf, "Q\n")){
            puts("quit.");
            shutdown(sock, SHUT_WR);
            return;
        }
        write(sock, buf, strlen(buf));
    }
}

void read_routine(int sock, char *buf){
    while (1) {
        int str_len = read(sock, buf, BUFSIZ);
        if(str_len == 0){
                close(sock);
                return;
            }

        buf[str_len] = 0;
        printf("Message from server: %s\n", buf);
    }
}





