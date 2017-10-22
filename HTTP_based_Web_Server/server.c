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
#define MAXBUFSIZE 1024
#define NUMBEROFELEMENT 64
#define SIZEOFELEMENTS 64
#define INDEXRESPONSE "%s 200 Ok\n\rContent-Type: text/html; charset=UTF-8\n\rContent-Length: %d\r"
#define INDEXRESPONSE2 "\n\n"
#define OKRESPONSE "%s 200 Ok\n\rContent-Type: %s; charset=UTF-8\n\rContent-Length: %d\n\r\n"
//void *client_handler(void*);
//int parse_file(struct_req*);

typedef struct{
  char req_method[16];
  char req_url[256];
  char req_version[16];
}struct_req;

typedef struct{
  int port_num;
  char doc_root[256];
  char content_type[NUMBEROFELEMENT][SIZEOFELEMENTS];
  char file_ext[NUMBEROFELEMENT][SIZEOFELEMENTS];
}struct_parse;

typedef struct{
  int thread_id;
  struct_parse p_struct;
}struct_attr;

void *client_handler(void* arg){
  struct_attr attr = *(struct_attr*) arg;
  int newsockfd = attr.thread_id;
  struct sockaddr_in server_addr, client_addr;
  int nbytes;
  char buffer[MAXBUFSIZE];
  char *c;
  char filepath[MAXBUFSIZE];
  strcpy(filepath, attr.p_struct.doc_root);
  int length_client = sizeof(client_addr);
  struct_req* c_pckt = malloc(sizeof(struct_req));
  FILE *fp;
  int i=0;
  char* line;

  //while(1){
    //filepath = strdup(attr.p_struct.doc_root);
    bzero(buffer, MAXBUFSIZE);
    if((nbytes = read(newsockfd, buffer, sizeof(buffer))) < 0){
      printf("Error: Reading from the socket\n");
      close(newsockfd);
      pthread_exit(NULL);
    }
    printf("Socket Id: %d\n", newsockfd);
    printf("Buffer data with request: %s\n", buffer);
    if((line = strtok(buffer, "\n")) == NULL){
      printf("Error: Parsing the request\n");
      close(newsockfd);
      pthread_exit(NULL);
    }
    else sscanf(line, "%s %s %s", c_pckt->req_method, c_pckt->req_url, c_pckt->req_version);
    char header[MAXBUFSIZE];
    strcpy(header, c_pckt->req_version);
    printf("%s\n", header );

    if(!strcmp(c_pckt->req_url, "/")){
      strcat(filepath, "/index.html");
    }
    else{
      strcat(filepath, c_pckt->req_url);
    }
    printf("%s\n",filepath);
    if((fp = fopen(filepath, "rb")) == NULL){
      printf("Error opening file or it does not exist.\n");
      bzero(buffer, MAXBUFSIZE);
      strcpy(buffer,"HTTP/1.1 404 Not Found\n\rContent-Type: text/html; charset=UTF-8\n\r\n\r");
      printf("%s\n", buffer );
      if(nbytes = write(newsockfd, buffer, sizeof(buffer)) < 0){
        printf("Error: Writing to the socket\n");
      }
      close(newsockfd);
      pthread_exit(NULL);
    }

    fseek(fp, 0, SEEK_END);
    int n = ftell(fp);
    if(fseek(fp, 0, SEEK_SET) != 0 ) {
      printf("Error Repositioning the File Pointer to the start\n");
      close(newsockfd);
      pthread_exit(NULL);
    }
    //bzero(filepath, 256);
    printf("File Length:%d\n", n);

    if(!strcmp(c_pckt->req_method, "GET")){
      if(!strcmp(c_pckt->req_url, "/")){

        sprintf(buffer, INDEXRESPONSE, header, n, INDEXRESPONSE2);

        printf("%s", buffer);
      }

      else /*(c = strstr(c_pckt->req_url, ".js"))*/{
        while(i<NUMBEROFELEMENT){
          if(strstr(filepath, attr.p_struct.file_ext[i])!=NULL){
            //  printf("%d\n", i);
            break;
          }
        i++;
        }
        char str1[64];
        sscanf((attr.p_struct.content_type[i]), "%s", str1);
        printf("%s\n", str1 );
        sprintf(buffer, OKRESPONSE, header, str1, n);
        printf("%s\n", buffer);
      }

      // else if(c = strstr(c_pckt->req_url, ".css")){
      //   while(i<NUMBEROFELEMENT){
      //     if((c = strstr(attr.p_struct.file_ext[i], ".css"))==NULL){
      //         i++;
      //       //  printf("%d\n", i);
      //         continue;
      //     }
      //     else break;
      //   }
      //   char str1[64];
      //   sscanf((attr.p_struct.content_type[i]), "%s", str1);
      //   printf("%s\n", str1 );
      //   sprintf(buffer, OKRESPONSE, header, str1, n);
      //   printf("%s\n", buffer);
      // }
      //
      // else if(c = strstr(c_pckt->req_url, ".png")){
      //   while(i<NUMBEROFELEMENT){
      //     if((c = strstr(attr.p_struct.file_ext[i], ".png"))==NULL){
      //         i++;
      //       //  printf("%d\n", i);
      //         continue;
      //     }
      //     else break;
      //   }
      //   char str1[64];
      //   sscanf((attr.p_struct.content_type[i]), "%s", str1);
      //   printf("%s\n", str1 );
      //   sprintf(buffer, OKRESPONSE, header, str1, n);
      //   printf("%s\n", buffer);
      // }
      // else if(c = strstr(c_pckt->req_url, ".gif")){
      //   while(i<NUMBEROFELEMENT){
      //     if((c = strstr(attr.p_struct.file_ext[i], ".gif"))==NULL){
      //         i++;
      //       //  printf("%d\n", i);
      //         continue;
      //     }
      //     else break;
      //   }
      //   char str1[64];
      //   sscanf((attr.p_struct.content_type[i]), "%s", str1);
      //   printf("%s\n", str1 );
      //   sprintf(buffer, OKRESPONSE, header, str1, n);
      //   printf("%s\n", buffer);
      // }
      //
      // else if(c = strstr(c_pckt->req_url, ".txt")){
      //   while(i<NUMBEROFELEMENT){
      //     if((c = strstr(attr.p_struct.file_ext[i], ".txt"))==NULL){
      //         i++;
      //       //  printf("%d\n", i);
      //         continue;
      //     }
      //     else break;
      //   }
      //   char str1[64];
      //   sscanf((attr.p_struct.content_type[i]), "%s", str1);
      //   printf("%s\n", str1 );
      //   sprintf(buffer, OKRESPONSE, header, str1, n);
      //   printf("%s\n", buffer);
      // }
      //
      // else if(c = strstr(c_pckt->req_url, ".htm")){
      //   while(i<NUMBEROFELEMENT){
      //     if((c = strstr(attr.p_struct.file_ext[i], ".htm"))==NULL){
      //         i++;
      //       //  printf("%d\n", i);
      //         continue;
      //     }
      //     else break;
      //   }
      //   char str1[64];
      //   sscanf((attr.p_struct.content_type[i]), "%s", str1);
      //   printf("%s\n", str1 );
      //   sprintf(buffer, OKRESPONSE, header, str1, n);
      //   printf("%s\n", buffer);
      // }
      //
      // else if(c = strstr(c_pckt->req_url, ".ico")){
      //   while(i<NUMBEROFELEMENT){
      //     if((c = strstr(attr.p_struct.file_ext[i], ".ico"))==NULL){
      //         i++;
      //       //  printf("%d\n", i);
      //         continue;
      //     }
      //     else break;
      //   }
      //   char str1[64];
      //   sscanf((attr.p_struct.content_type[i]), "%s", str1);
      //   printf("%s\n", str1 );
      //   sprintf(buffer, OKRESPONSE, header, str1, n);
      //   printf("%s\n", buffer);
      // }
      //
      // else if(c = strstr(c_pckt->req_url, ".jpg")){
      //   while(i<NUMBEROFELEMENT){
      //     if((c = strstr(attr.p_struct.file_ext[i], ".jpg"))==NULL){
      //         i++;
      //       //  printf("%d\n", i);
      //         continue;
      //     }
      //     else break;
      //   }
      //   char str1[64];
      //   sscanf((attr.p_struct.content_type[i]), "%s", str1);
      //   printf("%s\n", str1 );
      //   sprintf(buffer, OKRESPONSE, header, str1, n);
      //   printf("%s\n", buffer);
      // }
      if(nbytes = write(newsockfd, buffer, strlen(buffer)) < 0){
        printf("Error: Writing to the socket\n");
      }
    }
    do{
      bzero(buffer, MAXBUFSIZE);
      int read_length = fread(buffer, 1, MAXBUFSIZE, fp);
      //printf("%s\n", buffer );
      printf("Read Length of file requested:%d\n", read_length );
      if(nbytes = write(newsockfd, buffer, sizeof(buffer)) < 0){
        printf("Error: Writing to the socket\n");
        fseek(fp, (-1)*sizeof(buffer), SEEK_CUR);
      }
      if(read_length != MAXBUFSIZE){
        break;
      }
    }while(1);
    fclose(fp);

    //else printf("Out of Main if\n" );
  //}
  //shutdown(newsockfd, 0);
  close(newsockfd);
  pthread_exit(NULL);
//  memset(c_pckt, 0, sizeof(struct_req));
}

