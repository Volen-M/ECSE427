#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h> 
#include <sys/wait.h> 

#include "a1_lib.h"
#include "rpc.h"

#define BUFSIZE   1024

rpc_t *RPC_Init(char *host, int port);
bool RPC_Call(char *msg, char* answer);
void RPC_Close();

int addInts(int a, int b);
int multiplyInts(int a, int b);
float divideFloats(float a, float b);
uint64_t factorial(int x);
int subInts(int a, int b);


rpc_t rpc;
int sockfd; //Not a fan of making it a global but given assignment specification 
            //demands of RPC_Connect/RPC_Close I had to
pid_t pids[5];

int main(int argc, char *argv[]) {
  rpc =  (rpc_t){.name = "0.0.0.0", .port = 18000};
  
  if (argc != 3){
    printf("There is the wrong amount of args: %i\n", argc);
    printf("Using standard host ip \"%s\" and port \"%i\"\n", rpc.name,rpc.port);
  }
  else{
    rpc =  (rpc_t){.name = argv[1], .port = (int)strtol(argv[2], (char **)NULL, 10)};
  }
  printf("Server listening on %s:%i\n",rpc.name,rpc.port);

  char msg[BUFSIZE];
  char answer[BUFSIZE];

  const char *rdyMsg = ">>"; 

  pid_t pid; 
  rpc_t *rpcPtr = RPC_Init(rpc.name, rpc.port);

  signal(SIGQUIT, RPC_Close);



  int clientfd;
  for (;;){
    if (accept_connection(sockfd, &clientfd) < 0) {
        fprintf(stderr, "oh no\n");
        
        exit(1);
    }

    pid = fork();
    printf("pid %i\n", pid);
    if(pid == 0){
      ssize_t byte_count;
      for(;;) {
        memset(msg, 0, sizeof(msg));
        byte_count = recv_message(clientfd, msg, BUFSIZE);
        if(byte_count <0){
          close(clientfd);
          break;
        }
        if(strcmp(msg, "exit\n") == 0){
          sprintf(answer, "Bye! Front End Closed\n");
          send_message(clientfd, answer, strlen(answer));
          close(clientfd);
          exit(0);
          break;
        }
        else if(strcmp(msg, "shutdown\n")==0 || strcmp(msg, "quit\n")==0 ){
          sprintf(answer, "Bye! Server Shutdown\n");
          send_message(clientfd, answer, strlen(answer));
          memset(msg, 0, sizeof(msg));
          
          kill(pid, SIGQUIT);
          close(clientfd);
          close(sockfd);
          return 0;
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
      /*for(;;){
        int rval;
        int res = waitpid(-1, &rval, WNOHANG);
        printf("Returned value %d\n", WEXITSTATUS(rval));
      }*/
      close(clientfd);
			continue;
    }
  }

  return 0;
}

void RPC_Close(){
  close(sockfd);
  printf("Reaches closure");
  kill(-1*getppid(), SIGTERM);
  return;
}

//Returns boolean which shows success or failure of call
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

    //I avoid including a full library "Math.h" by adding an extra comparison step here
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
    sprintf(answer, "Has finished sleeping for %i seconds",int1);
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


//Really useless return argument, but it was demanded
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

  //given it's a float would never get here
  return 0;
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