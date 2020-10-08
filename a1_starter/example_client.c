#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "a1_lib.h"
#include "rpc.h"

#define BUFSIZE   1024

rpc_t *RPC_Connect(char *name, int port);

rpc_t rpcCurrent;

int main(void) {
  
  char user_input[BUFSIZE] = { 0 };
  char server_msg[BUFSIZE] = { 0 };
  
  rpcCurrent = (rpc_t){ .sockfd =0, .clientfd = 0}; 
  rpc_t *rpcPtr = RPC_Connect("0.0.0.0",10000);


  while (strcmp(user_input, "quit\n")) {
    memset(user_input, 0, sizeof(user_input));
    memset(server_msg, 0, sizeof(server_msg));

    // read user input from command line
    fgets(user_input, BUFSIZE, stdin);
    // send the input to server
    send_message(rpcPtr->sockfd, user_input, strlen(user_input));
    // receive a msg from the server
    ssize_t byte_count = recv_message(rpcPtr->sockfd, server_msg, sizeof(server_msg));
    if (byte_count <= 0) {
      break;
    }
    printf("Server: %s\n", server_msg);
  }

  return 0;
}

rpc_t *RPC_Connect(char *name, int port){
    if (connect_to_server(name, port, &(rpcCurrent.sockfd)) < 0) {
        fprintf(stderr, "Connect To Server Error\n");
        return -1;
    }
    return &rpcCurrent;
} //Initializes connection return backend