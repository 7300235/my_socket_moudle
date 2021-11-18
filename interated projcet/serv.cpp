#include<cstring>
#include<netinet/in.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/uio.h>
#include<unistd.h>
#include<csignal>
#include<cassert>
#include<cerrno>
#include<cstdio>
#include<pthread.h>


#include<unordered_map>
#include<iostream>
using namespace std;

#include"pool.h"
#include"user.h"

#define SERV_IP6 "::1"
#define PORT 6666
#define MAXUSERS 10

int ret=0;
int user::user_num=0;
int user::epfd=0;

extern int setnoblocking(int fd);
extern bool addepfd(int epfd,int fd,bool effience);

//ctrl+c action
void action(int sig){

}


//main function
int main(){
    sockaddr_in6 servaddr;
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin6_port=htons(PORT);
    servaddr.sin6_family=AF_LOCAL;
    inet_pton(AF_LOCAL,SERV_IP6,&servaddr);

    int lfd=socket(AF_LOCAL,SOCK_STREAM,0);
    int reuse=1;
    setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
    ret=bind(lfd,(sockaddr*)&servaddr,sizeof(servaddr));
    assert(ret!=-1);
    listen(lfd,1000);

    user::epfd=epoll_create(2*MAXUSERS);
    ret=addepfd(user::epfd,lfd,0);
    assert(ret!=-1); 
    epoll_event dealevent[2*MAXUSERS];
    unordered_map<int,user> users_table;
    pool<user>* poolpt=nullptr;
    try{
        poolpt=new pool<user>(pool<user>(3));
    }catch(...){
        return 1;
    }

    while(1){
        int n=epoll_wait(user::epfd,dealevent,2*MAXUSERS,-1);//time 选项
        if(n==-1 && errno != EINTR){
            perror("epoll failure!\n");
        }

        for(int i=0;i<n;i++){
            
            if(dealevent[i].data.fd=lfd){
                if(dealevent[i].events & EPOLLIN){
                    sockaddr_in6 clntaddr;
                    memset(&clntaddr,0,sizeof(clntaddr));
                    socklen_t clntlen=sizeof(clntaddr);
                    int cfd=accept(lfd,(sockaddr*)&clntaddr,&clntlen);
                    if(cfd<0){perror("accept failure!\n"); continue;}
                    
                    user::user_num++;
                    if(user::user_num>MAXUSERS){
                        cout<<"too much connections!\n";
                        user::user_num--;
                        close(cfd);
                        continue;
                    }
                    users_table[cfd]=user(cfd,clntaddr);//将事件注册到事件表里面

                }else if(dealevent[i].events & EPOLLERR){
                    perror("epoll error!\n");
                    break;
                }
            }else if(dealevent[i].events & EPOLLIN){
                if(!users_table[dealevent[i].data.fd].uread()){
                    users_table.erase(dealevent[i].data.fd);
                    continue;
                }
                poolpt->append(&users_table[dealevent[i].data.fd]);//关于容器对象的类型
            }else if(dealevent[i].events & EPOLLOUT){
                if(!users_table[dealevent[i].data.fd].uwrite()){
                    users_table.erase(dealevent[i].data.fd);
                }
            }else if(dealevent[i].events & EPOLLERR){
                users_table[dealevent[i].data.fd].uclose();
                users_table.erase(dealevent[i].data.fd);
            }else if(dealevent[i].events & EPOLLHUP){
                users_table[dealevent[i].data.fd].uclose();
                users_table.erase(dealevent[i].data.fd);
            }
        }
    }

    close(lfd);
    delete poolpt;
    close(user::epfd);
    for(auto elem:users_table){close(elem.first);}
    return 0;
}

/*int setnoblocking(int fd){
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return new_option;
}

bool addepfd(int epfd,int fd,bool effience=1){
    epoll_event basevent;
    basevent.events=EPOLLIN | EPOLLERR;
    if(effience){
    basevent.events|=EPOLLONESHOT;
    }
    basevent.data.fd=fd;
    setnoblocking(fd);
    return epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&basevent)!=-1;
}//epollin epollerr epollhup
*/