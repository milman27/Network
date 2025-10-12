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
#include "parse.h"
#include <signal.h>
char* numToStr(int value);

int main(int argc, char *argv[]){
    char cwd[PATH_MAX] = {0};
    if(argc == 1){
        getcwd(cwd, PATH_MAX*sizeof(char)); 
    }else if (argc == 2){
        strncpy(cwd, argv[1], PATH_MAX*sizeof(char));
    }else{
        printf("Too Many Arguments");
        return 10;
    }
    if(chroot(cwd) != 0){
        printf("Could not chroot into %s\n", cwd);
        printf("Error is %d\nIs this okay?(y/N)", errno);
        if(getchar() != 'y'){
            return 1;
        }
    }
    signal(SIGPIPE, SIG_IGN);
    char* timeoutHeader = 
        "HTTP/1.1 408 Request Timeout\r\nContent-Type: text/plain\r\nContent-Length: 19\r\nConnection: close\r\n\r\nRequest timed out.\r\n";
    char* header404 =    
        "HTTP/1.1 404 File Not Found\r\nContent-Type: text/plain\r\nContent-Length: 16\r\nConnection: close\r\n\r\nFile not found.\r\n";
    char* htmlheader = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: "; 
    char* jsheader = "HTTP/1.1 200 OK\r\nContent-Type: application/javascript\r\nContent-Length: "; 
    char* cssheader = "HTTP/1.1 200 OK\r\nContent-Type: text/css\r\nContent-Length: "; 
    char* htmlEnd = "\r\n\r\n";

    in_port_t port = htons(9005);
    const struct in_addr sin_addr = {INADDR_ANY};
    const struct sockaddr_in ip = {AF_INET,port,sin_addr,{0}};
    int s0 = socket(AF_INET, SOCK_STREAM, 0);
    const int optval = 1;
    int opt = setsockopt(s0, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if(opt < 0){
        printf("Err is %d", errno);
        return 5;
    }

    int s1;
    int pollsock;
 //   int pollstdin;
    int pollread;
    struct pollfd pollfd = {s0, POLLIN, 0};
    //struct pollfd pollin = {0, POLLIN, 0};
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
        //pollstdin = poll(&pollin, (nfds_t)1, 50);
        pollsock = poll(&pollfd, (nfds_t)1 , 50);
        if(0){
            char buf[3];
            for(;;){
                char input[1];
                read(0, &input, sizeof(input));
                /*if(buf[0] == 'r' && buf[1] == 'e' && buf[2] == 'l' && input[0] == 'o'){
                  printf("reloading html file\n");
                  htmlFile = open(path, O_RDONLY);
                  fstat(htmlFile, &stat);
                  free(html);
                  html = malloc(stat.st_size/sizeof(char));
                  read(htmlFile, html, stat.st_size);
                  }*/
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
            struct tm* timeinfo;
            timeinfo = localtime(&connectTime);
            struct sockaddr_in peeraddr = {0};
            socklen_t addrlen;
            char ipaddr[INET_ADDRSTRLEN];
            getpeername(s1, (struct sockaddr *)&peeraddr, &addrlen);
            inet_ntop(AF_INET, &(peeraddr.sin_addr.s_addr), ipaddr, INET_ADDRSTRLEN);
            printf("Connected to %s:%d\n", ipaddr, ntohs(peeraddr.sin_port) );
            printf("Connected at: %s" , asctime(timeinfo));
            struct pollfd pollrd = {s1, POLLIN, 0};
            if(time(NULL) - connectTime > 60 || time(NULL) - sendTime > 1){
                write(s1,timeoutHeader, 117);
                printf("Closing %d due to timeout.\n",s1);
                close(s1);
                continue;
            }
            char text[10000];
            pollread = poll(&pollrd,(nfds_t)1, 50);
            if(pollread > 0){
                sendTime = time(NULL);
                int i = 0;
                for(;poll(&pollrd, (nfds_t)1, 50); i++){
                    if(!read(s1, &text[i], 1)){
                        printf("Client disconnected. Closing %d\n", s1);
                        close(s1);
                        i = 0;
                        break;
                    }
                } 
                if (i == 0){
                    continue;
                }
                text[i] = '\0';
                for(int j = 0; j < i; j++){
                    putchar(text[j]);
                }
                token* tokens = tokenizeString(text);
                /*for(int i = 0; tokens[i].type != END; i++){
                  printf("%d:%d ",tokens[i].type, i );
                  for(int j = 0; j < tokens[i].length; j++){
                  putchar(tokens[i].start[j]);
                  }
                  putchar('\n');
                  }*/
                HTTPRequest* request = parseHTTP(tokens);
                if(request == NULL){
                    printf("Invalid HTTP Request\n");
                    write(s1, header404, strlen(header404));
                    close(s1);
                    printf("Closing %d\n" ,s1);
                    continue;
                }
                char* path = NULL;
                if(stringCmp(&(request->headers[0].value[strlen(request->headers[0].value) - 1]) , "/", 1)){
                    path = malloc(strlen(request->headers[0].value)+11);
                    memcpy(path, request->headers[0].value, strlen(request->headers[0].value));
                    memcpy(path+strlen(request->headers[0].value), "index.html", 10);
                    path[strlen(request->headers[0].value) + 10] = '\0';
                }else{
                    path = malloc(strlen(request->headers[0].value)+1);
                    memcpy(path, request->headers[0].value, strlen(request->headers[0].value));
                    path[strlen(request->headers[0].value)] = '\0';
                }
                printf("Accessing path: %s\n", path);
                if(access(path, F_OK) != 0){
                    printf("Cannot find file specified: %s\n", path);
                    write(s1,header404, strlen(header404));
                    free(path);
                    destroyParsedHTTP(request);
                    close(s1);
                    printf("Closing %d\n" ,s1);
                    continue;
                }
                int htmlFile = open(path, O_RDONLY);
                struct stat stat = {0}; 
                fstat(htmlFile, &stat);
                char *html = malloc(stat.st_size + 1);
                read(htmlFile, html, stat.st_size);

                html[stat.st_size] = '\0';
                //printf("%s",html);
                if(stringCmp(&(request->headers[0].value[strlen(request->headers[0].value)-2]), "js", 2)){ 
                    write(s1, jsheader, strlen(jsheader));
                }else if (stringCmp(&(request->headers[0].value[strlen(request->headers[0].value)-3]), "css", 3)){ 
                    write(s1, cssheader, strlen(cssheader));
                }else{
                    write(s1, htmlheader, strlen(htmlheader));
                }
                // printf(htmlheader);
                char* size = numToStr((int)stat.st_size - 1);
                write(s1, size,strlen(size));
                // printf(size);
                write(s1, htmlEnd, 4);
                // printf(htmlEnd);
                printf("sizeof(html) = %d\n", (int)stat.st_size);
                write(s1, html, (size_t)stat.st_size - 1);
                // printf(html);
                write(s1, htmlEnd, 4);
                destroyParsedHTTP(request);
                free(html);
                free(path);
            }
            printf("Closing %d\n", s1);
            close(s1);
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
