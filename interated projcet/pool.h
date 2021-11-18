#ifndef _POOL_H
#define _POOL_H

#include"sempho.h"
#include<exception>
#include<list>
#include<pthread.h>

//pool.h
template<typename T>
class pool{
public:
    pool(int num);
    ~pool();

public:
    void append(T* user_data);


private:
    void pool_init(int num);//初始化，是pool函数

    void process(T* user_data);//永不停止的线程
    static void* worker(void* arg);//工作函数 在这里只是简单的将数字+1


private:
    sempho uqueuestat;//消息队列处理
    locker uqueuelocker;//工作队列锁
    std::list<T*> uworkqueue;//工作队列处理
    int pthread_num;
    bool stop;
    pthread_t* pid_list;
};




//constructor
template<typename T>
pool<T>::pool(int num){
    pthread_num=num;
    stop=0;
    uqueuelocker=locker();
    uqueuestat=sempho();
    pid_list=nullptr;
    pool_init(num);
}

template<typename T>
pool<T>::~pool(){
    stop=1;
    delete [] pid_list;
}

//prot
template<typename T>
void pool<T>::append(T* user_data){
    uworkqueue.push_back(user_data);
    uqueuestat.post();
}


//infunction
template<typename T>
void pool<T>::pool_init(int num){
    pid_list=new pthread_t(num);
    for(int i=0;i<num;i++){
        int ret=pthread_create(&pid_list[i],nullptr,worker,this);
        if(ret==-1){
            delete [] pid_list;
            throw std::exception();
        }
        ret=pthread_detach(pid_list[i]);
        if(ret==-1){
            delete [] pid_list;
            throw std::exception();
        }
    }
}

template<typename T>
void* pool<T>::worker(void *arg){
    auto poolpt=(pool<T>*)arg;
    while(poolpt->stop){
        poolpt->uqueuestat.wait();
        poolpt->uqueuelocker.lock();
        T* data=poolpt->uworkqueue.front();
        poolpt->uworkqueue.pop_front();
        poolpt->uqueuelocker.unlock();
        poolpt->process(data);
    }
}

template<typename T>
void pool<T>::process(T* user_data){
    int num=strlen(user_data->ureadbuf);

    for(int i=0;i<num;i++){
        user_data->uwritebuf[i]=user_data->ureadbuf[i]+1;
    }
    user_data->byte_to_send=num;
    return;
}

#endif
