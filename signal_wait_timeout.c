#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>
using namespace  std;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t flag = PTHREAD_COND_INITIALIZER;

void *numWords(void *f)
{       
    pthread_mutex_lock(&mutex);
    for(int i=0;i<60000;i++)
    cout<<"pth2"<<endl;
    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&flag);
        
    return NULL;
}

int main(int argc ,char *argv[])
{
    pthread_t t1;
    void *numWords(void *);
    void *ret;
    clock_gettime((__clockid_t)CLOCK_MONOTONIC, t);
    t->tv_sec = t->tv_sec + (ms/1000);
    t->tv_nsec = t->tv_nsec + ((ms % 1000) * 1000000);
     
    pthread_create(&t1,NULL,numWords,NULL);
        
    pthread_mutex_lock(&mutex); 
    int err = pthread_cond_timedwait(&flag,&mutex,&timeToWait);
    cout<<"pth1:"<<err<<"ETIME:"<<ETIMEDOUT<<endl;
    pthread_mutex_unlock(&mutex);
    pthread_join(t1,&ret);  
    return 0;
}
