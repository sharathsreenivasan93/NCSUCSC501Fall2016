//
//  csc501hw1.c
//  
//
//  Created by Sharath Sreenivasan on 9/16/16.
//
//

#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include "rdtsc.h"

#define BUFFER_SIZE 25
#define READ_END 0
#define WRITE_END 1
#define READ_END2 0
#define WRITE_END2 1
#define SIZE 1048576


int i,j,time_calculate;
float average_system_call, average_process_call;
struct timeval t1, t2;
long number_of_iterations = 10000;
int fd[2];
sem_t pingsem, pongsem;

int byom()
{
    return (10);
}


long nanosec(struct timeval t)
{
    return ((t.tv_sec*1000000 + t.tv_usec)*1000);
}

void child_spawn(char* program, char** arg_list)
{
    pid_t child_pid;
    float child_spawn_time;
    time_calculate = gettimeofday(&t1,NULL);
    child_pid = fork();
    
    if(child_pid!=0)
    {
        printf("In child\n");
    }
    else
    {
        printf("In parent\n");
    }
    time_calculate = gettimeofday(&t2,NULL);
    child_spawn_time = (nanosec(t2) - nanosec(t1));
    printf("Child spawn time %f\n",child_spawn_time);
}


void process_thread()
{
    int a;
    char *arg_list[] = {
        "ls",
        "-l",
        "/",
        NULL
    };
    child_spawn("ls",arg_list);
    printf("Done with spawning\n");
}

void average_system_calculate()
{
    int file_name = open("testfile.txt", O_WRONLY | O_APPEND);
    
    time_calculate = gettimeofday(&t1,NULL);
    for (i = 0;i<number_of_iterations;i++)
        write(file_name,"1\n",1);
    
    time_calculate = gettimeofday(&t2,NULL);
    
    average_system_call = (nanosec(t2) - nanosec(t1))/(number_of_iterations*1.0);

    printf("Average System Call Time %f\n ",average_system_call);
}

void average_process_calculate()
{
    time_calculate = gettimeofday(&t1,NULL);
    for (i = 0;i<number_of_iterations;i++)
        j=byom();
    
    time_calculate = gettimeofday(&t2,NULL);
    
    average_process_call = (nanosec(t2) - nanosec(t1))/(number_of_iterations*1.0);
    
    printf("Average Process Call Time %f\n ",average_process_call);
}

void process_call()
{
    float process_call;
    time_calculate = gettimeofday(&t1,NULL);
    j=byom();
    time_calculate = gettimeofday(&t2,NULL);
    process_call = (nanosec(t2) - nanosec(t1))/(number_of_iterations*1.0);
    
    printf("Process Call Time %f\n ",average_process_call);
}

void *thread_1()
{
    printf("In the thread\n");
}
void kernel_thread()
{
    float thread_create_time;
    pthread_t thread_test;
    
    time_calculate = gettimeofday(&t1,NULL);
    
    pthread_create(&thread_test,NULL,thread_1,NULL);
    
    time_calculate = gettimeofday(&t2,NULL);
    
    thread_create_time = (nanosec(t2) - nanosec(t1));
    printf("Thread create time %f\n",thread_create_time);
}

void *ping(void *arg)
{
    for(i=0;i<100000;i++){
        sem_wait(&pingsem);
        //printf("Ping\n");
        sem_post(&pongsem);
    }
}

void *pong(void *arg)
{
    for(i=0;i<100000;i++){
        sem_wait(&pongsem);
        //printf("Pong\n");
        sem_post(&pingsem);
    }
}

void context_switch_thread()
{
    float context_switch_time;
    sem_init(&pingsem,0,0);
    sem_init(&pongsem,0,1);
    pthread_t ping_thread, pong_thread;
    time_calculate = gettimeofday(&t1,NULL);
    pthread_create(&ping_thread,NULL,ping,NULL);
    pthread_create(&pong_thread,NULL,pong,NULL);
    pthread_join(ping_thread,NULL);
    pthread_join(pong_thread,NULL);
    time_calculate = gettimeofday(&t2,NULL);
    context_switch_time = (nanosec(t2) - nanosec(t1))/100000;
    printf("Context Switch time %f\n",context_switch_time);
}

void context_switch_process()
{
    float context_switch_time_process;
    char write_msg[BUFFER_SIZE] = "Greetings";
    char read_msg[BUFFER_SIZE];
    int fd[2];
    int fd2[2];
    pid_t pid;
    if (pipe(fd) == -1) {
        printf("Pipe failed\n");
    }
    if (pipe(fd2) == -1) {
        printf("Pipe failed\n");
       
    }
    write(fd[WRITE_END], &write_msg, strlen(write_msg)+1);
    pid = fork();
    
    time_calculate = gettimeofday(&t1,NULL);
    if (pid>0){
        close(fd[READ_END]);
        close(fd2[WRITE_END2]);
        
        write(fd[WRITE_END], &write_msg, strlen(write_msg)+1);
        close(fd[WRITE_END]);
        
        read(fd2[READ_END2], &read_msg, BUFFER_SIZE);

    }
    
    else {
        close(fd[WRITE_END]);
        close(fd2[READ_END2]);

        read(fd[READ_END], &read_msg, BUFFER_SIZE);

        write(fd2[WRITE_END2], &write_msg, strlen(write_msg)+1);
        close(fd2[READ_END]);
    }
    time_calculate = gettimeofday(&t2,NULL);
    context_switch_time_process = (nanosec(t2) - nanosec(t1));
    printf("Context Switch time for a process %f\n",context_switch_time_process/2);
}

