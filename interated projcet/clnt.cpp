#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<string.h>
#include<cstdio>
#include<unistd.h>

#include<cassert>
#include<cerrno>

#include<iostream>
using namespace std;

#define SERV_IP6 "::1"
#define SERV_PORT 6666

int ret=0;

int main(){
    sockaddr_in6 servaddr;
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin6_family=AF_INET6;
    servaddr.sin6_port=htons(SERV_PORT);
    ret=inet_pton(AF_INET6,SERV_IP6,&servaddr.sin6_addr);//may error
    assert(ret!=-1);

    int ret=0;
    int cfd=socket(AF_INET6,SOCK_STREAM,0);
    assert(cfd!=-1);
    ret=connect(cfd,(sockaddr*)&servaddr,sizeof(servaddr));
    assert(ret!=-1);
    
    char readbuf[10];
    char writebuf[10];
    ret=write(cfd,writebuf,10);
    assert(ret>0);
    ret=read(cfd,readbuf,10);
    assert(ret>0);
    printf("%s\n",readbuf);

    cout<<"done!\n";
    close(cfd);

    return 0;
}
