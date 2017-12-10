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
#include <time.h>

//MACROS
#define MAXBUFSIZE 1024
#define ERR_METHOD "HTTP/1.1 400 Bad Request\n\rContent-Type: text/html\nContent-Length: %d\n\r\n<html><body><H1>Error 400 Bad Request: Method Not Supported </H1></body></html>"
#define ERR_VERSION "HTTP/1.1 400 Bad Request\n\rContent-Type: text/html\nContent-Length: %d\n\r\n<html><body><H1>Error 400 Bad Request: Invalid HTTP Version </H1></body></html>"
#define ERR_SERVERNOTFOUND "HTTP/1.1 400 Bad Request\n\rContent-Type: text/html\nContent-Length: %d\n\r\n<html><body><H1>Error 400 Bad Request: Server Not Found </H1></body></html>"
#define ERR_BLOCKED "HTTP/1.1 403 Forbidden\n\rContent-Type: text/html\nContent-Length: %d\n\r\n<html><body>ERROR 403 Forbidden</body></html>"


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

int checkCacheFile(char *url, unsigned long int timeout){
  FILE *fp;
  char *url_hash = MD5sum(url);
  char filename[MAXBUFSIZE];
  char* line=NULL;
  size_t length;
  unsigned long int fileCreationTime=0;
  unsigned long int exp_time;
  time_t current_time;

  current_time = time(NULL);

  if(url_hash == 0){
    printf("Error Calculating Hash value\n");
  }
  // printf("Hash Value: %s\n", url_hash);

  bzero(filename, sizeof(filename));
  sprintf(filename, "./cache/%s", url_hash);
  printf("Filename in CacheFileCheck: %s\n", filename );


  if((fp = fopen(filename, "r")) != NULL){
    getline(&line, &length, fp);
    sscanf(line, "%lu", &fileCreationTime);

    exp_time = current_time - fileCreationTime;
    //printf("Timeout: %lu, FileCreation: %lu, CurrentTime: %lu, Expiry:%lu %s", timeout, fileCreationTime, current_time, exp_time, url_hash);
    if(exp_time < timeout){
      fclose(fp);
      return 1;
    }
    else{
      fclose(fp);
      remove(filename);
      return 0;
    }
  }
  else return 0;
}

int checkCacheHost(char *hostname, char *ip){
  FILE* fp;
  char* line=NULL;
  size_t length;
  char filename[MAXBUFSIZE];
  int flag=0;
  //printf("****************In checkCacheHost***************\n");

  bzero(filename, sizeof(filename));
  sprintf(filename, "./cache/hosts");

  if((fp = fopen(filename, "r")) == NULL){
    //fclose(fp);
    //printf("****************In checkCacheHost File Open***************\n");
    return 0;
  }
  else{
    while((getline(&line, &length, fp)) != -1){
      if(strstr(line, hostname)){
        sscanf(line, "%*[^ ]%*c%s", ip);
        flag =1;
        break;
      }
    }
    if(flag==1){
      fclose(fp);
      return 1;
    }
    fclose(fp);
    return 0;
  }
}

int checkForbiddenHost(char *hostname, char *forbid_ip){
  FILE * fp;
  char* line=NULL;
  size_t length;
  int flag = 0;
  if((fp = fopen("forbidden", "r")) != NULL){

    if(strchr(hostname, ':')){
      sscanf(hostname, "%[^:]%*c", forbid_ip);
      while((getline(&line, &length, fp)) != -1){
        if(strstr(line, forbid_ip)){
          flag =1;
          break;
        }
      }
      if(flag==1){
        fclose(fp);
        return 1;
      }
      fclose(fp);
      return 0;
    }

    else{
      while((getline(&line, &length, fp)) != -1){
        if(strstr(line, hostname)){
          flag =1;
          break;
        }
      }
      if(flag==1){
        fclose(fp);
        return 1;
      }
      fclose(fp);
      return 0;
    }
  }
  return 0;
}

