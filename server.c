#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>//sockert_addr_in
#include <ctype.h>//toupper
#include <sys/select.h>

#include "wrap.h"

#define MAXLINE 80
#define SERV_PORT 1214

int main(){
    int fd_listen,fd_connect,maxfd,sockfd;
    int i,maxi,nready,n;
    int client[FD_SETSIZE];
    struct sockaddr_in serv_addr,clie_addr;
    socklen_t clie_addr_len;
    char buf[BUFSIZ],clie_IP[BUFSIZ];
    fd_set rset,allset;
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
    maxfd=fd_listen;
    maxi=-1;
    for(i=0;i<FD_SETSIZE;i++)
        client[i]=-1;
    FD_ZERO(&allset);
    FD_SET(fd_listen,&allset);

    printf("Accepting connections...\n");

    //建立SELECT IO复用模型
    while(1){
        rset=allset;
        nready=select(maxfd+1,&rset,NULL,NULL,NULL);
        if(nready<0)
            perr_exit("Select error.");
        if(FD_ISSET(fd_listen,&rset)){
            clie_addr_len=sizeof(clie_addr);
            fd_connect = Accept(fd_listen,(struct sockaddr*)&clie_addr, &clie_addr_len);
            //接收连接请求，clie_addr:传出参数，addr_len:传入传出参数，返回一个新的文件描述符，用于通信
            //阻塞等待客户端发起连接，在select中由于只在select返回值的时候accept所以不阻塞
            printf("Receive from %s at port %d.\n",inet_ntop(AF_INET,&clie_addr.sin_addr.s_addr,clie_IP,sizeof(clie_IP)),
                    ntohs(clie_addr.sin_port));
            for(i=0;i<FD_SETSIZE;i++){
                if(client[i]<0){
                    client[i]=fd_connect;
                    break;
                }
            }
            if(i==FD_SETSIZE){
                fputs("Too many clients\n",stderr);
                exit(1);
            }
            FD_SET(fd_connect,&allset);//添加新描述符
            if(fd_connect>maxfd)
                maxfd=fd_connect;
            if(i>maxi)
                maxi=i;
            if(--nready==0)
                continue;
        }
        
        for(i=0;i<=maxi;i++){
            if((sockfd=client[i])<0)
                continue;
            if(FD_ISSET(sockfd,&rset)){
                if((n=Read(sockfd,buf,MAXLINE))==0){
                    Close(sockfd);
                    FD_CLR(sockfd,&allset);
                    client[i]=-1;
                }
                else{
                    for(int j=0;j<n;j++)
                        buf[j]=toupper(buf[j]);
                    Write(sockfd,buf,n);
                }
                if(--nready==0)
                    break;
            }
        }

    }
    Close(fd_listen);
    return 0;
}
