#include"user.h"

int setnoblocking(int fd){
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

void modepfd(int epfd,int fd,int option){
    epoll_event event;
    event.data.fd=fd;
    event.events|=option;
    epoll_ctl(epfd,fd,EPOLL_CTL_MOD,&event);
}

void delepfd(int epfd,int fd){
    epoll_ctl(epfd,EPOLL_CTL_DEL,fd,nullptr);
}


//constructor
//constructor
user::user(){;}

user::user(int file,sockaddr_in6 address){
    fd=file;
    memset(&clntaddr,0,sizeof(clntaddr));
    clntaddr=address;
    byte_to_send=0;
    user_init();
}


user::~user(){
    uclose();
}

void user::user_init(){
    addepfd(epfd,fd);
}

//main process

bool user::uread(){
    int ret=recv(fd,ureadbuf,128,MSG_DONTWAIT);
    modepfd(epfd,fd,EPOLLHUP);
    modepfd(epfd,fd,EPOLLOUT);
    modepfd(epfd,fd,~EPOLLIN);
    return ret;
}

bool user::uwrite(){
    if(byte_to_send==0){return true;}
    else{
        int snum=send(fd,uwritebuf,byte_to_send,MSG_DONTWAIT);
        /*if(snum>0 && errno==EAGAIN){//ok?}*/

        if(snum<=0){return false;}//error 处理
        else{//还有没有送完的情况
            modepfd(epfd,fd,~EPOLLOUT);//正常送完的情况
            modepfd(epfd,fd,EPOLLIN);
            return true;
        }

    }
}

void user::uclose(){
    close(fd);
    delepfd(epfd,fd);
    user_num--;
}
