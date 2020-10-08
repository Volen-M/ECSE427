#include <stdio.h>
#include <string.h>

#include "a1_lib.h"
#include "rpc.h"

#define BUFSIZE   1024

rpc_t *RPC_Init(char *host, int port);

int addInts(int a, int b);
int multiplyInts(int a, int b);
float divideFloats(float a, float b);
uint64_t factorial(int x);
int subInts(int a, int b);


rpc_t rpcCurrent;

int main(void) {
  char msg[BUFSIZE];

  const char *answer = "Standard Answer\n";
  int running = 1;

  rpcCurrent = (rpc_t){ .sockfd =0, .clientfd = 0}; 
  rpc_t *rpcPtr = RPC_Init("0.0.0.0", 10000);


  while (strcmp(msg, "quit\n")) {
    memset(msg, 0, sizeof(msg));
    ssize_t byte_count = recv_message(rpcPtr->clientfd, msg, BUFSIZE);
    if (byte_count <= 0) {
      break;
    }
    printf("Client: %s\n", msg);
    send_message(rpcPtr->clientfd, answer, strlen(answer));
  }

  return 0;
}

rpc_t *RPC_Init(char *host, int port){
    if (create_server(host, port, &(rpcCurrent.sockfd)) < 0) {
        fprintf(stderr, "oh no\n");
        return NULL;
    }

    if (accept_connection(rpcCurrent.sockfd, &(rpcCurrent.clientfd)) < 0) {
        fprintf(stderr, "oh no\n");
        return NULL;
    }
    
    return &rpcCurrent;
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

uint64_t factorial(int x){
  int i = 1;
  int sum = 0;
  for(i=1;i<=x;i++){
    sum*=x;
  }
  return sum;
}