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

int parse_file(struct_parse *parse){
  FILE *fp;
  char line[MAXBUFSIZE];
  char str[MAXBUFSIZE]={0};
  char *c;
  char *split;
  int entry=0;
  fp = fopen("dfc.conf", "r");
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
  char md5string[256];
  FILE *inFile = fopen (filename, "rb");
  MD5_CTX mdContext;
  int bytes;
  unsigned char data[1024];

  if (inFile == NULL) {
      printf ("%s can't be opened.\n", filename);
      return 0;
  }

  MD5_Init (&mdContext);
  while ((bytes = fread (data, 1, 1024, inFile)) != 0)
      MD5_Update (&mdContext, data, bytes);
  MD5_Final (hash_hex,&mdContext);
  for(i = 0; i < MD5_DIGEST_LENGTH; i++){
    //printf("%02x", c[i]);
    snprintf(&md5string[i*2], 32, "%02x", hash_hex[i]);
  }
  printf("\n************\n");
  printf ("Hash Value: %s\n\n", md5string);
  fclose (inFile);
  return 0;
}

int main(){
  int parse_status;
  struct_parse parse;
  char command[32];
  char *cname;
  char *filename;

  parse_status = parse_file(&(parse));
  if(parse_status==-1){
    printf("Error Parsing the Configuration File\n");
    exit(1);
  }

  /* Display the availble commands for the user */
  printf("\n\tEnter Command as under: \n");
  printf("\t1. list \n" "\t2. get <filename>\n" "\t3. put <filename>\n");
  gets(command);	// store the command entered by the user

  cname = strdup(command);
  strtok(cname, " ");
  filename = strtok(NULL, " ");
  printf("Filename: %s\n", filename);

  char* hash_value = MD5sum(filename);
  if(!strcmp(cname, "put")){
    printf("Put File: \"%s\" on the server.\n", filename);

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
