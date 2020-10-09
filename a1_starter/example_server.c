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


rpc_t rpc;
int sockfd;


int main(void) {

  char msg[BUFSIZE];

  const char *answer = "Standard\n";
  static int running = 0;
  pid_t pid; 
  rpc =  (rpc_t){.name = "0.0.0.0", .port = 13000};
  rpc_t *rpcPtr = RPC_Init(rpc.name, rpc.port);

  int clientfd;
  while (1){
      if (accept_connection(sockfd, &clientfd) < 0) {
        fprintf(stderr, "oh no\n");
        exit(1);
    }
    pid = fork();
    if(pid == 0){
			printf("here1\n");
      while (1) {
        ssize_t byte_count = recv_message(clientfd, msg, BUFSIZE);

        if(strcmp(msg, "quit\n")){
          close(clientfd);
          break;
        }
        else{
          printf("Client: %s\n", msg);
          send_message(clientfd, answer, strlen(answer));
          memset(msg, 0, sizeof(msg));
        }
        
      }
      exit(0);
    }
    else {
			printf("here2\n");
      close(clientfd);
			continue;
    }
  }

  close(sockfd);
  return 0;
}

rpc_t *RPC_Init(char *host, int port){
    if (create_server(host, port, &sockfd) < 0) {
        fprintf(stderr, "RPC Init error\n");
    }
    return &rpc;
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