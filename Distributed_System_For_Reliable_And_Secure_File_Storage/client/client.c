/******************************************
# Programming Assignment 3
# client.c
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
#include <openssl/md5.h>
#define MAXBUFSIZE 1024
#define SERVERS 4


typedef struct{
  char dfs[4][64];
  int port_num[4][64];
  char dfs_ip[4][64];
  char username[4][64];
  char password[4][64];
}struct_parse;

typedef struct{
  char username[64];
  char password[64];
  char command[32];
  char filename[32];
}struct_authenticate;

int parse_file(struct_parse *parse, char* conf){
  FILE *fp;
  char line[MAXBUFSIZE];
  char str[MAXBUFSIZE]={0};
  char *c;
  fp = fopen(conf, "r");
  if(!fp){
    printf("Error reading Configuration file\n");
    return -1;
  }
  else{
    while(fgets(line, sizeof(line), fp) > 0){
      //printf("%s\n", line );
      if(c = strstr(line, "DFS1")){
        sscanf(c, "%s %[^:]%*c%d", (*parse).dfs[0], (*parse).dfs_ip[0], (*parse).port_num[0]);
        printf("************\n");
        printf("%d\n", *(*parse).port_num[0]);
        printf("%s\n", (*parse).dfs[0]);
        printf("%s\n", (*parse).dfs_ip[0]);
      }

      else if(c = strstr(line, "DFS2")){
        sscanf(c, "%s %[^:]%*c%d", (*parse).dfs[1], (*parse).dfs_ip[1], (*parse).port_num[1]);
        printf("************\n");
        printf("%d\n", *(*parse).port_num[1]);
        printf("%s\n", (*parse).dfs[1]);
        printf("%s\n", (*parse).dfs_ip[1]);
      }

      else if(c = strstr(line, "DFS3")){
        sscanf(c, "%s %[^:]%*c%d", (*parse).dfs[2], (*parse).dfs_ip[2], (*parse).port_num[2]);
        printf("************\n");
        printf("%d\n", *(*parse).port_num[2]);
        printf("%s\n", (*parse).dfs[2]);
        printf("%s\n", (*parse).dfs_ip[2]);
      }

      else if(c = strstr(line, "DFS4")){
        sscanf(c, "%s %[^:]%*c%d", (*parse).dfs[3], (*parse).dfs_ip[3], (*parse).port_num[3]);
        printf("************\n");
        printf("%d\n", *(*parse).port_num[3]);
        printf("%s\n", (*parse).dfs[3]);
        printf("%s\n", (*parse).dfs_ip[3]);
      }

      else if(c = strstr(line, "Username: ")){
        sscanf(line, "%*s %s", (*parse).username[0]);
        printf("************\n");
        printf("Username is: %s\n", (*parse).username[0]);
      }

      else if(c = strstr(line, "Password: ")){
        sscanf(line, "%*s %s", (*parse).password[0]);
        printf("The password is: %s\n", (*parse).password[0]);
      }

    }
    return 0;
  }
}

/**********************************
Code by: https://stackoverflow.com/questions/10324611/how-to-calculate-the-md5-hash-of-a-large-file-in-c?newreg=957f1b2b2132420fb1b4783484823624
**********************/
char* MD5sum(char *filename){
  unsigned char hash_hex[MD5_DIGEST_LENGTH];
  int i;
  char *md5string =(char*) malloc(sizeof(char)*256);
  FILE *fp = fopen(filename, "rb");
  MD5_CTX mdContext;
  int nbytes;
  unsigned char buffer[1024];

  if(fp == NULL) {
      printf ("Error opening file for MD5SUM Calculation\n");
      return 0;
  }

  MD5_Init (&mdContext);
  while ((nbytes = fread(buffer, 1, 1024, fp)) != 0){
    MD5_Update (&mdContext, buffer, nbytes);
  }
  MD5_Final (hash_hex,&mdContext);
  for(i = 0; i < MD5_DIGEST_LENGTH; i++){
    //printf("%02x", c[i]);
    snprintf(&md5string[i*2], 32, "%02x", hash_hex[i]);
  }
  printf("\n************\n");
  printf ("Hash Value: %s\n\n", md5string);
  fclose(fp);
  return (char *)md5string;
}

