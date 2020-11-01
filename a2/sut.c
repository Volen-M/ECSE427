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



int numtasks = 0;
bool shutdownTriggered;
pthread_mutex_t waitlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t readylock = PTHREAD_MUTEX_INITIALIZER;

struct queue readyqueue;
struct queue waitingqueue;
taskdesc *currTaskCexec;
taskdesc *currTaskIexec;
pthread_t thread_handle_iexec;
pthread_t thread_handle_cexec;



ucontext_t contextcexec, contextiexec;

int sockfd = -1;
bool isConnected = false;
void sut_open_iexec(char *dest, int port){
    // printf("Sut Open Enter Through I-Exec\n");
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
    for(;;){
        if (connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
            //isConnected = false;
            //perror("Failed to connect to server\n");
            struct queue_entry *tempEntry = queue_new_node(currTaskIexec);
            tempEntry = queue_new_node(currTaskIexec);
            pthread_mutex_lock(&waitlock);
            queue_insert_tail(&waitingqueue,tempEntry);
            pthread_mutex_unlock(&waitlock);
            swapcontext(&(currTaskIexec->taskcontext),&contextiexec);
        }
        else{
            isConnected = true;
            printf("Connected to server\n");
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

void sut_write_iexec(char *buf, int size){
    printf("Sut Write Enter Through I-Exec\n");
     for(;;){
        // printf("Sut_write\n");
        if (send(sockfd, buf, size, MSG_DONTWAIT)==-1){
            struct queue_entry *tempEntry = queue_new_node(currTaskIexec);
            tempEntry = queue_new_node(currTaskIexec);
            pthread_mutex_lock(&waitlock);
            queue_insert_tail(&waitingqueue,tempEntry);
            pthread_mutex_unlock(&waitlock);
            swapcontext(&(currTaskIexec->taskcontext),&contextiexec);
        }
        else{
            printf("break\n");
            break;
        }
    }
    numtasks--;
    // printf("Sut Write Exiting\n");
    swapcontext(&(currTaskIexec->taskcontext),&contextiexec);
}

char sutReadMsg[BUFSIZE];
bool hasAquiredMsg = false;
void sut_read_iexec(){
    printf("Read\n");

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
            printf("Leaving read connection loop\n");
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
    // printf("Launched I-Exec Kernel thread\n");
    //pthread_mutex_t *lock = arg;
    bool queueIsEmpty = true;
    for (;;){
        // printf("I-Exec running homie: %d\n", numtasks);
        queueIsEmpty = STAILQ_EMPTY(&waitingqueue);
        if (queueIsEmpty == false){
            pthread_mutex_lock(&waitlock);
            struct queue_entry *tempEntry = queue_pop_head(&waitingqueue);
            pthread_mutex_unlock(&waitlock);
            currTaskIexec = tempEntry->data;
            swapcontext(&contextiexec, &(currTaskIexec->taskcontext));
            // printf("back in iexec\n");
        }
        else if (shutdownTriggered == true && numtasks <= 0){
            break;
        }
        usleep(100);
    }
}

void *cexec(void *arg){
    // printf("Launched C-Exec Kernel thread\n");
    //pthread_mutex_t *lock = arg;
    bool queueIsEmpty;
    for (;;){
        // printf("C-Exec running homie: %d\n", numtasks);
        queueIsEmpty = STAILQ_EMPTY(&readyqueue);
        if (queueIsEmpty == false){
            // pthread_mutex_lock(lock);
            pthread_mutex_lock(&readylock);
            struct queue_entry *tempEntry = queue_pop_head(&readyqueue);
            pthread_mutex_unlock(&readylock);
            currTaskCexec = tempEntry->data;
            swapcontext(&contextcexec, &(currTaskCexec->taskcontext));
            // pthread_mutex_unlock(lock);
        }
        else if (shutdownTriggered == true && numtasks <= 0){
            break;
        }
        usleep(100);
    }
}


void sut_init(){
    //Create two wait queues
    //Create two threads EXEC-C and EXEC-I
    printf("Initializing Sut\n");
	numtasks = 0;
    shutdownTriggered = false;

    // 

    readyqueue = queue_create();
    waitingqueue = queue_create();
    queue_init(&readyqueue);
    queue_init(&waitingqueue);


    pthread_create(&thread_handle_iexec, NULL, iexec, NULL);
    pthread_create(&thread_handle_cexec, NULL, cexec, NULL);
    printf("Done Initializing Sut\n");

    return;
}


bool sut_create(sut_task_f fn){
    // printf("Creating task %d\n",numtasks);
    if(numtasks >= MAX_THREADS){
		printf("Error: Too many tasks\n");
        return -1;
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

    // printf("Here\n");
    numtasks++;
	makecontext(&(newtask->taskcontext), fn, 0);
    
    
    // //Put newly created task at end of queue
    struct queue_entry *tempEntry = queue_new_node(newtask);
    pthread_mutex_lock(&readylock);
    queue_insert_tail(&readyqueue,tempEntry);
    pthread_mutex_unlock(&readylock);
    // printf("Here\n");
    return true;
}

void sut_yield(){
    //Put current task at end of queue
    //Go back Exec-C context
    struct queue_entry *tempEntry = queue_new_node(currTaskCexec);
    pthread_mutex_lock(&readylock);
    queue_insert_tail(&readyqueue,tempEntry);
    pthread_mutex_unlock(&readylock);
    swapcontext(&(currTaskCexec->taskcontext),&contextcexec);
    return;
}

void sut_exit(){
    //Stop current task do not put at end of queue
    // printf("Sut Exit Enter\n");
    ucontext_t throwawayContext;
    numtasks--;
    swapcontext(&throwawayContext,&contextcexec);
    // printf("Sut Exit End\n");
    return;
}

void sut_open(char *dest, int port){
    //printf("Sut Open Enter\n");
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
    getcontext(&(newtask->mastercontext));
    if(isConnected == false){
        pthread_mutex_lock(&waitlock);
        queue_insert_tail(&waitingqueue,tempEntry);
        pthread_mutex_unlock(&waitlock);
        swapcontext(&(currTaskCexec->taskcontext),&contextcexec);
    }
    //swapcontext(&(newtask->taskcontext),&contextcexec);
    numtasks--;
    return;
}



void sut_write(char *buf, int size){
    // printf("Creating write function\n");
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
    //Socket data read in I-exec until socket wait then it goes to back of queue until ready
    
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
    getcontext(&(newtask->mastercontext));
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
    printf("Shutting Down\n");
    return;
}


