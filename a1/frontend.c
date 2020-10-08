#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "rpc.h"
#include "a1_lib.h"

#define BUFSIZE   1024


rpc_t *RPC_Connect(char *name, int port){
    rpc_t *newRPC;
    if (connect_to_server(*name, port, &(newRPC->sockfd)) < 0) {
        fprintf(stderr, "Connect To Server Error\n");
        return -1;
    }
    return newRPC;
}


int main(void) {
  //int sockfd;
  char user_input[BUFSIZE] = { 0 };
  char server_msg[BUFSIZE] = { 0 };

  /*
  if (connect_to_server("0.0.0.0", 10000, &sockfd) < 0) {
    fprintf(stderr, "oh no\n");
    return -1;
  }
  */
  rpc_t *currentRPC; 
  currentRPC = RPC_Connect("0.0.0.0", 10000);

  //backend = RPC_Connect(backendIP, backendPort)
  //while(no_exit) {
    //print_prompt()
    //line = read_line()cmd = parse_line(line)//
    //RPC_Call(backend, cmd.name, cmd.args)
 // }
 //RPC_Close(backend)


  //Old Example Code
  
  while (strcmp(user_input, "quit\n")) {
    memset(user_input, 0, sizeof(user_input));
    memset(server_msg, 0, sizeof(server_msg));

    // read user input from command line
    fgets(user_input, BUFSIZE, stdin);
    // send the input to server
    send_message(currentRPC->sockfd, user_input, strlen(user_input));
    // receive a msg from the server
    ssize_t byte_count = recv_message(currentRPC->sockfd, server_msg, sizeof(server_msg));
    if (byte_count <= 0) {
      break;
    }
    printf("Server: %s\n", server_msg);
  }
  

  return 0;
}

