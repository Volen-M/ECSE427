#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "a1_lib.h"
#include "rpc.h"

#define BUFSIZE   1024

int sockfd;
rpc_t rpc;

rpc_t *RPC_Connect(char *name, int port);


int main(void) {
  
  char user_input[BUFSIZE] = { 0 };
  char server_msg[BUFSIZE] = { 0 };
  rpc =  (rpc_t){.name = "0.0.0.0", .port = 13000};
  rpc_t *rpcPtr = RPC_Connect(rpc.name, rpc.port);


  while (strcmp(user_input, "quit\n")) {
    memset(user_input, 0, sizeof(user_input));
    memset(server_msg, 0, sizeof(server_msg));

    // read user input from command line
    fgets(user_input, BUFSIZE, stdin);
    // send the input to server
    send_message(sockfd, user_input, strlen(user_input));
    // receive a msg from the server
    ssize_t byte_count = recv_message(sockfd, server_msg, sizeof(server_msg));
    if (byte_count <= 0) {
      break;
    }
    printf("Server: %s\n", server_msg);
  }

  return 0;
}

rpc_t *RPC_Connect(char *name, int port){
    if (connect_to_server(name, port, &sockfd) < 0) {
        fprintf(stderr, "Connect To Server Error\n");
        return -1;
    }
    return &rpc;
} //Initializes connection return backend