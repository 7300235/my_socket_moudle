#ifndef _SEMPHO_H
#define _SEMPHO_H


#include<pthread.h>
#include<semaphore.h>

//locker
class sempho{
public:    
    sempho(int num=0){
        if(sem_init(&t,0,num)==-1){
            throw std::exception();
        }
    }
    ~sempho(){
        sem_destroy(&t);//自动destroy，非常方便
    }

    bool wait(){
        return sem_wait(&t)!=-1;
    }

    bool post(){
        return sem_post(&t)!=-1;    
    }

private:
    sem_t t;
};

class locker{
public:
    locker(){
        int ret=pthread_mutex_init(&mut_t,nullptr);
        if(ret!=0) throw std::exception();
    }
    ~locker(){
        pthread_mutex_destroy(&mut_t);
    }


public:
    bool lock(){
        return pthread_mutex_lock(&mut_t)==0;
    }
    bool unlock(){
        return pthread_mutex_unlock(&mut_t);
    }

private:
    pthread_mutex_t mut_t;

};

#endif