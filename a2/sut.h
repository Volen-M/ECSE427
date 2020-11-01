#ifndef __SUT_H__
#define __SUT_H__
#include <stdbool.h>

#define MAX_THREADS                        32
#define THREAD_STACK_SIZE                  1024*64
#define BUFSIZE                            1024

typedef void (*sut_task_f)();
/**
 *  Pauses the execution of a thread until it is selected again by the scheduler
 *  It is put back in task ready queue
 *
 *  @return:    Void
 */
void sut_init();

/**
 *  Creates task given C function.
 *
 *  @params:
 *    fn:     A C function pointer.
 *  @return:    Boolean showing if task creation is successful (only trigger if over MAX_THREADS limit)
 */
bool sut_create(sut_task_f fn);

/**
 *  Pauses the execution of a thread until it is selected again by the scheduler
 *  It is put back in task ready queue
 *
 *  @return:    Void
 */
void sut_yield();

/**
 *  Terminates task execution. Does not put task back in task ready queue
 *
 *  @return:    Void
 */
void sut_exit();

/**
 *  Reads data from an external process without stalling the executor. 
 *  It sends request to request queue and the task itself is put in a wait queue until response arrives. 
 * 
 *  @params:
 *    *dest:  Destination pointer of data to be read.
 *    port:   Port.
 *  @return:    Void
 */
void sut_open(char *dest, int port);

/**
 *  Non blocking data write. It wrates data to remote process
 *  No acknowledgement of the sent data needed.
 * 
 *  @params:
 *    *buf:  Buffer where data is to be written
 *    size:  Port.
 *  @return:    Void
 */
void sut_write(char *buf, int size);

/**
 *  Pauses the execution of a thread until it is selected again by the scheduler
 *
 *  @return:    Void
 */
void sut_close();

/**
 *  Reads data from an external process without stalling the executor. 
 *  It sends request to request queue and the task itself is put in a wait queue until response arrives. 
 * 
 *  @return:  Read Data
 */
char *sut_read();

/**
 *   Cleanly "shuts" down the thread library
 *
 *  @return:    Void
 */
void sut_shutdown();


#endif
