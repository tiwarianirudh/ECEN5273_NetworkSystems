/******************************************
# Programming Assignment 1
# server.c
# Code Adapted from Professor's example-code
# Date: September 24, 2017
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

/***** Declaring the data encrption/decrption functions *****/
void data_encryption(char *buffer, int data_len, char key1[], char key2[]);
void data_decryption(char *buffer, int data_len, char key1[], char key2[]);

/***** Packet Structure *****/
typedef struct{
	int pckt_index;										// to store the packet index
	int pckt_ack;											// to store the packet acknowledgement
	char data_buff[MAXBUFSIZE];				// to store packet data
	int len_data;											// to store packet data length
}struct_pckt;

/***** DATA DECRYPTION FUNCTION *****/
void data_decryption(char *buffer, int data_len, char key1[], char key2[]){
	for(int i=0; i<data_len; i++){
		buffer[i] ^= key2[i%58];
	}
	for(int j=0; j<data_len; j++){
		buffer[j] ^= key1[j%38];
	}
}

/***** DATA Encryption Function *****/
void data_encryption(char *buffer, int data_len, char key1[], char key2[]){
	for(int i=0; i<data_len; i++){
		buffer[i] ^= key1[i%38];
	}
	for(int j=0; j<data_len; j++){
		buffer[j] ^= key2[j%58];
	}
}

