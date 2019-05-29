#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>//sockert_addr_in
#include <ctype.h>//toupper

#include "wrap.h"

#define SERV_IP "127.0.0.1"
#define SERV_PORT 9699

int main(){
    int fd_listen,fd_connect;
    struct sockaddr_in serv_addr,clie_addr;
    socklen_t clie_addr_len;
    char buf[BUFSIZ],clie_IP[BUFSIZ];
    int n,i;
    fd_listen = Socket(AF_INET,SOCK_STREAM,0);
    //创建一个socket描述符，成功返回文件描述符，失败返回 -1；
    bzero(&serv_addr,sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(SERV_PORT);
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    Bind(fd_listen,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
    //绑定 IP 和 端口号 ，最后一个是结构体的长度（sizeof(addr)）
    //服务器不能随机绑定 IP 和 端口号，不调用bind()就是随机的
    Listen(fd_listen,128);
    //指定监听上限数，同时允许多少客户端建立连接；
    printf("Accepting connections...\n");
    //while(1){
    clie_addr_len=sizeof(clie_addr);
    fd_connect = Accept(fd_listen,(struct sockaddr*)&clie_addr, &clie_addr_len);
    //刻在脑子里，接收连接请求，clie_addr:传出参数，addr_len:传入传出参数，返回一个新的文件描述符，用于通信
    //阻塞等待客户端发起连接
    printf("Client IP:%s, Port:%d\n",
        inet_ntop(AF_INET,&clie_addr.sin_addr.s_addr,clie_IP,sizeof(clie_IP)),
        ntohs(clie_addr.sin_port));
    while(1){
        n = Read(fd_connect, buf,sizeof(buf));
        for(i = 0;i<n;i++){
            buf[i]=toupper(buf[i]);
        }
        Write(fd_connect,buf,n);
    }
    close(fd_listen);
    close(fd_connect);
    return 0;
}