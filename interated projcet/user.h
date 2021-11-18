#ifndef USER_H
#define USER_H
//users
#include<netinet/in.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<cstring>
#include<unistd.h>

#define BUFSIZE 128
class user{
public:
    user();
    user(int file,sockaddr_in6 address);
    ~user();

private:
    void user_init();//user constctor

public:
    static int user_num;
    static int epfd;

public:
    bool uread();//epoo_in
    bool uwrite();//epoll_out
    void uclose();


private:
    int fd;
    sockaddr_in6 clntaddr;

public://???question
    char ureadbuf[BUFSIZE];
    char uwritebuf[BUFSIZE];
    int byte_to_send;//process的接口
};


#endif
