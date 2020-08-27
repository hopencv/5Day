#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <cstring>
#include <unistd.h>

void * function_A(void * num)
{
    int i=0;
    while(1)
    {
        printf("thread A i = [%3d]\n " ,i );
        i++;
        i%=100;
        sleep(2);
    }
}

void * function_B(void * num)
{
    int i=0;
    while(1)
    {
        printf("thread B i = [%3d]\n " ,i );
        i++;
        i%=100;
        sleep(1);
    }
}

int main(void)
{
    pthread_t pthread_A , pthread_B;
    int cnt=0;
    printf("Create Thread A\n");
    pthread_create(&pthread_A, NULL, function_A, NULL);
    pthread_create(&pthread_B, NULL, function_B, NULL);
    
    //pthread_join(pthread_A, NULL);
    //pthread_join(pthread_B, NULL);
    
    while(1)
    {
    
		printf("Thread test : %3d \n" , cnt);
		cnt++;
		cnt=cnt%100;
		sleep(1);
	}
    
    return 1;
}
