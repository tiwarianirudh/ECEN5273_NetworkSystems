/******************************************
# Programming Assignment 3
# server.c
# Code Adapted from Professor's example-code
# Date: November 12, 2017
#Author: Anirudh Tiwari
******************************************/

/***** INCLUDES *****/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <dirent.h>
#define MAXBUFSIZE 1024

typedef struct{
  char username[4][64];
  char password[4][64];
}struct_parse;

typedef struct{
  char username[64];
  char password[64];
  char command[32];
  char filename[32];
}struct_authenticate;

int parse_file(struct_parse *parse, char* conf, int* entry);


int parse_file(struct_parse *parse, char* conf, int* entry){
  FILE *fp;
  char line[MAXBUFSIZE];

  fp = fopen(conf, "r");
  if(!fp){
    printf("Error reading Configuration file\n");
    return -1;
  }
  else{
    while(fgets(line, sizeof(line), fp) > 0){
        sscanf(line, "%s %s", (*parse).username[*entry], (*parse).password[*entry]);
        printf("******From Conf File******\n");
        printf("Username is: %s", (*parse).username[*entry]);
        printf("The password is: %s\n", (*parse).password[*entry]);
        (*entry)++;
        //printf("entry in parse%d\n", entry );
    }
    return 0;
  }
}

int main(int argc, char * argv[]){
  int parse_status;
  struct_parse parse;
  struct_authenticate *auth = (struct_authenticate*)malloc(sizeof(struct_authenticate));
  int sockfd;
  char port_num[32]= {0};
  struct sockaddr_in server_addr, client_addr;
  int newsockfd;
  unsigned int length_client = sizeof(client_addr);
  char buffer[MAXBUFSIZE];
  int nbytes;
  int* entry = (int*)malloc(sizeof(int));

  if (argc != 2)
	{
		printf ("\nUsage: <conf file>\n");
		exit(1);
	}
  strcpy(port_num, argv[1]);
  char *conf = "dfs.conf";
  parse_status = parse_file(&(parse), conf, entry);
  if(parse_status==-1){
    printf("Error Parsing the Configuration File\n");
    exit(1);
  }
printf("%d\n", *entry);

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    printf("Error creating socket \n");
  }

  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(atoi(port_num));
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

/*****binding the socket *****/
  if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
	   printf("unable to bind socket\n");
     exit(-1);
  }

  printf("\n\n**********Waiting for New Connection**********\n\n");
  if(listen(sockfd, 1024) < 0){
    printf("*******Error in Listen*******\n");
  }

  while(1){
    /***** Accepting new functions*****/
    if((newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &length_client)) < 0){
      printf("Error in accept. \n");
      exit(-1);
    }
    printf("**********New Connection at Port %s - Socket : %d**********\n\n", port_num, newsockfd);
    recv(newsockfd, auth, sizeof(*auth), 0);
    //for(int i=0; i<entry;i++){
      printf("Username in struct_auth recieved: %s\t\t Password: %s\n\n", &auth->username[0], &auth->password[0] );
    //}
    for(int i=0; i<*entry; i++){
      if(!(strcmp((parse).username[i], &auth->username[0]))){
        if(!(strcmp((parse).password[i], &auth->password[0]))){
          bzero(buffer, MAXBUFSIZE);
          strcpy(buffer, "User Exists");
          if((nbytes = send(newsockfd, buffer, strlen(buffer), 0)) < 0){
            printf("exists nbytes%d\n", nbytes);
            perror("Error: \n");
          }
          break;
        }
        else{
          bzero(buffer, MAXBUFSIZE);
          strcpy(buffer, "password error");
          if((nbytes = send(newsockfd, buffer, strlen(buffer), 0)) < 0){
            printf("error nbytes%d\n", nbytes);
            perror("Error: \n");

          }
          break;
        }
      }
      else if(i == *entry-1){
        bzero(buffer, MAXBUFSIZE);
        strcpy(buffer, "Invalid User: User Does not exist");
        if((nbytes = send(newsockfd, buffer, strlen(buffer), 0)) < 0){
          printf("invalid nbytes%d\n", nbytes);
          perror("Error: \n");
        }
        break;
      }
    }
    printf("Yaha tak to aa gaya\n");
  }
  return 0;
}
