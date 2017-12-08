/******************************************
# Programming Assignment 4
# webproxy.c
# Code Adapted from Professor's example-code
# Date: December 10, 2017
#Author: Anirudh Tiwari
******************************************/


//Includes
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

//MACROS
#define MAXBUFSIZE 1024
#define ERR_METHOD "<html><body><H1>Error 400 Bad Request: Method Not Supported </H1></body></html>"
#define ERR_VERSION "<html><body><H1>Error 400 Bad Request: INVALID HTTP Version </H1></body></html>"
#define ERR_SERVERNOTFOUND "<html><body><H1>Error: Server Not Found </H1></body></html>"

int nbytes;
char buffer[MAXBUFSIZE];
char filename[MAXBUFSIZE];  //Cache filename
char req_buffer[MAXBUFSIZE];
char *url_hash;


char* MD5sum(char *url){
  unsigned char hash_hex[MD5_DIGEST_LENGTH];
  int i;
  char *md5string =(char*) malloc(sizeof(char)*256);
  MD5_CTX mdContext;

  MD5_Init (&mdContext);
  MD5_Update (&mdContext, url, strlen(url));
  MD5_Final (hash_hex,&mdContext);

  for(i = 0; i < MD5_DIGEST_LENGTH; i++){
    snprintf(&md5string[i*2], 32, "%02x", hash_hex[i]);
  }

  return (char *)md5string;
}

int checkCacheFile(char *url){
  FILE *fp;
  url_hash = MD5sum(url);
  if(url_hash == 0){
    printf("Error Calculating Hash value\n");
  }
  // printf("Hash Value: %s\n", url_hash);

  bzero(buffer, sizeof(buffer));
  bzero(filename, sizeof(filename));
  sprintf(filename, "./cache/%s", url_hash);
  bzero(buffer, sizeof(buffer));

  if((fp = fopen(filename, "r")) != NULL){
    return 1;
  }
  else return 0;
}


