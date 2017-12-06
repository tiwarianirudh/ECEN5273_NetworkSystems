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
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>

//MACROS
#define MAXBUFSIZE 1024
#define ERR_METHOD "<html><body><H1>Error 400 Bad Request: Method Not Supported </H1></body></html>"
#define ERR_VERSION "<html><body><H1>Error 400 Bad Request: INVALID HTTP Version </H1></body></html>"
#define ERR_SERVERNOTFOUND "<html><body><H1>Error: Server Not Found </H1></body></html>"


// //client handler for multiple requests
// void *client_handler(void* arg){
//
// }




/*****Main Function *****/
int main(int argc, char* argv[]){
  int sockfd, sockfd1;
  int nbytes;
  char buffer[MAXBUFSIZE];
  struct sockaddr_in server, client_addr, proxy_addr;
  unsigned int length_client = sizeof(client_addr);
  int newsockfd, newsockfd1;
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
      if(pid==0){
        printf("**********New Connection at Port %d - Socket : %d**********\n\n", atoi(argv[1]), newsockfd);

        char method[MAXBUFSIZE];
        char url[MAXBUFSIZE];
        char version[MAXBUFSIZE];
        char hostname[MAXBUFSIZE];
        struct hostent *server_hp;							// to represent entry in host database

        bzero(buffer, sizeof(buffer));
        recv(newsockfd, buffer, sizeof(buffer), 0);
        printf("Request Buffer:\n%s\n", buffer);

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
          sscanf(url, "%*[^/]%*c%*c%[^/]", hostname);
          printf("Hostname: %s\n", hostname );

          bzero(&server,sizeof(server));               //zero the struct
        	server.sin_family = AF_INET;                 //address family
        	server.sin_port = htons(80);      //sets port to network byte order
        	//server.sin_addr.s_addr = inet_addr(hostname); //sets remote IP address

          server_hp = gethostbyname(hostname);					 // Return information about host in argv[1]
        	if(server_hp < 0){
            send(newsockfd, ERR_SERVERNOTFOUND, strlen(ERR_SERVERNOTFOUND), 0 );
            printf("Error:Server Not Found\n");
            exit(1);
        	}

        	bcopy((char*)server_hp->h_addr, (char*)&server.sin_addr, server_hp->h_length);

          /***** Creating the socket*****/
          if ((sockfd1 = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            printf("Error creating socket at proxy \n");
            exit(1);
          }

          if((connect(sockfd1, (struct sockaddr *)&server, sizeof(server))) < 0){
            printf("Error in Connect to the server. \n");
            exit(1);
          }

          send(sockfd1, buffer, strlen(buffer), 0);

          bzero(buffer, sizeof(buffer));

          while((nbytes = recv(sockfd1, buffer, sizeof(buffer), 0))) {
          printf("Received Bytes: %d\n", nbytes);
          printf("Buffer Recieved: %s\n", buffer );
          send(newsockfd, buffer, nbytes, 0);
          printf("\n\n Loop\n");
          bzero(buffer, sizeof(buffer));
          }
          printf("\n\n Loop Exit\n");
        }
      }
    }
  }
  return 0;
}
