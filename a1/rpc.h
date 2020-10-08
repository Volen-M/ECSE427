#ifndef RPC_H_
#define RPC_H_

/*
#define CMD_LENGTH      256
#define ARGS_LENGTH     256

typedef struct message_t{
    char cmd[CMD_LENGTH];
    char args[ARGS_LENGTH];
} message_t;
*/
typedef struct rpc_t{
    int sockfd;
    int clientfd;
} rpc_t;

/*
rpc_t *RPC_Init(char *host, int port);
rpc_t *RPC_Connect(char *name, int port);
*/
// rpc_t is a type defined by you and it holds
// all necessary state, config about the RPC conne;ction
//RPC_Register(rpc_t *r, char *name, callback_t fn);
// callback_t is type defined by you
//RPC_Close(rpc_t *r);
//RPC_Call(rpc_t *r, char *name, args..);
// You can have different variations to
// handle different number of parameters and types

#endif