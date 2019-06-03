#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>//sockert_addr_in
#include <ctype.h>//toupper
//#include <sys/select.h>
//#include <poll.h>
#include <sys/epoll.h>
#include <errno.h>

#include "wrap.h"

#define MAXLINE 80
#define SERV_PORT 1214
#define OPEN_MAX 1024

int main(){
    int fd_listen,fd_connect,maxfd,sockfd;
    int i,j,maxi,nready,n,epfd,res;
    int client[OPEN_MAX];
    struct sockaddr_in serv_addr,clie_addr;
    socklen_t clie_addr_len;
    // client[OPEN_MAX];
    struct epoll_event curepfd,epollfd[OPEN_MAX];
    char buf[BUFSIZ],clie_IP[BUFSIZ];
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

    for(i=0;i<OPEN_MAX;i++){
        client[i]=-1;
    }
    maxi=-1;

    epfd = epoll_create(OPEN_MAX);
    if(epfd == -1)
        perr_exit("Epoll create error.\n");
    
    curepfd.events=EPOLLIN;curepfd.data.fd=fd_listen;

    res=epoll_ctl(epfd,EPOLL_CTL_ADD,fd_listen,&curepfd);
    //将建立连接请求加入到红黑树当中去
    if(res==-1)
        perr_exit("Epoll ctr error.");
    
    // client[0].fd=fd_listen;
    // client[0].events=POLLIN;//监听连接请求读事件
    // maxi=0;
    // for(i=0;i<OPEN_MAX;i++)
    //     client[i].fd=-1;

    printf("Accepting connections...\n");

    //建立EPOLL IO复用模型
    while(1){
        nready=epoll_wait(epfd,epollfd,OPEN_MAX,-1);//监听如果有时间将其加入epoll数组当中
        if(nready == -1)
            perr_exit("Epoll wait error.");
        for(i=0;i<nready;i++){
            if(!(epollfd[i].events & EPOLLIN))
                continue;
            if(epollfd[i].data.fd==fd_listen){
                clie_addr_len=sizeof(clie_addr);
                fd_connect = Accept(fd_listen,(struct sockaddr*)&clie_addr, &clie_addr_len);
                //接收连接请求，clie_addr:传出参数，addr_len:传入传出参数，返回一个新的文件描述符，用于通信
                //阻塞等待客户端发起连接，在select中由于只在select返回值的时候accept所以不阻塞
                printf("Receive from %s at port %d.\n",inet_ntop(AF_INET,&clie_addr.sin_addr.s_addr,clie_IP,sizeof(clie_IP)),
                        ntohs(clie_addr.sin_port));
                for(j=0;j<OPEN_MAX;j++){
                    if(client[j]<0){
                        client[j]=fd_connect;
                        break;
                    }
                }
        
                if(j==OPEN_MAX){
                fputs("Too many clients\n",stderr);
                    exit(1);
                }
                if(j>maxi)
                    maxi=j;
                curepfd.events=EPOLLIN;
                curepfd.data.fd=fd_connect;
                res = epoll_ctl(epfd,EPOLL_CTL_ADD,fd_connect,&curepfd);
                if(res == -1)
                    perr_exit("Epoll ctl error.\n");
            }
            else{
                sockfd=epollfd[i].data.fd;
                n=Read(sockfd,buf,MAXLINE);
                if(n==0){
                    for(j=0;j<maxi;j++){
                        if(client[j]==sockfd){
                            client[j]=-1;
                            break;
                        }
                    }
                    res=epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,NULL);
                    if(res==-1)
                        perr_exit("Epoll ctl error.\n");

                    Close(sockfd);
                    printf("Client[%d] closed connection\n",j);
                }
                else{
                    for(j=0;j<n;j++)
                        buf[j]=toupper(buf[j]);
                    Write(sockfd,buf,n);
                }
            }
        }
    }
    Close(fd_listen);
    Close(epfd);
    return 0;
}
