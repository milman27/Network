#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

int main(int lol, char** xd){
const struct sockaddr addr = {AF_UNIX, "a"};
int socket2 = socket(AF_UNIX,SOCK_STREAM,0);
int s1;
    if(socket2 == -1){
        printf("Error is %d", errno);
        return 1;
    }
unlink("a");
int binded = bind(socket2, &addr, sizeof(addr) + sizeof("a"));
    if (binded == -1){
        printf("Error is %d", errno);
        return 2;
    }
    listen(socket2, 1);
    for(;;){
        s1 = accept(socket2, NULL,NULL);
        if(s1 == -1){
            printf("Error is %d", errno);
            return 3;
        }
        printf("Connected through %d", s1);
        for(;;){
            char text[1];
            ssize_t n = read(s1, &text, sizeof(text));
            if(n < 1){
                break;
            }
            putchar(text[0]);
        }
        close(s1);
    }
    close(socket2);
    unlink("a");
printf("%d", socket2);
    return 0;
}