int linkPrefetch(char* prefetch_ip, char* filename, char* hostname ){
  FILE * fp1;
  FILE * fp2;
  char* line = NULL;
  size_t length;
  char* url_full;
  char url[MAXBUFSIZE];
  char url_inc[MAXBUFSIZE]; //URL Incomplete
  char server_req[MAXBUFSIZE];
  char buffer[MAXBUFSIZE];
  int nbytes = 0;
  char* url_hash = NULL;
  char cacheFilename[64];
  struct sockaddr_in server_add_prefetch;
  int sockfd2;
  char * ret_strstr = NULL;

  bzero(&server_add_prefetch,sizeof(server_add_prefetch));               //zero the struct
  server_add_prefetch.sin_family = AF_INET;                 //address family
  server_add_prefetch.sin_port = htons(80);      //sets port to network byte order
  server_add_prefetch.sin_addr.s_addr = inet_addr(prefetch_ip); //sets remote IP address

  if((fp1 = fopen(filename, "r")) != NULL){

    while((getline(&line, &length, fp1) != -1)){
      bzero(cacheFilename, sizeof(cacheFilename));
      bzero(url, sizeof(url));
      bzero(url_inc, sizeof(url_inc));
      bzero(server_req, sizeof(server_req));
      bzero(buffer, sizeof(buffer));

      if((ret_strstr = strstr(line, "href"))){
        if ((sockfd2 = socket(AF_INET, SOCK_STREAM, 0)) < 0){
          printf("Error creating socket at proxy \n");
          exit(1);
        }

        if((connect(sockfd2, (struct sockaddr *)&server_add_prefetch, sizeof(server_add_prefetch))) < 0){
          printf("Error in Connect to the server. \n");
          exit(1);
        }

        if((url_full = strstr(line, "http://"))){
          sscanf(url_full, "%[^\"]", url);
        }
        else{
          sscanf(ret_strstr, "%*[^=]%*c%*c%[^\"]", url_inc );
          if(url_inc[0] == '/'){
            sprintf(url, "http://%s%s", hostname, url_inc);
          }
          else{
            sprintf(url, "http://%s/%s", hostname, url_inc);
          }
        }
        sprintf(server_req, "GET %s HTTP/1.0\r\n\r\n", url);

        url_hash = MD5sum(url);

        sprintf(cacheFilename, "./cache/%s", url_hash );
        send(sockfd2, server_req, strlen(server_req), 0);

        fp2 = fopen(cacheFilename, "ab");
        if(fp2 < 0){
          printf("Error Creating Cache file in Prefetch\n");
          exit(1);
        }
        time_t current_time2;
        current_time2 = time(NULL);
        //printf("*******Current time1: %lu*******\n", current_time1 );
        fprintf(fp2, "%lu\n", current_time2);

        while((nbytes = recv(sockfd2, buffer, sizeof(buffer), 0))){
          printf("Prefetch Link Received Bytes: %d\n", nbytes);
          //printf("Buffer Recieved: %s\n", buffer );
          fwrite(buffer, 1, nbytes, fp2);
          //printf("\n\n Loop\n");
          bzero(buffer, sizeof(buffer));
        }
        // printf("\n\n Loop Exit\n");
        fclose(fp2);
        printf("######Link Prefetched: %s\tSockedFD2: %d\n Filename:%s", url, sockfd2, cacheFilename );
      }
    }
    fclose(fp1);
    return 0;
  }
  else{
    return 1;
  }

}


