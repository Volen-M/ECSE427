#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <ucontext.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "sut.h"
#include "queue.h"

typedef struct _taskdesc{
	int taskid;
	char *taskstack;
	void *taskfunc;
	ucontext_t taskcontext;
    ucontext_t mastercontext;
}taskdesc;



int sockfd = -1;
int numtasks = 0;

bool shutdownTriggered;                                     //Trigger to know that when currently scheduled tasks are done it should shutdown
bool hasAquiredMsg = false;                                 //Trigger for when C-EXEC goes back in sut_read
bool isConnected = false;                                   //Trigger for knowing if read should attempt and for when C-EXEC goes back in sut_open

char sutReadMsg[BUFSIZE];

pthread_mutex_t waitlock = PTHREAD_MUTEX_INITIALIZER;       //I-EXEC queue lock
pthread_mutex_t readylock = PTHREAD_MUTEX_INITIALIZER;      //C-EXEC queue lock

struct queue readyqueue;                                    //C-EXEC queue
struct queue waitingqueue;                                  //I-EXEC queue (All I-EXEC queue members have passed through C-EXEC queue)

taskdesc *currTaskCexec;
taskdesc *currTaskIexec;

pthread_t thread_handle_iexec;
pthread_t thread_handle_cexec;

ucontext_t contextcexec, contextiexec;



/**
 * This is the Helper Function For sut_open for I-Exec, it goes back to sut_open in C-EXEC
 * */
void sut_open_iexec(char *dest, int port){
    struct sockaddr_in server_address = { 0 };

    // create a new socket
    int socketfd= socket(AF_INET, SOCK_STREAM, 0);
    sockfd = socketfd;
    if (sockfd < 0) {
        perror("Failed to create a new socket\n");
        return;
    }

    // connect to server
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, dest, &(server_address.sin_addr.s_addr));
    server_address.sin_port = htons(port);

    //Puts task back in wait queue until a connection is successful
    for(;;){
        if (connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
            isConnected = false;
            struct queue_entry *tempEntry = queue_new_node(currTaskIexec);
            tempEntry = queue_new_node(currTaskIexec);
            pthread_mutex_lock(&waitlock);
            queue_insert_tail(&waitingqueue,tempEntry);
            pthread_mutex_unlock(&waitlock);
            swapcontext(&(currTaskIexec->taskcontext),&contextiexec);
        }
        else{
            //Goes to C-Exec ready queue
            isConnected = true;
            taskdesc *newtask;
            newtask = malloc(sizeof(taskdesc));
            getcontext(&(newtask->taskcontext));
            newtask->taskid = numtasks;
            newtask->taskstack = (char *)malloc(THREAD_STACK_SIZE);
            newtask->taskcontext.uc_stack.ss_sp = newtask->taskstack;
            newtask->taskcontext.uc_stack.ss_size = THREAD_STACK_SIZE;
            newtask->taskcontext.uc_link = 0;
            newtask->taskcontext.uc_stack.ss_flags = 0;
            newtask->taskcontext = currTaskIexec->mastercontext;
            struct queue_entry *tempEntry = queue_new_node(newtask);
            pthread_mutex_lock(&readylock);
            queue_insert_tail(&readyqueue,tempEntry);
            pthread_mutex_unlock(&readylock);
            swapcontext(&(currTaskIexec->taskcontext),&contextiexec);
            break;
        }
    }
}

/**
 * This is the Helper Function For Sut_Write for I-Exec it never goes back to C-Exec
 * */
void sut_write_iexec(char *buf, int size){
     for(;;){
        if (send(sockfd, buf, size, MSG_DONTWAIT)==-1){
            struct queue_entry *tempEntry = queue_new_node(currTaskIexec);
            tempEntry = queue_new_node(currTaskIexec);
            pthread_mutex_lock(&waitlock);
            queue_insert_tail(&waitingqueue,tempEntry);
            pthread_mutex_unlock(&waitlock);
            swapcontext(&(currTaskIexec->taskcontext),&contextiexec);
        }
        else{
            break;
        }
    }
    numtasks--;
    swapcontext(&(currTaskIexec->taskcontext),&contextiexec);
}

/**
 * This is the Helper Function For Sut_read for I-Exec, it goes back to sut_read in C-EXEC
 * */