/***** MAIN FUNCTION - ENTRY POINT *****/
int main (int argc, char * argv[] )
{
	setvbuf(stdout, NULL, _IONBF, 0); 		//to set stdout to not buffer printf -> stackoverflow.com
	int sock;                           	// this will be our socket
	struct sockaddr_in sin, remote;     	//"Internet socket address structure"
	unsigned int remote_length;         	// length of the sockaddr_in structure
	int nbytes;                        		// number of bytes to be sent or received
	char buffer[MAXBUFSIZE];             	// buffer for storing the data to be sent/recieved
	char *cname;													// to extract the command from user input at client
	char *filename;												// to store the filename(in any) from user input at client
	struct_pckt* c_pckt = malloc(sizeof(struct_pckt));// defining packet structure for client packet
	struct_pckt* s_pckt = malloc(sizeof(struct_pckt));// defining packet structure for server packet
	FILE *fp;															// declaring file pointer to perform file read/write
	int read_length;											// to store the length of the read data
	struct timeval timeout;								// defining timeout structure
	char key1[39] = "MyMethodOfEncrptingTheDataIIntendToSend"; // Data encryption key-1
	char key2[59] = "ThisIsMySecondKeyForMyMethodOfEncrptingTheDataIIntendToSend";// Data encryption key-2


/***** Check for required number of command line arguments
to start the server *****/
	if (argc != 2)
	{
		printf ("USAGE:  <port>\n");
		exit(1);
	}

	/******************
	  This code populates the sockaddr_in struct with
	  the information about our socket
	 ******************/
	bzero(&sin,sizeof(sin));                    //zero the struct
	sin.sin_family = AF_INET;                   //address family
	sin.sin_port = htons(atoi(argv[1]));        //htons() sets the port # to network byte order
	sin.sin_addr.s_addr = INADDR_ANY;           //supplies the IP address of the local machine


	/***** Causes the system to create a generic socket of type UDP (datagram)*****/
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("unable to create socket");
	}

	/******************
	  Once we've created a socket, we must bind that socket to the
	  local address and port we've supplied in the sockaddr_in struct
	 ******************/
	if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		printf("unable to bind socket\n");
	}

	remote_length = sizeof(struct sockaddr_in);

	//waits for an incoming message
	while(1){
		/* Initializing timeout struct */
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

		bzero(buffer, sizeof(buffer));

		/* Recieving the command from the client */
		nbytes = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&remote, &remote_length);

		/***** Split the command from the user to get the command and filename if any
		 Refernce: http://www.geeksforgeeks.org/how-to-split-a-string-in-cc-python-and-java/
		 *****/
		cname = strdup(buffer);
		strtok(cname," ");
		filename = strtok(NULL, " ");

		printf("Command Recieved: %s \n", cname);
		printf("File name:%s\n", filename );

		/***** Condition: To send file to the client (requested by the client)
		Given a valid file name by the user, which is available
		at the server *****/
		if(!strcmp(cname, "get")){
			/* Setting Timeout */
			timeout.tv_sec = 0;
			timeout.tv_usec = 300000;
			setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

			s_pckt->pckt_index = 1;	// Initializing the server packet index with 1
			c_pckt->pckt_ack = 0;		// Initializing the packet acknowledgement with 0

			FILE *fp;
			fp = fopen(filename, "rb");
			if(fp == NULL){
				perror("File does nor exist or Error opening file \n");
				char msg[] = "Error";
				nbytes = sendto(sock, msg, sizeof(msg), 0, (struct sockaddr *)&remote, remote_length);
				continue;
			}
			else{
				char msg[] = "Success";
				nbytes = sendto(sock, msg, sizeof(msg), 0, (struct sockaddr *)&remote, remote_length);
			}
			//Starting the loop for continous sending of data to the server
			do{
				bzero(s_pckt->data_buff, sizeof(s_pckt->data_buff));
				read_length = fread(s_pckt->data_buff, 1, MAXBUFSIZE , fp);
				s_pckt->len_data = read_length;
				printf("Read length%d\n", read_length );

				data_encryption(s_pckt->data_buff, s_pckt->len_data, key1, key2);//Encrypting data to be sent to the client
				nbytes = sendto(sock, (struct_pckt*) s_pckt, sizeof(struct_pckt), 0, (struct sockaddr *)&remote, remote_length);
				printf("Packet Size send to client: %d\n",nbytes);

				bzero(s_pckt->data_buff, sizeof(s_pckt->data_buff));
				nbytes = recvfrom(sock, (struct_pckt*) c_pckt, sizeof(struct_pckt), 0, (struct sockaddr *)&remote, &remote_length);

				//Packet Authentication
				if(nbytes<0){
					printf("---------------------Timeout--------------------------\n");
					fseek(fp, (-1)*read_length, SEEK_CUR);
					continue;
				}
				printf("Packet Size from client: %d and client ack_index: %d \n",nbytes, c_pckt->pckt_ack);

				//incrementing Server packet index if the expected acknowledgement is recieved
				if(c_pckt->pckt_ack == s_pckt->pckt_index){
					s_pckt->pckt_index++;
				}
				//retaining Server packet index if the expected acknowledgement is not recieved
				else{
					fseek(fp, (-1)*read_length, SEEK_CUR);
				}
				if(read_length != MAXBUFSIZE){
					break;
				}
			}while(1);

			// Clearing the memory for the packet structures
			memset(c_pckt, 0, sizeof(struct_pckt));
			memset(s_pckt, 0, sizeof(struct_pckt));
		}

		/***** Condition: To recieve a file from the client (sent by the client)
		Given a valid file name by the user, which is available
		at the client *****/
		else if(!strcmp(cname,"put")){
			//clear memory for packet structres
			memset(c_pckt, 0, sizeof(struct_pckt));
			memset(s_pckt, 0, sizeof(struct_pckt));

			int exp_index=1;	// Initializing the expected packet with 1
			s_pckt->pckt_ack = 1;//Initializing packet structure acknowledgement

			bzero(buffer, sizeof(buffer));
			nbytes = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *) &remote, &remote_length);

			if(!strcmp(buffer, "Error")){
				continue;
			}
			FILE *fp;
			fp = fopen(filename,"ab");
			if(fp == NULL){
				perror("Error opening file \n");
			}

			/* Loop for continous receiving of valid packets */
			do{
				bzero(c_pckt->data_buff, sizeof(c_pckt->data_buff));
				nbytes = recvfrom(sock, (struct_pckt*) c_pckt, sizeof(struct_pckt), 0, (struct sockaddr *) &remote, &remote_length);
				printf("Packet Size from the client: %d \n",nbytes);

				data_decryption(c_pckt->data_buff, c_pckt->len_data, key1, key2);//decrypting data sent by the client

				/* Packet Validation */
				if(c_pckt->pckt_index == exp_index){
					printf("Writing to the file \n" );
					fwrite(c_pckt->data_buff, c_pckt->len_data, 1, fp);
					bzero(c_pckt->data_buff, sizeof(c_pckt->data_buff));
					s_pckt->pckt_ack=exp_index;
					//Sending Acknowledgement if the expected packet is recieved
					nbytes = sendto(sock, (struct_pckt*) s_pckt, sizeof(struct_pckt), 0, (struct sockaddr *) &remote, remote_length);
					printf("Packet Size being sent to client: %d and ACK from server: %d\n",nbytes, s_pckt->pckt_ack);
					exp_index++;
				}
				else{
					s_pckt->pckt_ack=c_pckt->pckt_index;
					//Sending Acknowledgement if the expected packet not received
					nbytes = sendto(sock, (struct_pckt*)s_pckt, sizeof(struct_pckt), 0, (struct sockaddr *) &remote, remote_length);
				}
				if(c_pckt->len_data != MAXBUFSIZE){
					break;
				}
			}while(1); // check for remaining data to be recieved
			fclose(fp);

			// Clearing the memory for the packet structures
			memset(c_pckt, 0, sizeof(struct_pckt));
			memset(s_pckt, 0, sizeof(struct_pckt));
		}

		/***** Condition: To delete a file from the server (requested by the client)
		Given a valid file name by the user, which is available
		at the server *****/
		else if(!strcmp(cname,"delete")){
			if(remove(filename)){
				char msg[] = "unable to delete file";
				// Sending acknowledgement for error in deleting the file.
				nbytes = sendto(sock, msg, sizeof(msg), 0, (struct sockaddr *) &remote, remote_length);
				perror("Error");
			}
			else{
				//Sending acknowledgement for deleting the file.
				char msg[] = "Deleted File";
				nbytes = sendto(sock, msg, sizeof(msg), 0, (struct sockaddr *) &remote, remote_length);
			}
		}

		/***** Condition: Client's request to list all file from the server
		Thus, sending the file-names in the directory to the client
		Reference: http://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/ *****/
		else if(!strcmp(cname,"ls")){
			DIR *current_dir;
			struct dirent *struct_dir;

			current_dir = opendir(".");
			if(current_dir == NULL){
				char msg[] = "Unable to read Directory";
				/* sending error if unable to read the directory */
				nbytes = sendto(sock, msg, sizeof(msg), 0, (struct sockaddr *) &remote, remote_length);
				perror("Error");
			}
			else{
				bzero(buffer, sizeof(buffer));
				//Concatinating the file names in the buffer
				while((struct_dir = readdir(current_dir)) != NULL){
					strcat(buffer, struct_dir->d_name);
					strcat(buffer, "\n");
				}
			/* Sending the file-names to the client*/
			nbytes = sendto(sock, buffer, sizeof(buffer) , 0, (struct sockaddr *)&remote, remote_length);
			}
		}
		/***** Condition: Client's request to the server
		to release the connection *****/
		else if(!strcmp(cname,"exit")){
			char msg[] = "Connection Released by Server";
			nbytes = sendto(sock, msg, sizeof(msg), 0, (struct sockaddr *) &remote, remote_length);
			close(sock);
			exit(0);
		}

		else{
			printf("Unsupported Command \n");
		}

		bzero(buffer, sizeof(buffer));
		}

}
