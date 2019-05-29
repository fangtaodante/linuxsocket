#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>//sockert_addr_in
#include <ctype.h>//toupper
#include "wrap.h"

#define SER_ADDR "127.0.0.1"
#define SER_PORT 9699

int main(){
    int fd_connect;
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    char buf[BUFSIZ];
    int n;
    fd_connect = Socket(AF_INET,SOCK_STREAM,0);
    inet_pton(AF_INET,SER_ADDR,&server_addr.sin_addr.s_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SER_PORT);
    Connect(fd_connect,(struct sockaddr*)&server_addr,sizeof(server_addr));
    while(1){
        fgets(buf,sizeof(buf),stdin); // hello world ---->hello world\n\0
        Write(fd_connect,buf,strlen(buf));
        n = Read(fd_connect,buf,sizeof(buf));
        Write(STDOUT_FILENO,buf,n);
    }

    close(fd_connect);
    return 0;
}