void sut_read_iexec(){

    for(;;){
        if (isConnected == false) {
            //perror("Failed to connect to server\n");
            struct queue_entry *tempEntry = queue_new_node(currTaskIexec);
            tempEntry = queue_new_node(currTaskIexec);
            pthread_mutex_lock(&waitlock);
            queue_insert_tail(&waitingqueue,tempEntry);
            pthread_mutex_unlock(&waitlock);
            swapcontext(&(currTaskIexec->taskcontext),&contextiexec);
        }
        else{
            break;
        }
    }

    ssize_t byte_count;
    for(;;) {
        memset(sutReadMsg, 0, sizeof(sutReadMsg));
        byte_count = recv(sockfd, sutReadMsg, BUFSIZE, 0);
        if (byte_count>0){
            hasAquiredMsg = true;
            taskdesc *newtask;
            newtask = malloc(sizeof(taskdesc));
            getcontext(&(newtask->taskcontext));
            newtask->taskid = numtasks;
            newtask->taskstack = (char *)malloc(THREAD_STACK_SIZE);
            newtask->taskcontext.uc_stack.ss_sp = newtask->taskstack;
            newtask->taskcontext.uc_stack.ss_size = THREAD_STACK_SIZE;
            newtask->taskcontext.uc_link = 0;
            newtask->taskcontext.uc_stack.ss_flags = 0;
            newtask->taskcontext = currTaskIexec->mastercontext;
            struct queue_entry *tempEntry = queue_new_node(newtask);
            pthread_mutex_lock(&readylock);
            queue_insert_tail(&readyqueue,tempEntry);
            pthread_mutex_unlock(&readylock);
            swapcontext(&(currTaskIexec->taskcontext),&contextiexec);
            return;
        }
        else{
            struct queue_entry *tempEntry = queue_new_node(currTaskIexec);
            tempEntry = queue_new_node(currTaskIexec);
            pthread_mutex_lock(&waitlock);
            queue_insert_tail(&waitingqueue,tempEntry);
            pthread_mutex_unlock(&waitlock);
            swapcontext(&(currTaskIexec->taskcontext),&contextiexec);
        }
    }
    return;
}


void sut_close_iexec(){
    close(sockfd);
    sockfd = -1;
    isConnected = false;
    numtasks--;
    swapcontext(&(currTaskIexec->taskcontext),&contextiexec);
}

void *iexec(void *arg){
    bool queueIsEmpty = true;
    for (;;){
        queueIsEmpty = STAILQ_EMPTY(&waitingqueue);
        if (queueIsEmpty == false){
            pthread_mutex_lock(&waitlock);
            struct queue_entry *tempEntry = queue_pop_head(&waitingqueue);
            pthread_mutex_unlock(&waitlock);
            currTaskIexec = tempEntry->data;
            swapcontext(&contextiexec, &(currTaskIexec->taskcontext));
        }
        else if (shutdownTriggered == true && numtasks <= 0){
            break;
        }
        usleep(100);
    }
}

void *cexec(void *arg){
    bool queueIsEmpty;
    for (;;){
        queueIsEmpty = STAILQ_EMPTY(&readyqueue);
        if (queueIsEmpty == false){
            pthread_mutex_lock(&readylock);
            struct queue_entry *tempEntry = queue_pop_head(&readyqueue);
            pthread_mutex_unlock(&readylock);
            currTaskCexec = tempEntry->data;
            swapcontext(&contextcexec, &(currTaskCexec->taskcontext));
        }
        else if (shutdownTriggered == true && numtasks <= 0){
            break;
        }
        usleep(100);
    }
}


void sut_init(){
	//Initialize variables
    numtasks = 0;
    shutdownTriggered = false;
    hasAquiredMsg = false;
    isConnected = false;  
    memset(sutReadMsg, 0, sizeof(sutReadMsg));

    //Creates two wait queues
    readyqueue = queue_create();
    waitingqueue = queue_create();
    queue_init(&readyqueue);
    queue_init(&waitingqueue);

    //Create two threads EXEC-C and EXEC-I
    pthread_create(&thread_handle_iexec, NULL, iexec, NULL);
    pthread_create(&thread_handle_cexec, NULL, cexec, NULL);

    return;
}


bool sut_create(sut_task_f fn){
    if(numtasks >= MAX_THREADS){
		printf("Error: Too many tasks\n");
        return false;
    }

    taskdesc *newtask;
    newtask = malloc(sizeof(taskdesc));
    currTaskCexec = (taskdesc*)malloc(sizeof(taskdesc));
    currTaskIexec = (taskdesc*)malloc(sizeof(taskdesc)); 

	getcontext(&(newtask->taskcontext));
	newtask->taskid = numtasks;
	newtask->taskstack = (char *)malloc(THREAD_STACK_SIZE);
	newtask->taskcontext.uc_stack.ss_sp = newtask->taskstack;
	newtask->taskcontext.uc_stack.ss_size = THREAD_STACK_SIZE;
	newtask->taskcontext.uc_link = 0;
	newtask->taskcontext.uc_stack.ss_flags = 0;
	newtask->taskfunc = fn;

    numtasks++;
	makecontext(&(newtask->taskcontext), fn, 0);
    
    
    //Put newly created task at end of queue
    struct queue_entry *tempEntry = queue_new_node(newtask);
    pthread_mutex_lock(&readylock);
    queue_insert_tail(&readyqueue,tempEntry);
    pthread_mutex_unlock(&readylock);
    return true;
}

void sut_yield(){
    struct queue_entry *tempEntry = queue_new_node(currTaskCexec);
    pthread_mutex_lock(&readylock);
    queue_insert_tail(&readyqueue,tempEntry);               //Put current task at end of queue
    pthread_mutex_unlock(&readylock);
    swapcontext(&(currTaskCexec->taskcontext),&contextcexec);    //Go back Exec-C context
    return;
}

