#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "a1_lib.h"
#include "rpc.h"

#define BUFSIZE   1024

rpc_t *RPC_Init(char *host, int port);
bool RPC_Call(char *msg, char* answer);

int addInts(int a, int b);
int multiplyInts(int a, int b);
float divideFloats(float a, float b);
uint64_t factorial(int x);
int subInts(int a, int b);


rpc_t rpc;
int sockfd;


int main(int argc, char *argv[]) {
  rpc =  (rpc_t){.name = "0.0.0.0", .port = 13000};
  
  if (argc != 3){
    printf("There is the wrong amount of args: %i\n", argc);
    printf("Using standard host ip \"0.0.0.0\" and port \"13000\"\n");
  }
  else{
    rpc =  (rpc_t){.name = argv[1], .port = (int)strtol(argv[2], (char **)NULL, 10)};
  }
  printf("Server listening on %s:%s\n",argv[1],argv[2]);

  char msg[BUFSIZE];
  const char *rdyMsg = ">>"; 
  static bool hasReceivedShutdown = false;

  char answer[BUFSIZE];
  static int running = 0;
  pid_t pid; 
  rpc_t *rpcPtr = RPC_Init(rpc.name, rpc.port);




  int clientfd;
  for (;;){
    if (accept_connection(sockfd, &clientfd) < 0) {
        fprintf(stderr, "oh no\n");
        
        exit(1);
    }
    if(hasReceivedShutdown == true){
      if (running ==0){
        printf("Shutdown Procedure Initiated\n");
        close(clientfd);
        close(sockfd);
        return 0; 
      }
      close(clientfd);
    }
    pid = fork();
    if(pid == 0){
      running++;
      ssize_t byte_count;
      for(;;) {
        memset(msg, 0, sizeof(msg));
        byte_count = recv_message(clientfd, msg, BUFSIZE);
        if(byte_count <0){
          running--;
          close(clientfd);
          break;
        }
        if(strcmp(msg, "quit\n") == 0){
          running--;
          sprintf(answer, "Bye!\n");
          send_message(clientfd, answer, strlen(answer));
          close(clientfd);
          break;
        }
        else if(strcmp(msg, "shutdown\n")==0 || strcmp(msg, "exit\n")==0 ){
          hasReceivedShutdown = true;
          running--;
          close(clientfd);
          break;
        }
        else{
          printf("Client: %s\n", msg);
          RPC_Call(msg, answer);
          sprintf(answer, "%s\n>>", answer);
          send_message(clientfd, answer, strlen(answer));
          //send_message(clientfd, rdyMsg, strlen(rdyMsg));
        }
        
      }
      running--;
      exit(0);
    }
    else {
      close(clientfd);
			continue;
    }
  }

  close(sockfd);
  return 0;
}
/*
bool RPC_Call(char *msg, char* answer){
  return true;
}
*/

bool RPC_Call(char* msg, char* answer){
  char delimiter[] = " ";
  char *ptr = strtok(msg, delimiter);
  

  if (ptr == NULL){
    sprintf(answer, "Null command has been passed");
    return false;
  }

  if (strcmp(ptr,"add")==0){
    ptr = strtok(NULL, delimiter);
    if (ptr  == NULL){
      sprintf(answer, "Error: Missing Arguments");
      return false;
    }
    int int1 = (int)strtol(ptr, (char **)NULL, 10);
    ptr = strtok(NULL, delimiter);
    if (ptr  == NULL){
      sprintf(answer, "Error: Missing Arguments");
      return false;
    }
    int int2 = (int)strtol(ptr, (char **)NULL, 10);
    int int3 = addInts(int1,int2);
    sprintf(answer,"%i",int3);
    ptr = strtok(NULL, delimiter);
    if (ptr  != NULL){
      sprintf(answer, "Error: Too Many Arguments");
      return false;
    }
  }
  else if (strcmp(ptr,"multiply")==0){
    ptr = strtok(NULL, delimiter);
    if (ptr  == NULL){
      sprintf(answer, "Error: Missing Arguments");
      return false;
    }
    int int1 = (int)strtol(ptr, (char **)NULL, 10);
    ptr = strtok(NULL, delimiter);
    if (ptr  == NULL){
      sprintf(answer, "Error: Missing Arguments");
      return false;
    }
    int int2 = (int)strtol(ptr, (char **)NULL, 10);
    int int3 = multiplyInts(int1,int2);
    sprintf(answer,"%i",int3);
    ptr = strtok(NULL, delimiter);
    if (ptr  != NULL){
      sprintf(answer, "Error: Too Many Arguments");
      return false;
    }
  }
  else if (strcmp(ptr,"divide")==0){
    ptr = strtok(NULL, delimiter);
    if (ptr  == NULL){
      sprintf(answer, "Error: Missing Arguments");
      return false;
    }
    float float1 = (float)strtof(ptr, (char **)NULL);
    ptr = strtok(NULL, delimiter);
    if (ptr  == NULL){
      sprintf(answer, "Error: Missing Arguments");
      return false;
    }
    float float2 = (float)strtof(ptr, (char **)NULL);
    //printf("%s - %f", ptr, float2);
    if(float2 <= 10 * __FLT_EPSILON__ && float2 >= -10 * __FLT_EPSILON__ ){
      sprintf(answer, "Error: Division by Zero");
      return false;
    }
    float float3 = divideFloats(float1,float2);
    sprintf(answer,"%f",float3);
    ptr = strtok(NULL, delimiter);
    if (ptr  != NULL){
      sprintf(answer, "Error: Too Many Arguments");
      return false;
    }
  }
  else if (strcmp(ptr,"factorial")==0){
    ptr = strtok(NULL, delimiter);
    if (ptr  == NULL){
      sprintf(answer, "Error: Missing Arguments");
      return false;
    }
    int int1 = (int)strtol(ptr, (char **)NULL, 10);
    uint64_t int2 = factorial(int1);
    sprintf(answer,"%lu",int2);
    ptr = strtok(NULL, delimiter);
    if (ptr  != NULL){
      sprintf(answer, "Error: Too Many Arguments");
      return false;
    }
  }
  else if (strcmp(ptr,"sleep")==0){ 
    ptr = strtok(NULL, delimiter);
    if (ptr  == NULL){
      sprintf(answer, "Error: Missing Arguments");
      return false;
    }
    int int1 = (int)strtol(ptr, (char **)NULL, 10);
    sleep(int1);
  }
  else {
    ptr = strtok(ptr, "\n");
    if (ptr != NULL){
      sprintf(answer, "Error: Command \"%s\" not found", ptr);
    }
    else{
      sprintf(answer, "Error: Command \"NULL\" not found");
    }
  }
  return true;
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
  if (x <= 0){
    return 0;
  }
  int i = 1;
  uint64_t sum = 1;
  for(i=1;i<=x;i++){
    sum*=i;
  }
  return sum;
}