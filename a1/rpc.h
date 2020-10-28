#ifndef RPC_H_
#define RPC_H_

#define PORT_LENGTH      45 //45 bit length due to IPv6

typedef struct rpc_t{
    char name[PORT_LENGTH]; //Cannot make this dynamic, given it is in struct as demanded
    int port;
} rpc_t;

#endif //RPC_H_
