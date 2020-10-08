#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "rpc.h"
#include "a1_lib.h"



rpc_t *RPC_Init(char *host, int port){
    rpc_t *rpcPtr, currentRPC;
    if (create_server(host, port, &(currentRPC.sockfd)) < 0) {
        fprintf(stderr, "oh no\n");
        return NULL;
    }

    if (accept_connection(currentRPC.sockfd, &(currentRPC.clientfd)) < 0) {
        fprintf(stderr, "oh no\n");
        return NULL;
    }
    
    return rpcPtr;
}


rpc_t *RPC_Connect(char *name, int port){
    rpc_t *newRPC;
    if (connect_to_server(*name, port, &(newRPC->sockfd)) < 0) {
        fprintf(stderr, "Connect To Server Error\n");
        return -1;
    }
    return newRPC;
} //Initializes connection return backend


/*
RPC_Close(rpc_t *r) //Closes backend, rpc_t should be a backend reference

RPC_Call(rpc_t *r, char *name, args..) //This is the command (message)

RPC_Register(“add”, addInts); // Adds to a list the function pointer and the function call
*/

//RPC_Register(rpc_t *r, char *name, callback_t fn) // Adds to a list the function pointer and the function call
// callback_t is type defined by you