int parse_file(struct_parse *parse){
  FILE *fp;
  char line[MAXBUFSIZE];
  char str[MAXBUFSIZE]={0};
  char *c;
  char *split;
  int entry=0;
  fp = fopen("ws.conf", "r");
  if(!fp){
    printf("Error reading Configuration file\n");
    return -1;
  }
  else{
    while(fgets(line, sizeof(line), fp) > 0){
      //printf("%s\n", line );

      if(c = strstr(line, "Listen")){
        split = strtok(line, "Listen ");
        (*parse).port_num = atoi(split);
        printf("%d\n", (*parse).port_num );
      }

      else if(c = strstr(line, "DocumentRoot")){
        //printf("%lu", strlen(line));
        strncpy(str, line+14, strlen(line)-16);
        //printf("%lu", strlen(line));
        str[strlen(str)] = '\0';
        strcpy((*parse).doc_root, str);

        //printf("doc_root: %s\n", (*parse).doc_root);
        //printf("%lu\n", strlen((*parse).doc_root));
      }

      else if(line[0]=='.'){
        sscanf(line, "%s %s", (*parse).file_ext[entry], (*parse).content_type[entry]);
        //printf("Content Type: %s \tFile Ext: %s \n",(*parse).content_type[entry], (*parse).file_ext[entry]);
        entry++;
      }
    }
    //printf("Content Type: %s %lu \tFile Ext: %s %lu\n",(*parse).content_type, strlen((*parse).content_type), (*parse).file_ext, strlen((*parse).file_ext));

    return 0;
  }
}

