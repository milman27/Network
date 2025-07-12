#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <netinet/ip.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <curses.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <linux/limits.h>
#include <time.h>
char* numToStr(int value);

int main(int argc, char *argv[]){
    char cwd[PATH_MAX];
    if(argc == 1){
        getcwd(cwd, PATH_MAX*sizeof(char)); 
    }else if (argc == 2){
        strncpy(cwd, argv[1], PATH_MAX*sizeof(char));
    }else{
        printf("Too Many Arguments");
        return 10;
    }
    char* path = strncat(cwd, "/love.html",  (size_t)11 );
    if(access(cwd, F_OK) != 0){
        printf("Cannot find file specified: %s", cwd);
        return 11;
    }
    int htmlFile = open(path, O_RDONLY);
    struct stat stat = {0}; 
    fstat(htmlFile, &stat);
    char *html = malloc(stat.st_size);
    read(htmlFile, html, stat.st_size);
    printf("%s",html);
    char* timeoutHeader = 
        "HTTP/1.1 408 Request Timeout\r\nContent-Type: text/plain\r\nContent-Length: 19\r\nConnection: close\r\n\r\nRequest timed out.\r\n";
        
    char* htmlheader = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: "; 
    char* htmlEnd = "\r\n\r\n";

    in_port_t port = htons(9005);
    const struct in_addr sin_addr = {INADDR_ANY};
    const struct sockaddr_in ip = {AF_INET,port,sin_addr};
    int s0 = socket(AF_INET, SOCK_STREAM, 0);
    const int optval = 1;
    int opt = setsockopt(s0, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    
    if(opt < 0){
        printf("Err is %d", errno);
        return 5;
    }

    int s1;
    int pollsock;
    int pollstdin;
    int pollread;
    struct pollfd pollfd = {s0, POLLIN, 0};
    struct pollfd pollin = {0, POLLIN, 0};
    if(s0 == -1){
        printf("Error is %d",errno);
        return 1;
    }
    //unlink("192.168.178.20");
    int br = bind(s0, (struct sockaddr*)&ip, sizeof(ip));
    if(br == -1){
        printf("Error is %d",errno);
        return 2;
    }
    listen(s0, 1);
    for(;;){
        pollstdin = poll(&pollin, (nfds_t)1, 50);
        pollsock = poll(&pollfd, (nfds_t)1 , 50);
        if(pollstdin > 0){
            char buf[3];
            for(;;){
                char input[1];
                read(0, &input, sizeof(input));
                if(buf[0] == 'r' && buf[1] == 'e' && buf[2] == 'l' && input[0] == 'o'){
                    printf("reloading html file\n");
                    htmlFile = open(path, O_RDONLY);
                    fstat(htmlFile, &stat);
                    free(html);
                    html = malloc(stat.st_size/sizeof(char));
                    read(htmlFile, html, stat.st_size);
                }
                if(input[0] == '\n'){
                    break; 
                }
                buf[0] = buf[1];
                buf[1] = buf[2];
                buf[2] = input[0];
            }
        }
        if(pollsock > 0){
            time_t connectTime= time(NULL);
            time_t sendTime= time(NULL);
            s1 = accept(s0, NULL, NULL);
            struct tm * timeinfo;
            timeinfo = localtime(&connectTime);
            struct sockaddr_in peeraddr = {0};
            socklen_t addrlen;
            char ipaddr[INET_ADDRSTRLEN];
            getpeername(s1, (struct sockaddr *)&peeraddr, &addrlen);
            inet_ntop(AF_INET, &(peeraddr.sin_addr.s_addr), ipaddr, INET_ADDRSTRLEN);
            printf("Connected to %s:%d\n", ipaddr, ntohs(peeraddr.sin_port) );
            printf("Connected at: %s" , asctime(timeinfo));
            struct pollfd pollrd = {s1, POLLIN, 0};
            char prev[3];
            for(;;){
                if(time(NULL) - connectTime > 60 || time(NULL) - sendTime > 1){
                    write(s1,timeoutHeader, 117);
                    close(s1);
                    break;
                }
                char text[1];
                pollread = poll(&pollrd,(nfds_t)1, 50);
                if(pollread > 0){
                    
                    sendTime = time(NULL);
                    ssize_t n = read(s1, &text, sizeof(text));
                    putchar(text[0]);
                    int c = text[0];

                    if(n < 1){
                        break;
                    }
                    if(prev[2] == '\r'&& prev[1] == '\n' && prev[0] == '\r' && c == '\n'){
                        write(s1, htmlheader, strlen(htmlheader));
                        // printf(htmlheader);
                        char* size = numToStr((int)stat.st_size -1);
                        write(s1, size,strlen(size));
                        // printf(size);
                        write(s1, htmlEnd, 4);
                        // printf(htmlEnd);
                        printf("sizeof(html) = %d\n" ,(int) stat.st_size);
                        write(s1, html, (size_t)stat.st_size -1 );
                        // printf(html);
                        write(s1, htmlEnd, 4);
                        close(s1);
                        prev[0] = 0;
                        prev[1] = 0;
                        prev[2] = 0;
                        break;
                    }
                    prev[2] = prev[1];
                    prev[1] = prev[0];
                    prev[0] = c;
                }
            }
        }
    }
    close(s0);
    return 0;

}
char* numToStr(int value){
    char* ret = malloc(100);
    int ptr = 0;
    bool top = FALSE;
    for(int i = 12 ; i >= 0; i--){
        double power = pow(10, i); 
        if(power > value && !top ){
            continue;
        }
        top = TRUE;
        ret[ptr++] = value/(int)power + '0';
        value = value - ((value/(int)power) * (int)power);
    }
    ret[ptr] = '\0'; 

    return ret;

}

