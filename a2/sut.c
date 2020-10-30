#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <ucontext.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "sut.h"
#include "queue.h"

typedef struct _taskdesc{
	int taskid;
    int *sockfd;
	char *taskstack;
	void *taskfunc;
	ucontext_t taskcontext;
}taskdesc;


int numtasks;
bool shutdownTriggered;

struct queue readyqueue;
struct queue waitingqueue;
taskdesc *currTaskCexec;
taskdesc *currTaskIexec;
pthread_t thread_handle_iexec;
pthread_t thread_handle_cexec;



ucontext_t contextcexec, contextiexec;



void *iexec(void *arg){
    // printf("Launched I-Exec Kernel thread\n");
    //pthread_mutex_t *lock = arg;
    bool queueIsEmpty = true;
    for (;;){
        // printf("I-Exec running homie\n");
        queueIsEmpty = STAILQ_EMPTY(&waitingqueue);
        if (queueIsEmpty == false){
            // pthread_mutex_lock(lock);
            struct queue_entry *tempEntry = queue_pop_head(&waitingqueue);
            currTaskIexec = tempEntry->data;
            swapcontext(&contextiexec, &(currTaskIexec->taskcontext));
            // pthread_mutex_unlock(lock);
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
        // printf("C-Exec running homie\n");
        queueIsEmpty = STAILQ_EMPTY(&readyqueue);
        if (queueIsEmpty == false){
            // pthread_mutex_lock(lock);
            struct queue_entry *tempEntry = queue_pop_head(&readyqueue);
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

    // pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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
    currTaskCexec = malloc(sizeof(taskdesc));
    currTaskIexec = malloc(sizeof(taskdesc)); 

	getcontext(&(newtask->taskcontext));
	newtask->taskid = numtasks;
	newtask->taskstack = (char *)malloc(THREAD_STACK_SIZE);
	newtask->taskcontext.uc_stack.ss_sp = newtask->taskstack;
	newtask->taskcontext.uc_stack.ss_size = THREAD_STACK_SIZE;
	newtask->taskcontext.uc_link = 0;
	newtask->taskcontext.uc_stack.ss_flags = 0;
	newtask->taskfunc = fn;
    newtask->sockfd = -1;

    // printf("Here\n");
	makecontext(&(newtask->taskcontext), fn, 0);
    
    
    // //Put newly created task at end of queue
    struct queue_entry *tempEntry = queue_new_node(newtask);
    queue_insert_tail(&readyqueue,tempEntry);
    numtasks++;
    // printf("Here\n");
    return true;
}

void sut_yield(){
    //Put current task at end of queue
    //Go back Exec-C context
    struct queue_entry *tempEntry = queue_new_node(currTaskCexec);
    queue_insert_tail(&readyqueue,tempEntry);
    swapcontext(&(currTaskCexec->taskcontext),&contextcexec);
    return;
}

void sut_exit(){
    //Stop current taks do not put at end of queue
    // printf("Sut Exit Enter\n");
    ucontext_t throwawayContext;
    numtasks--;
    swapcontext(&throwawayContext,&contextcexec);
    // printf("Sut Exit End\n");
    return;
}

void sut_open(char *dest, int port){
    printf("Sut Open Enter\n");
    struct queue_entry *tempEntry = queue_new_node(currTaskCexec);
    queue_insert_tail(&waitingqueue,tempEntry);
    swapcontext(&(currTaskCexec->taskcontext),&contextcexec);
    printf("Sut Open Enter Through I-Exec\n");
    struct sockaddr_in server_address = { 0 };

    // create a new socket
    int socketfd= socket(AF_INET, SOCK_STREAM, 0);
    *(currTaskIexec->sockfd) = socketfd;
    if (*(currTaskIexec->sockfd) < 0) {
        perror("Failed to create a new socket\n");
        return;
    }
    printf("Here\n");
    // connect to server
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, dest, &(server_address.sin_addr.s_addr));
    server_address.sin_port = htons(port);
    if (connect(*(currTaskIexec->sockfd), (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Failed to connect to server\n");
        return;
    }
    printf("Sut Open Exiting\n");
    return;
}

void sut_write(char *buf, int size){
    struct queue_entry *tempEntry = queue_new_node(currTaskCexec);
    queue_insert_tail(&waitingqueue,tempEntry);
    swapcontext(&(currTaskCexec->taskcontext),&contextcexec);
    send(*(currTaskIexec->sockfd), buf, size, 0);
    return ;
}

void sut_close(){
    struct queue_entry *tempEntry = queue_new_node(currTaskCexec);
    queue_insert_tail(&waitingqueue,tempEntry);
    swapcontext(&(currTaskCexec->taskcontext),&contextcexec);
    close(*(currTaskIexec->sockfd));
    return;
}

char *sut_read(){
    //Socket data read in I-exec until socket wait then it goes to back of queue until ready

    struct queue_entry *tempEntry = queue_new_node(currTaskCexec);
    queue_insert_tail(&waitingqueue,tempEntry);
    swapcontext(&(currTaskCexec->taskcontext),&contextcexec);

    ssize_t byte_count;
    char msg[BUFSIZE];
    int clientfd;
    // if (accept_connection(*(currTaskCexec->sockfd), &clientfd) < 0) {
    //     fprintf(stderr, "oh no\n");
    //     exit(1);
    // }
    for(;;) {
        memset(msg, 0, sizeof(msg));
        byte_count = recv((*currTaskIexec->sockfd), msg, BUFSIZE, 0);
        if (byte_count>0){
            return msg;
        }
        else{
            tempEntry = queue_new_node(currTaskIexec);
            queue_insert_tail(&waitingqueue,tempEntry);
            swapcontext(&(currTaskIexec->taskcontext),&contextiexec);
        }
    }
    return msg;
}

void sut_shutdown(){
    shutdownTriggered = true;
    pthread_join(thread_handle_iexec,NULL);
    pthread_join(thread_handle_cexec,NULL);
    printf("Shutting Down\n");
    return;
}