void sut_exit(){
    
    ucontext_t throwawayContext;
    numtasks--;
    swapcontext(&throwawayContext,&contextcexec); //Stop current task, does not put at end of queue
    return;
}

void sut_open(char *dest, int port){
    numtasks++;
    taskdesc *newtask;
    newtask = malloc(sizeof(taskdesc));
	getcontext(&(newtask->taskcontext));
	newtask->taskid = numtasks;
	newtask->taskstack = (char *)malloc(THREAD_STACK_SIZE);
	newtask->taskcontext.uc_stack.ss_sp = newtask->taskstack;
	newtask->taskcontext.uc_stack.ss_size = THREAD_STACK_SIZE;
	newtask->taskcontext.uc_link = 0;
	newtask->taskcontext.uc_stack.ss_flags = 0;
	newtask->taskfunc = (void *)sut_open_iexec;
	makecontext(&(newtask->taskcontext), (void *)sut_open_iexec, 2, dest, port);

    struct queue_entry *tempEntry = queue_new_node(newtask);
    getcontext(&(newtask->mastercontext));                  //When sut_open_iexec comes back it comes back here
    if(isConnected == false){
        pthread_mutex_lock(&waitlock);
        queue_insert_tail(&waitingqueue,tempEntry);
        pthread_mutex_unlock(&waitlock);
        swapcontext(&(currTaskCexec->taskcontext),&contextcexec);
    }
    numtasks--;
    return;
}



void sut_write(char *buf, int size){
    numtasks++;
    taskdesc *newtask;
    newtask = malloc(sizeof(taskdesc));
	getcontext(&(newtask->taskcontext));
	newtask->taskid = numtasks;
	newtask->taskstack = (char *)malloc(THREAD_STACK_SIZE);
	newtask->taskcontext.uc_stack.ss_sp = newtask->taskstack;
	newtask->taskcontext.uc_stack.ss_size = THREAD_STACK_SIZE;
	newtask->taskcontext.uc_link = 0;
	newtask->taskcontext.uc_stack.ss_flags = 0;
	newtask->taskfunc = (void *)sut_write_iexec;
	makecontext(&(newtask->taskcontext), (void *)sut_write_iexec, 2, buf, size);

    struct queue_entry *tempEntry = queue_new_node(newtask);
    pthread_mutex_lock(&waitlock);
    queue_insert_tail(&waitingqueue,tempEntry);
    pthread_mutex_unlock(&waitlock);
    return ;
}

void sut_close(){
    numtasks++;
    taskdesc *newtask;
    newtask = malloc(sizeof(taskdesc));
	getcontext(&(newtask->taskcontext));
	newtask->taskid = numtasks;
	newtask->taskstack = (char *)malloc(THREAD_STACK_SIZE);
	newtask->taskcontext.uc_stack.ss_sp = newtask->taskstack;
	newtask->taskcontext.uc_stack.ss_size = THREAD_STACK_SIZE;
	newtask->taskcontext.uc_link = 0;
	newtask->taskcontext.uc_stack.ss_flags = 0;
	newtask->taskfunc = (void *)sut_close_iexec;
	makecontext(&(newtask->taskcontext), (void *)sut_close_iexec, 0);

    struct queue_entry *tempEntry = queue_new_node(newtask);
    pthread_mutex_lock(&waitlock);
    queue_insert_tail(&waitingqueue,tempEntry);
    pthread_mutex_unlock(&waitlock);
    return;
}


char *sut_read(){
    
    numtasks++;
    taskdesc *newtask;
    newtask = malloc(sizeof(taskdesc));
	getcontext(&(newtask->taskcontext));
	newtask->taskid = numtasks;
	newtask->taskstack = (char *)malloc(THREAD_STACK_SIZE);
	newtask->taskcontext.uc_stack.ss_sp = newtask->taskstack;
	newtask->taskcontext.uc_stack.ss_size = THREAD_STACK_SIZE;
	newtask->taskcontext.uc_link = 0;
	newtask->taskcontext.uc_stack.ss_flags = 0;
	newtask->taskfunc = (void *)sut_read_iexec;
	makecontext(&(newtask->taskcontext), (void *)sut_read_iexec, 0);

    struct queue_entry *tempEntry = queue_new_node(newtask);
    getcontext(&(newtask->mastercontext));                  //When sut_read_iexec comes back it comes back here
    if(hasAquiredMsg == false){
        pthread_mutex_lock(&waitlock);
        queue_insert_tail(&waitingqueue,tempEntry);
        pthread_mutex_unlock(&waitlock);
        swapcontext(&(currTaskCexec->taskcontext),&contextcexec);
    }
    hasAquiredMsg = false;
    numtasks--;
    return sutReadMsg;
}

void sut_shutdown(){
    shutdownTriggered = true;
    pthread_join(thread_handle_iexec,NULL);
    pthread_join(thread_handle_cexec,NULL);
    return;
}