int* intMD5sum(char * hash_value){
  int *hash_int = (int*)malloc(sizeof(int));
  *hash_int = strtol(hash_value+31, NULL, 16);
  printf("Integer Hash: %d\n", *hash_int );
  return (int *)hash_int;
}



int main(int argc, char * argv[]){
  int parse_status;
  struct_parse parse;
  struct_authenticate *auth = (struct_authenticate*)malloc(sizeof(struct_authenticate));
  char command[32];
  char *cname;
  char *filename;
  char* conf;
  int sockfd[4];
  struct sockaddr_in server_addr, client_addr;
  int nbytes;
  char buffer[MAXBUFSIZE];
  int x; //Modulo4 value of hash_int

  if (argc != 2)
	{
		printf ("\nUsage: <conf file>\n");
		exit(1);
	}
  conf = argv[1];
  //scanf(" %[^\n]s",conf);

  parse_status = parse_file(&(parse), conf);
  if(parse_status==-1){
    printf("Error Parsing the Configuration File\n");
    exit(1);
  }

  /* Display the availble commands for the user */
  printf("\n\tEnter Command as under: \n");
  printf("\t1. list \n" "\t2. get <filename>\n" "\t3. put <filename>\n");
  scanf(" %[^\n]s",command);	// store the command entered by the user

  cname = strdup(command);
  strtok(cname, " ");
  filename = strtok(NULL, " ");
  printf("Filename: %s\n", filename);

  char *hash_value = MD5sum(filename);
  printf ("Hash Value in Main: %s\n\n", hash_value);

  int *hash_int = intMD5sum(hash_value);
  printf ("Hash Integer Value in Main: %d\n\n", *hash_int);

  x = (*hash_int)%4;
  printf("\n************\n");
  printf("MD5HASH%%4 value: %d\n", x );



  if(!strcmp(cname, "put")){
    printf("\n************\n");
    printf("Put File: \"%s\" on the server.\n", filename);

    strcpy(auth->username, *parse.username);
    strcpy(auth->password, *parse.password);
    strcpy(auth->command, command);
    strcpy(auth->filename, filename);
    printf("Username:%sPassword:%sCommand:%sFilename:%s\n", auth->username, auth->password, auth->command, auth->filename );

    for(int i=0; i<4; i++){
      if((sockfd[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    		printf("Error in creating a socket for the Server:%s at Port: %d\n", parse.dfs[i], *parse.port_num[i]);
    	}
    	memset(&server_addr, 0, sizeof(server_addr));
    	server_addr.sin_family = AF_INET;
    	server_addr.sin_port = htons(*parse.port_num[i]);
    	server_addr.sin_addr.s_addr = inet_addr(parse.dfs_ip[i]);
      printf("\n\nSockfd %d: %d\n", i, sockfd[i]);
      printf("%d\n", *(parse).port_num[i]);
      printf("%s\n", (parse).dfs[i]);
      printf("%s\n", (parse).dfs_ip[i]);
    	if(connect(sockfd[i],(struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    	{
        perror("Error: \n");
        printf("Error in Connecting to socket for the Server:%s at Port: %d\n", parse.dfs[i], *parse.port_num[i]);
        continue;
    	}

      if((nbytes = send(sockfd[i], auth, sizeof(*auth), 0)) < 0){
        printf("%d\n", nbytes);
        printf("Error in sending to socket for the Server:%s at Port: %d\n", parse.dfs[i], *parse.port_num[i]);
      }
      printf("Nbytes Sent: %d\n", nbytes);
      bzero(buffer, MAXBUFSIZE);
      nbytes = 0;
      if((nbytes = recv(sockfd[i], buffer, sizeof(buffer), 0))<0){
        perror("Error: \n");
      }
      //for(int i=0; i<entry;i++){
      else printf("%s\n", buffer);
      //}

    }

    FILE *fp;
    fp = fopen(filename, "rb");
    if(fp == NULL){
      perror("File does nor exist or Error opening file\n");
    }

    else{
      char msg[] = "Success";
    }
  }

  return 0;
}