int main(int argc, char* argv[]){
  int sockfd;
  int parse_status;
  int nbytes;
  char buffer[MAXBUFSIZE];
  struct sockaddr_in server_addr, client_addr;
  int length_client = sizeof(client_addr);
  int newsockfd;
  int rc;
  int thread_num=1;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_t threads;
  //unsigned int thread_id;
  struct_parse parse;
  struct_attr struct_thread;

  // if(argc < 2){
  //   printf("Error! <port> \n");
  //   exit(1);
  // }

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    printf("Error creating socket \n");
  }

  parse_status = parse_file(&(parse));
  if(parse_status==-1){
    printf("Error Parsing the Configuration File\n");
    exit(1);
  }
  printf("%d\n", parse.port_num );

  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons((parse.port_num));
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
	   printf("unable to bind socket\n");
  }
  struct_thread.p_struct = parse;

  if(listen(sockfd, 1024) < 0){
    printf("*******Error in Listen*******\n");
  }

  while(1){
    if((newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &length_client)) < 0){
      printf("Error in accept. \n");
      exit(-1);
    }
    else{
      struct_thread.thread_id = newsockfd;
      pthread_create(&threads, NULL, client_handler, (void*)&struct_thread);
      if(rc){
        printf("Could not create thread.\n");
        exit(-1);
      }
      printf("**********************Thread Created: %d*************************\n", thread_num);
    }
    thread_num++;
  //  pthread_join(threads, NULL);
  }
  //memset((*parse), 0, sizeof(struct_parse));
  return 0;
}