void response(int newsockfd){
  //int newsockfd = a;
  int sockfd1;
  char method[MAXBUFSIZE];
  char url[MAXBUFSIZE];
  char version[MAXBUFSIZE];
  char ip[128];
  char port[32];
  char hostname[MAXBUFSIZE];
  struct hostent *server_hp;							// to represent entry in host database
  FILE *fp;
  struct sockaddr_in server;

  bzero(buffer, sizeof(buffer));
  bzero(req_buffer, sizeof(req_buffer));
  while((nbytes = recv(newsockfd, buffer, sizeof(buffer), 0))){
    //printf("Request Buffer:\n%s\n", buffer);
    strncpy(req_buffer, buffer, nbytes);

    bzero(method, sizeof(method));
    bzero(url, sizeof(method));
    bzero(version, sizeof(version));
    sscanf(buffer, "%s %s %s", method, url, version);
    printf("Method: %s \tURL: %s \tVersion:%s\n", method, url, version );

    if(strcmp(method, "GET") != 0){
      send(newsockfd, ERR_METHOD, strlen(ERR_METHOD), 0 );
      printf("Error:Method Not Supported\n");
      exit(1);
    }

    else if((strcmp(version, "HTTP/1.0") != 0) && (strcmp(version, "HTTP/1.1") != 0)){
      send(newsockfd, ERR_VERSION, strlen(ERR_VERSION), 0 );
      printf("Error:Invalid Version\n");
      exit(1);
    }

    else{

      int cacheFilePresent = checkCacheFile(url);

      if(cacheFilePresent == 1){
        printf("*****Page found in Cache Socket:%d*****\n", newsockfd );
        FILE *fp;
        bzero(filename, sizeof(filename));
        sprintf(filename, "./cache/%s", url_hash);
        fp = fopen(filename, "r");

        bzero(buffer, sizeof(buffer));
        while((nbytes = fread(buffer, 1, nbytes, fp))){
          send(newsockfd, buffer, nbytes, 0);
          bzero(buffer, sizeof(buffer));
        }
        fclose(fp);
        continue;
      }

      else{
        printf("*****Page Not found in Cache:%d*****\n", newsockfd);

        sscanf(url, "%*[^/]%*c%*c%[^/]", hostname);
        printf("Hostname: %s\n", hostname );

        if(strchr(hostname, ':')){
          sscanf(hostname, "%[^:]%*c%[^/]", ip, port);
          bzero(&server,sizeof(server));               //zero the struct
          server.sin_family = AF_INET;                 //address family
          server.sin_port = htons(atoi(port));      //sets port to network byte order
          server.sin_addr.s_addr = inet_addr(ip); //sets remote IP address
        }

        else{
          if(1==0){

          }
          else{
            bzero(&server,sizeof(server));               //zero the struct
            server.sin_family = AF_INET;                 //address family
            server.sin_port = htons(80);      //sets port to network byte order
            //server.sin_addr.s_addr = inet_addr(hostname); //sets remote IP address


            server_hp = gethostbyname(hostname);					 // Return information about host in argv[1]
            bcopy((char*)server_hp->h_addr, (char*)&server.sin_addr, server_hp->h_length);
            if(server_hp < 0){
              send(newsockfd, ERR_SERVERNOTFOUND, strlen(ERR_SERVERNOTFOUND), 0 );
              printf("Error:Server Not Found\n");
              exit(1);
            }
            else{
              sprintf(filename, "./cache/hosts");
              fp = fopen(filename, "ab");

              bzero(buffer, sizeof(buffer));
              sprintf(buffer, "%s %s\n", hostname, inet_ntoa(server.sin_addr));
              fwrite(buffer, 1, strlen(buffer), fp);
              fclose(fp);
            }
          }

        }

        /***** Creating the socket*****/
        if ((sockfd1 = socket(AF_INET, SOCK_STREAM, 0)) < 0){
          printf("Error creating socket at proxy \n");
          exit(1);
        }

        if((connect(sockfd1, (struct sockaddr *)&server, sizeof(server))) < 0){
          printf("Error in Connect to the server. \n");
          exit(1);
        }

        send(sockfd1, req_buffer, strlen(req_buffer), 0);

        bzero(buffer, sizeof(buffer));
        bzero(filename, sizeof(filename));
        sprintf(filename, "./cache/%s", url_hash);

        fp = fopen(filename, "ab");
        if(fp < 0){
          printf("Error Creating Cache file\n");
          exit(1);
        }
        bzero(buffer, sizeof(buffer));
        while((nbytes = recv(sockfd1, buffer, sizeof(buffer), 0))){
          //printf("Received Bytes: %d\n", nbytes);
          //printf("Buffer Recieved: %s\n", buffer );

          send(newsockfd, buffer, nbytes, 0);
          fwrite(buffer, 1, nbytes, fp);
          //printf("\n\n Loop\n");
          bzero(buffer, sizeof(buffer));
        }
        // printf("\n\n Loop Exit\n");
      }
    }
  }
}


/*****Main Function *****/
int main(int argc, char* argv[]){
  int sockfd;
  struct sockaddr_in client_addr, proxy_addr;
  unsigned int length_client = sizeof(client_addr);
  int newsockfd;
  int pid;

  if (argc != 2)
  {
    printf ("\nUsage: <portNo>\n");
    exit(1);
  }

/***** Creating the socket*****/
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    printf("Error creating socket \n");
    exit(1);
  }


  bzero(&proxy_addr, sizeof(proxy_addr));
  proxy_addr.sin_family = AF_INET;
  proxy_addr.sin_port = htons(atoi(argv[1]));
  proxy_addr.sin_addr.s_addr = INADDR_ANY;

/*****binding the socket *****/
  if(bind(sockfd, (struct sockaddr *)&proxy_addr, sizeof(proxy_addr)) < 0){
	   printf("unable to bind socket\n");
     exit(1);
  }
  printf("**********Waiting for New Connection**********\n\n");

/*****Waiting for new connections *****/
  if(listen(sockfd, 1024) < 0){
    printf("*******Error in Listen*******\n");
    exit(1);
  }

  while(1){
    /***** Accepting new functions*****/
    if((newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &length_client)) < 0){
      printf("Error in accept. \n");
      exit(1);
    }
    else{
      /**** FORK for handling multiple clients ****/
      pid = fork();

      if(pid<0){
        printf("Error in Fork\n");
      }

      if(pid==0){
        printf("**********New Connection at Port %d - Socket : %d**********\n\n", atoi(argv[1]), newsockfd);

        response(newsockfd);

        close(newsockfd);
        exit(0);
      }
    }
  }
  return 0;
}
