#ifndef RPC_H_
#define RPC_H_

#define PORT_LENGTH      256

typedef struct rpc_t{
    char name[PORT_LENGTH];
    int port;
} rpc_t;

#endif
