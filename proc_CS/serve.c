#include <asm-generic/socket.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>


#define BUFSIZE 30

void read_childporc(int sig){
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG);
    printf("removed proc id: %d\n", pid);
}


int main(int argc, char *argv[]){
    int sock_l, sock_c;
    struct sockaddr_in sevr_addr, clin_addr;
    socklen_t clin_sz; 
    struct sigaction act;
    
    if(argc < 2){
        printf("Usage: %s <port>\n",argv[0]);
        exit(1);
    }
    act.sa_flags = 0;
    act.sa_handler = read_childporc;
    sigemptyset(&act.sa_mask);
    sigaction(SIGCHLD, &act, 0);        //注册信号

    sock_l = socket(AF_INET, SOCK_STREAM, 0);

    //处理服务器断开连接后，可以将处于time-watie状态的套接字再次分配
    int option;
    socklen_t optlen = sizeof(option);
    option = 1;
    setsockopt(sock_l, SOL_SOCKET, SO_REUSEADDR, (void*)&option, optlen);
   
    memset(&sevr_addr, 0, sizeof(sevr_addr));    
    sevr_addr.sin_family = AF_INET;
    sevr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sevr_addr.sin_port = htons(atoi(argv[1]));

    if(bind(sock_l, (struct sockaddr*)&sevr_addr, sizeof(sevr_addr)) == -1){
        perror("bind error\n");
        exit(1);
    }

    if(listen(sock_l, 5) == -1){
        perror("listen error\n");
        exit(1);
    }
    
    /*加入管道模块，让一个子进程从管道中读取数据并写入到指定的文件*/
    int fds[2];
    pipe(fds);
    pid_t pid_pipe = fork();  
    
    if(pid_pipe == 0){
        FILE *fp = fopen("echomsg.txt", "wa");  //问题：没法实现非截断式的文件写入
        char msgbuf[BUFSIZE];
        int len = 0;

        for(int i = 0; i < 10; i++){
            len = read(fds[0], msgbuf, BUFSIZE - 1);
                fwrite((void *)msgbuf, 1, len, fp);
                fflush(fp);

        }
        fclose(fp);
        return 0;
    }


    char buf[BUFSIZE];
    int  str_len;

    while(1){
        sock_c = accept(sock_l, (struct sockaddr *)&clin_addr, &clin_sz);
        if(sock_c == -1){
            //perror("accept error\n");  //着用用于打印出异常的来源
            continue;
        }else {
            puts("new client connected....");
        }
        
        pid_t pid = fork(); 
        
        if(pid == -1){
            close(sock_c);
            continue;
        }if(pid == 0){
            close(sock_l);
            while((str_len = read(sock_c, buf, sizeof(buf))) != 0){
                write(sock_c, buf, str_len);
                write(fds[1], buf, str_len);
            }
            
            close(sock_c);
            puts("client disconnected...");
            return 0;
        }else {
            close(sock_c);
        }
        
    }

    close(sock_l);
    return 0;

}
