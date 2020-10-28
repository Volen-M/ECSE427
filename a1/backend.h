#ifndef BACKEND_H_
#define BACKEND_H_

#include "rpc.h"
#include "a1_lib.h"
/**
 *  Initializes a server.
 *
 *  @params:
 *    host:     A string that is the host address of the server.
 *    port:     An integer in the range [0, 65536) that is the port.
 *  @return:    (Useless but demanded) On success, the file returns the name and port as struct
 */
rpc_t *RPC_Init(char *host, int port);

/**
 *  Calls out the appropriate function to complete given msg argument
 *
 *  @params:
 *    msg:      A string that is the command. It modifies it by the end leaving only the command
 *    answer:   A string that will contain the final message/answer
 *  @return:    A boolean showing success or failure of execution
 */
bool RPC_Call(char *msg, char* answer);

/**
 *  Immediate shutdown (To be called after shutdown signal has been received) 
 *
 *  @return:    void
 */
void RPC_Close();

/**
 *  Simple Integer adder
 *
 *  @params:
 *    a:       First integer to add
 *    b:       Second integer to add
 *  @return:   Integer sum result
 */
int addInts(int a, int b);

/**
 *  Simple Integer Multiplier
 *
 *  @params:
 *    a:       Integer Multiplicand
 *    b:       Integer Multiplier
 *  @return:   Integer product result
 */
int multiplyInts(int a, int b);

/**
 *  Simple float divider (does not do division by 0 error checking)
 *
 *  @params:
 *    a:       Float dividend
 *    b:       Float divisor
 *  @return:   Float quotient result
 */
float divideFloats(float a, float b);

/**
 *  Simple factorial function (no overflow checking)
 *
 *  @params:
 *    x:       Integer number that will be factorialed (x!)
 *  @return:   Unsigned 64bit int result
 */
uint64_t factorial(int x);


#endif //BACKEND_H_