void ram_access_time()
{
    float ram_access_time_read;
    int a[SIZE], p,tmp,index = 0;
    for (i=0;i<SIZE;i++){
        a[i]=0;
    }
    time_calculate = gettimeofday(&t1,NULL);
    for(i=0;i<10000000;i++){
        tmp = a[index];
        index = (index + i + tmp) & 1048575;
    }
    time_calculate = gettimeofday(&t2,NULL);
    ram_access_time_read = (nanosec(t2)-nanosec(t1))/10000000.0;
    printf("RAM reading time is %f\n",ram_access_time_read);
}

void mem_read()
{
    int number_of_elements = 2000000, sum=0;
    int a[number_of_elements];
    float mem_read_time = 0.0,final_time;
    for(i=0;i<number_of_elements;i++){
        a[i]=5;
    }
    time_calculate = gettimeofday(&t1,NULL);
    for (i=0; i<number_of_elements; i+=10) {
        sum+=a[i];
        sum+=a[i+1];
        sum+=a[i+2];
        sum+=a[i+3];
        sum+=a[i+4];
        sum+=a[i+5];
        sum+=a[i+6];
        sum+=a[i+7];
        sum+=a[i+8];
        sum+=a[i+9];
    }
    time_calculate = gettimeofday(&t2,NULL);
    mem_read_time = nanosec(t2)-nanosec(t1);
    printf("Read time is %f\n",mem_read_time);
    final_time = (4.0*number_of_elements)/mem_read_time;
    printf("Read Bandwidth %f\n", final_time);
}

void mem_write()
{
    int number_of_elements = 100000, sum = 0;
    int a[number_of_elements], *p;
    float mem_write_time, final_time;
    p=a;
    time_calculate = gettimeofday(&t1,NULL);
    for(i=0;i<number_of_elements;i+=10){
        *(p) = 5;
        *(p+1) = 5;
        *(p+2) = 5;
        *(p+3) = 5;
        *(p+4) = 5;
        *(p+5) = 5;
        *(p+6) = 5;
        *(p+7) = 5;
        *(p+8) = 5;
        *(p+9) = 5;
        p = (p+10);
    }
    time_calculate = gettimeofday(&t2,NULL);
    mem_write_time = nanosec(t2)-nanosec(t1);
    printf("Write Time is %f\n",mem_write_time);
    final_time = (32.0*number_of_elements)/mem_write_time;
    printf("Write Bandwidth %f\n", final_time);
}

void page_fault()
{
    int fd,start,end,total=0;
    int offset = 1048576;
    fd = open("testfile.txt",O_RDWR);
    if (fd==-1)
    {
        perror("error message");
    }
    int size = 35340093;
    char* map;
    char b;
    map = (char*)mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    if(map == MAP_FAILED)
    {
        printf("Failure");
    }
    int result = 0;
    
    float page_fault_time;
    
    time_calculate = gettimeofday(&t1,NULL);
    for(int i = 0;i<100;i++)
    {
        b = map[((i*offset) % (size - 1))];
    }
    time_calculate = gettimeofday(&t2,NULL);
    page_fault_time = (nanosec(t2)-nanosec(t1))/(1000*1.0);
    printf("Page Fault Time %f\n", page_fault_time);
}

void overhead()
{
    float overhead_time;
    time_calculate = gettimeofday(&t1,NULL);
    
    for(i=0;i<10000;i++){
        
    }
    
    time_calculate = gettimeofday(&t2,NULL);
    overhead_time = (nanosec(t2)-nanosec(t1))/10000.0;
    printf("Overhead Time %f\n", overhead_time);
}
void read_time()
{
    float readtime,a,b;
    time_calculate = gettimeofday(&t1,NULL);
    for (i=0;i<1000;i++){
        a = rdtsc();
        b = rdtsc();
    }
    time_calculate = gettimeofday(&t2,NULL);
    readtime = (nanosec(t2)-nanosec(t1))/1000.0;
    printf("Read Time %f\n", readtime);
}


main()
{
    average_process_calculate();
    average_system_calculate();
    process_call();
    process_thread();
    kernel_thread();
    context_switch_thread();
    context_switch_process();
    ram_access_time();
    mem_read();
    mem_write();
    page_fault();
    overhead();
    read_time();
}