void response(int newsockfd, unsigned long int timeout){
  //int newsockfd = a;
  char filename[MAXBUFSIZE];
  int sockfd1;
  char method[MAXBUFSIZE];
  char url[MAXBUFSIZE];
  char version[MAXBUFSIZE];
  char ip[128] = "";
  char forbid_ip[128] = "";
  char port[32];
  char hostname[MAXBUFSIZE];
  struct hostent *server_hp;							// to represent entry in host database
  FILE *fp;
  struct sockaddr_in server;
  int nbytes;
  char buffer[MAXBUFSIZE];
  char req_buffer[MAXBUFSIZE];
  char *url_hash;
  char* line=NULL;
  size_t length;
  int flag = 0;
  int pid2;

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

    url_hash = MD5sum(url);

    if(strcmp(method, "GET") != 0){
      bzero(buffer, sizeof(buffer));
      sprintf(buffer, ERR_METHOD, (int)strlen("<html><body><H1>Error 400 Bad Request: Method Not Supported </H1></body></html>"));
      printf("Error Buffer\n%s\n", buffer);
      nbytes = send(newsockfd, buffer, strlen(buffer), 0 );
      continue;
    }

    else if((strcmp(version, "HTTP/1.0") != 0) && (strcmp(version, "HTTP/1.1") != 0)){
      bzero(buffer, sizeof(buffer));
      sprintf(buffer, ERR_VERSION, (int)strlen("HTTP/1.1 400 Bad Request\n\rContent-Type: text/html\nContent-Length: %d\n\r\n<html><body><H1>Error 400 Bad Request: Invalid HTTP Version </H1></body></html>"));
      printf("Error Buffer\n%s\n", buffer);
      nbytes = send(newsockfd, buffer, strlen(buffer), 0 );
      continue;
    }

    else{
      sscanf(url, "%*[^/]%*c%*c%[^/]", hostname);
      printf("Hostname: %s\n", hostname );

      int checkForbidden = checkForbiddenHost(hostname, forbid_ip);

      if(checkForbidden == 1){
        bzero(buffer, sizeof(buffer));
        sprintf(buffer, ERR_BLOCKED, (int)strlen("<html><body>ERROR 403 Forbidden</body></html>"));
        printf("Error Buffer\n%s\n", buffer);
        nbytes = send(newsockfd, buffer, strlen(buffer), 0 );
        continue;
      }

      int cacheFilePresent = checkCacheFile(url, timeout);

      if(cacheFilePresent == 1){
        printf("\n*****Page found in Cache Socket:%d*****\n", newsockfd );
        FILE *fp;
        bzero(filename, sizeof(filename));
        sprintf(filename, "./cache/%s", url_hash);
        fp = fopen(filename, "r");
        getline(&line, &length, fp);

        bzero(buffer, sizeof(buffer));
        while((nbytes = fread(buffer, 1, sizeof(buffer), fp))){
          send(newsockfd, buffer, nbytes, 0);
          bzero(buffer, sizeof(buffer));
        }
        fclose(fp);
        continue;
      }

      else{
        printf("\n*****Page Not found in Cache:%d*****\n", newsockfd);

        if(strchr(hostname, ':')){
          sscanf(hostname, "%[^:]%*c%[^/]", ip, port);
          bzero(&server,sizeof(server));               //zero the struct
          server.sin_family = AF_INET;                 //address family
          server.sin_port = htons(atoi(port));      //sets port to network byte order
          server.sin_addr.s_addr = inet_addr(ip); //sets remote IP address
        }

        else{
          int checkHostPresent = checkCacheHost(hostname, ip);

          if(checkHostPresent==1){
            printf("\n*******Host Present in Cache*******\n");

            bzero(filename, sizeof(filename));
            sprintf(filename, "./cache/hosts");
            //fp = fopen(filename, "ab");

            bzero(&server,sizeof(server));               //zero the struct
            server.sin_family = AF_INET;                 //address family
            server.sin_port = htons(80);      //sets port to network byte order
            server.sin_addr.s_addr = inet_addr(ip); //sets remote IP address
          }
          else{
            printf("\n*******Host Not Present in Cache*******\n");
            bzero(&server,sizeof(server));               //zero the struct
            server.sin_family = AF_INET;                 //address family
            server.sin_port = htons(80);      //sets port to network byte order
            //server.sin_addr.s_addr = inet_addr(hostname); //sets remote IP address


            server_hp = gethostbyname(hostname);					 // Return information about host in argv[1]
            bcopy((char*)server_hp->h_addr, (char*)&server.sin_addr, server_hp->h_length);
            if(server_hp < 0){
              bzero(buffer, sizeof(buffer));
              sprintf(buffer, ERR_SERVERNOTFOUND, (int)strlen("<html><body><H1>Error 400 Bad Request: Server Not Found </H1></body></html>"));
              printf("Error Buffer\n%s\n", buffer);
              nbytes = send(newsockfd, buffer, strlen(buffer), 0 );
              continue;
            }
            else{
              bzero(filename, sizeof(filename));
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
          continue;
        }

        if((connect(sockfd1, (struct sockaddr *)&server, sizeof(server))) < 0){
          printf("Error in Connect to the server. \n");
          continue;
        }

        send(sockfd1, req_buffer, strlen(req_buffer), 0);

        bzero(buffer, sizeof(buffer));
        bzero(filename, sizeof(filename));
        sprintf(filename, "./cache/%s", url_hash);
        //printf("*********Filename in CacheFileCreate: %s***********\n", filename );

        fp = fopen(filename, "ab");
        if(fp < 0){
          printf("Error Creating Cache file\n");
          exit(1);
        }
        time_t current_time1;
        current_time1 = time(NULL);
        //printf("*******Current time1: %lu*******\n", current_time1 );
        fprintf(fp, "%lu\n", current_time1);

        bzero(buffer, sizeof(buffer));
        while((nbytes = recv(sockfd1, buffer, sizeof(buffer), 0))){
          //printf("Received Bytes: %d\n", nbytes);
          //printf("Buffer Recieved: %s\n", buffer );
          if(strstr(buffer, "<html")){
            flag = 1;
          }

          send(newsockfd, buffer, nbytes, 0);
          fwrite(buffer, 1, nbytes, fp);
          //printf("\n\n Loop\n");
          bzero(buffer, sizeof(buffer));
        }
        // printf("\n\n Loop Exit\n");
        fclose(fp);

        if(flag==1){
          pid2 = fork();

          if(pid2 == 0){
            int ret = linkPrefetch(inet_ntoa(server.sin_addr), filename, hostname );
            exit(0);
          }
        }
      }
    }
  }
}


/*****Main Function *****/
int main(int argc, char* argv[]){
  int sockfd;
  unsigned long int timeout;
  struct sockaddr_in client_addr, proxy_addr;
  unsigned int length_client = sizeof(client_addr);
  int newsockfd;
  int pid;

  if (argc != 3)
  {
    printf ("\nUsage: <portNo> <timeout>\n");
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
        timeout = atoi(argv[2]);
        response(newsockfd, timeout);

        close(newsockfd);
        exit(0);
      }
    }
  }
  return 0;
}
