#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>//sockert_addr_in
#include <ctype.h>//toupper
#include <pthread.h>

#include "wrap.h"

#define MAXLINE 80
#define SERV_PORT 1214

struct s_info{
    struct sockaddr_in cliaddr;
    int cfd;
};


void *do_work(void *arg){
    int n,i;
    struct s_info *ts = (struct s_info*)arg;
    char buf[MAXLINE];
    char str[INET_ADDRSTRLEN];
    pthread_detach(pthread_self());//分离进程，就不用回收了
    while(1){
        n = Read(ts->cfd,buf,sizeof(buf));
        if(n==0){
            printf("The other side has been closed.\n");
            return 0;
        }else if(n==-1){
            perror("Read error");
            exit(1);
        }else{
            printf("Receive from %s at Port %d.\n",
            inet_ntop(AF_INET,&(*ts).cliaddr.sin_addr.s_addr,str,sizeof(str)),
            ntohs((*ts).cliaddr.sin_port));
            for(i=0;i<n;i++)
                buf[i]=toupper(buf[i]);
            Write(ts->cfd,buf,n);
            Write(STDOUT_FILENO,buf,n);
        }
        Close(ts->cfd);

        return (void *)0;
    }
}

int main(){
    int fd_listen,fd_connect;
    struct sockaddr_in serv_addr,clie_addr;
    socklen_t clie_addr_len;
    char buf[BUFSIZ],clie_IP[BUFSIZ];
    int i=0;
    pthread_t tid;
    struct s_info ts[256];
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

    //建立多线程服务器
    while(1){
        clie_addr_len=sizeof(clie_addr);
        fd_connect = Accept(fd_listen,(struct sockaddr*)&clie_addr, &clie_addr_len);
        //接收连接请求，clie_addr:传出参数，addr_len:传入传出参数，返回一个新的文件描述符，用于通信
        //阻塞等待客户端发起连接
        ts[i].cliaddr=clie_addr;
        ts[i].cfd=fd_connect;
        pthread_create(&tid,NULL,do_work,(void*)&ts[i]);
        i++;
    }
    return 0;
}