#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "rpc.h"
#include "backend.h"
#include "a1_lib.h"


#define BUFSIZE   1024

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

int main(void) {
  int sockfd, clientfd;
  char msg[BUFSIZE];
  const char *greeting = "hello, world\n";
  int running = 1;


  rpc_t *serv = RPC_Init("0.0.0.0", 1000);
/*
  if (create_server("0.0.0.0", 10000, &sockfd) < 0) {
    fprintf(stderr, "oh no\n");
    return -1;
  }

  if (accept_connection(sockfd, &clientfd) < 0) {
    fprintf(stderr, "oh no\n");
    return -1;
  }
  */
/*
  rpc_t *serv = RPC_Init(myIP, myPort);
  for_all_functions(name)
  RPC_Register(serv, name, function)
  while(no_shutdown) {
    client = accept_on_server_socket(serv)
    serv_client(client)
  }
  */


  //--Example Code
  
  while (strcmp(msg, "quit\n")) {
    memset(msg, 0, sizeof(msg));
    ssize_t byte_count = recv_message(serv->clientfd, msg, BUFSIZE);
    if (byte_count <= 0) {
      break;
    }
    printf("Client: %s\n", msg);
    send_message(serv->clientfd, greeting, strlen(greeting));
  }

  return 0;
}

int addInts(int x, int y) {
    return x + y;
}

int subInts(int a, int b){
  return a-b;
}


int multiplyInts(int a, int b){
  return a*b;
}

float divideFloats(float a, float b){
  if(b!= 0){
    return a/b;
  }
  exit(EXIT_FAILURE);
}

/*
int sleep(int x){
  unistdsleep(x);
  return EXIT_SUCCESS;
}
*/


uint64_t factorial(int x){
  int i = 1;
  int sum = 0;
  for(i=1;i<=x;i++){
    sum*=x;
  }
  return sum;
}
// return factorial x
