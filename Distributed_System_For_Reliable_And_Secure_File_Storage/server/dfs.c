/******************************************
# Programming Assignment 3
# server.c
# Code Adapted from Professor's example-code
# Date: November 19, 2017
#Author: Anirudh Tiwari
******************************************/

/***** INCLUDES *****/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
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
  int part_num = 0;
  unsigned long int len_part = 0;
  char part_file[64];
  char dfs[64];
  char dfs_partname[128];
  char path_directory[64];
  int auth_flag = 0;


  if (argc != 3)
	{
		printf ("\nUsage: </DFSn> <portNo>\n");
		exit(1);
	}
  strcpy(port_num, argv[2]);
  strcpy(dfs, argv[1]);
  char *conf = "dfs.conf";
  parse_status = parse_file(&(parse), conf, entry);
  if(parse_status==-1){
    printf("Error Parsing the Configuration File\n");
    exit(1);
  }
  //printf("%d\n", *entry);

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
      printf("Username in struct_auth recieved: %s\t Password: %s Command: %s \n\n", &auth->username[0], &auth->password[0],&auth->command[0]);
    //}
    for(int i=0; i<*entry; i++){
      if(!(strcmp((parse).username[i], &auth->username[0]))){
        if(!(strcmp((parse).password[i], &auth->password[0]))){
          bzero(buffer, MAXBUFSIZE);
          strcpy(buffer, "User Exists");
          auth_flag = 1;
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
    if(auth_flag==1){

/***************************
*****Functionality: PUT
***************************/
      if(!strcmp(&auth->command[0], "put")){
        //Recieving Servers Part-A
        bzero(buffer, MAXBUFSIZE);
        recv(newsockfd, buffer, sizeof(buffer), 0);
        printf("%s ****\n", buffer );
        sscanf(buffer, "%*[^:]%*c%d %s %lu", &part_num, part_file, &len_part);
        printf("Part Number:%d, Filename: %s, Part Length:%lu\n", part_num, part_file, len_part );

        bzero(buffer, MAXBUFSIZE);
        strcpy(buffer, "Recieved Iteration");
        if((nbytes = send(newsockfd, buffer, strlen(buffer), 0)) < 0){
          printf("In Synq Send()\n");
          perror("Error: \n");
        }

        int parts_iteration = (len_part/MAXBUFSIZE);
        //printf("********** Number of iterations: %d **********\n", parts_iteration);
        int temp = 0;

        //FILE* dfs_file;

        bzero(path_directory, MAXBUFSIZE);
        sprintf(path_directory, ".%s/%s", argv[1], &auth->username[0] );
        printf("%s\n", path_directory );
        mkdir(path_directory, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

        // if(part_num == 1){
          bzero(dfs_partname, sizeof(dfs_partname));
          sprintf(dfs_partname, "%s/.%s.%d", path_directory, part_file, part_num);
          printf("%s \n", dfs_partname );

          FILE* dfs_file;
          dfs_file = fopen(dfs_partname, "ab");
          if(!dfs_file){
            printf("Error creating part number: %d\n", part_num);
            return -1;
          }
          do{
            bzero(buffer, MAXBUFSIZE);
            nbytes = recv(newsockfd, buffer, sizeof(buffer), 0);
            fwrite(buffer, 1, nbytes, dfs_file);
            temp++;
            bzero(buffer, MAXBUFSIZE);
            strcpy(buffer, "Recieved Iteration");
            if((nbytes = send(newsockfd, buffer, strlen(buffer), 0)) < 0){
              printf("In Synq Send()\n");
              perror("Error: \n");
            }
            if(temp == (parts_iteration)){
              bzero(buffer, MAXBUFSIZE);
              nbytes = recv(newsockfd, buffer, sizeof(buffer), 0);
              fwrite(buffer, 1, nbytes, dfs_file);

              bzero(buffer, MAXBUFSIZE);
              strcpy(buffer, "Recieved Iteration");
              if((nbytes = send(newsockfd, buffer, strlen(buffer), 0)) < 0){
                printf("In Synq Send()\n");
                perror("Error: \n");
              }
            }
          }while(temp<parts_iteration);
          //printf("LoopRan : %d\n", temp);
          temp=0;
          part_num = 0;
          len_part = 0;

          bzero(part_file, sizeof(part_file));
          fclose(dfs_file);
        // }

        //Recieving Servers PART-B
        bzero(buffer, MAXBUFSIZE);
        recv(newsockfd, buffer, sizeof(buffer), 0);
        printf("%s \n", buffer );
        sscanf(buffer, "%*[^:]%*c%d %s %lu", &part_num, part_file, &len_part);
        printf("Part Number:%d, Filename: %s, Part Length:%lu\n", part_num, part_file, len_part );
        parts_iteration = (len_part/MAXBUFSIZE);
        printf("********** Number of iterations: %d **********\n", parts_iteration);

        bzero(buffer, MAXBUFSIZE);
        strcpy(buffer, "Recieved Iteration");
        if((nbytes = send(newsockfd, buffer, strlen(buffer), 0)) < 0){
          printf("In Synq Send()\n");
          perror("Error: \n");
        }
        //FILE* dfs_file;

        bzero(path_directory, MAXBUFSIZE);
        sprintf(path_directory, ".%s/%s", argv[1], &auth->username[0] );
        printf("%s\n", path_directory );
        mkdir(path_directory, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

        // if(part_num == 1){
        bzero(dfs_partname, sizeof(dfs_partname));
        sprintf(dfs_partname, "%s/.%s.%d", path_directory, part_file, part_num);
        printf("%s\n", dfs_partname );
        dfs_file = fopen(dfs_partname, "ab");
        if(!dfs_file){
          printf("Error creating part number: %d\n", part_num);
          return -1;
        }
        do{
          bzero(buffer, MAXBUFSIZE);
          nbytes = recv(newsockfd, buffer, sizeof(buffer), 0);
          fwrite(buffer, 1, nbytes, dfs_file);
          temp++;
          bzero(buffer, MAXBUFSIZE);
          strcpy(buffer, "Recieved Iteration");
          if((nbytes = send(newsockfd, buffer, strlen(buffer), 0)) < 0){
            printf("In Synq Send()\n");
            perror("Error: \n");
          }
          if(temp == (parts_iteration)){
            bzero(buffer, MAXBUFSIZE);
            nbytes = recv(newsockfd, buffer, sizeof(buffer), 0);
            fwrite(buffer, 1, nbytes, dfs_file);
            bzero(buffer, MAXBUFSIZE);
            strcpy(buffer, "Recieved Iteration");
            if((nbytes = send(newsockfd, buffer, strlen(buffer), 0)) < 0){
              printf("In Synq Send()\n");
              perror("Error: \n");
            }
          }
        }while(temp<parts_iteration);
        //printf("LoopRan2 : %d\n", temp);
        temp = 0;
        part_num = 0;
        len_part = 0;
        bzero(part_file, sizeof(part_file));
        fclose(dfs_file);

        printf("\n\n********PUT operation Complete********\n\n");
      }

    else if(!strcmp(&auth->command[0],"list")){
      bzero(buffer, MAXBUFSIZE);
      recv(newsockfd, buffer, sizeof(buffer), 0);
			DIR *current_dir;
			struct dirent *struct_dir;

      bzero(path_directory, MAXBUFSIZE);
      sprintf(path_directory, ".%s/%s", argv[1], &auth->username[0] );
      //printf("%s\n", path_directory );
      mkdir(path_directory, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

			current_dir = opendir(path_directory);
			if(current_dir == NULL){
				char msg[] = "Unable to read Directory";
        printf("%s\n", msg );
				/* sending error if unable to read the directory */
				nbytes = send(newsockfd, msg, strlen(msg), 0);
				perror("Error");
        continue;
			}
			else{
				bzero(buffer, sizeof(buffer));
				//Concatinating the file names in the buffer
				while((struct_dir = readdir(current_dir)) != NULL){
          if(!strcmp(struct_dir->d_name, ".") || !strcmp(struct_dir->d_name, "..")){
            continue;
          }
          else{
            strcat(buffer, struct_dir->d_name);
					  strcat(buffer, "\n");
          }
				}
			/* Sending the file-names to the client*/
      if((nbytes = send(newsockfd, buffer, strlen(buffer), 0)) < 0){
        //printf("");
        perror("Error: \n");
      }//printf("%d\n", nbytes );
      bzero(buffer, MAXBUFSIZE);
      recv(newsockfd, buffer, sizeof(buffer), 0);
			}
		}
  }
}
  return 0;
}
