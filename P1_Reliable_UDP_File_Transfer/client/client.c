/******************************************
# Programming Assignment 1
# client.c
# Code Adapted from Professor's example-code
# Date: September 24, 2017
#Author: Anirudh Tiwari
******************************************/

/***** INCLUDES *****/
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
#include <errno.h>
#include <stdio.h>
#define MAXBUFSIZE 1024

/***** Declaring the data encryption/decryption functions *****/
void data_encryption(char *buffer, int data_len, char key1[], char key2[]);
void data_decryption(char *buffer, int data_len, char key1[], char key2[]);

/***** Packet Structure *****/
typedef struct{
	int pckt_index; 						// to store the packet index
	int pckt_ack;								// to store the packet acknowledgement
	char data_buff[MAXBUFSIZE];	// to store packet data
	int len_data;								// to store packet data length
}struct_pckt;

/***** DATA Encryption Function *****/
void data_encryption(char *buffer, int data_len, char key1[], char key2[]){
	for(int i=0; i<data_len; i++){
		buffer[i] ^= key1[i%38];
	}
	for(int j=0; j<data_len; j++){
		buffer[j] ^= key2[j%58];
	}
}

/***** DATA DECRYPTION FUNCTION *****/
void data_decryption(char *buffer, int data_len, char key1[], char key2[]){
	for(int i=0; i<data_len; i++){
		buffer[i] ^= key2[i%58];
	}
	for(int j=0; j<data_len; j++){
		buffer[j] ^= key1[j%38];
	}
}


/***** MAIN FUNCTION - ENTRY POINT *****/
int main (int argc, char * argv[])
{
	setvbuf(stdout, NULL, _IONBF, 0);	 			// to set stdout to not buffer printf -> Refernce: stackoverflow.com
	int nbytes;                             // number of bytes to be sent or received
	int length;															// length of the sockaddr_in structure
	int sock;                               // this will be our socket
	char command[100];											// to store user input
	char buffer[MAXBUFSIZE];								// buffer for storing the data to be sent/recieved
	struct sockaddr_in server, remote;      //"Internet socket address structure"
	struct hostent *server_hp;							// to represent entry in host database
	char *cname;														// to extract the command from user input
	char *filename;													// to store the filename(in any) from user input
	FILE *fp;																// declaring file pointer to perform file read/write
	struct_pckt* c_pckt = malloc(sizeof(struct_pckt)); 				// defining packet structure for client packet
	struct_pckt* s_pckt = malloc(sizeof(struct_pckt)); 				// defining packet structure for server packet
	struct timeval timeout; 								// defining timeout structure
	int read_length;												// to store the length of the read data
	char key1[39] = "MyMethodOfEncrptingTheDataIIntendToSend";// Data encryption key-1
	char key2[59] = "ThisIsMySecondKeyForMyMethodOfEncrptingTheDataIIntendToSend";// Data encryption key-2

/***** Check for required number of command line arguments
to connect to server *****/
	if (argc < 3)
	{
		printf("USAGE:  <server_ip> <server_port>\n");
		exit(1);
	}

	/******************
	  Here we populate a sockaddr_in struct with
	  information regarding where we'd like to send our packet
	  i.e the Server.
	 ******************/
	bzero(&server,sizeof(server));               //zero the struct
	server.sin_family = AF_INET;                 //address family
	server.sin_port = htons(atoi(argv[2]));      //sets port to network byte order
	server.sin_addr.s_addr = inet_addr(argv[1]); //sets remote IP address

	server_hp = gethostbyname(argv[1]);					 // Return information about host in argv[1]
	if(server_hp < 0){
			perror("Host Unknown");
	}

	bcopy((char*)server_hp->h_addr, (char*)&server.sin_addr, server_hp->h_length);

	/***** Causes the system to create a generic socket of type UDP (datagram) *****/
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("unable to create socket");
	}

	/******************
	  sendto() sends immediately.
	  it will report an error if the message fails to leave the computer
	  however, with UDP, there is no error if the message is lost in the network once it leaves the computer.
	 ******************/
	length = sizeof(struct sockaddr_in);

	while(1){
		/* Initializing timeout struct */
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

		bzero(buffer, sizeof(buffer)); // clear buffer

		/* Display the availble commands for the user */
		printf("\n\tEnter Command as under: \n");
		printf("\t1. get <filename> \n" "\t2. put <filename>\n" "\t3. delete <filename>\n"
		"\t4. ls\n" "\t5. exit \n" );
		gets(command);	// store the command entered by the user

		/* Send the command to the server */
		nbytes = sendto(sock, command, sizeof(command) , 0, (struct sockaddr *)&server, length);


		/***** Split the command from the user to get the command and filename if any
		 Refernce: http://www.geeksforgeeks.org/how-to-split-a-string-in-cc-python-and-java/
		 *****/
		cname = strdup(command);
		strtok(cname, " ");
		filename = strtok(NULL, " ");
		printf("Filename: %s\n", filename);

		/***** Condition: To request file from the server
		Given a valid file name by the user, which is available
		at the server *****/
		if(!strcmp(cname, "get")){
			printf("Get File: %s from the server.\n", filename);
			int exp_index=1; 	// Initializing the expected packet index with 1

			nbytes = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *) &remote, &length);
			if(!strcmp(buffer, "Error")){
				printf("File does not exist on the server. \n");
				continue;
			}

			//Starting the loop for continous receive of data from the server
			do{
				bzero(s_pckt->data_buff, sizeof(s_pckt->data_buff));
				nbytes = recvfrom(sock, (struct_pckt*) s_pckt, sizeof(struct_pckt), 0, (struct sockaddr *) &remote, &length);

				printf("Packet Size from the server: %d \n",nbytes);

				data_decryption(s_pckt->data_buff, s_pckt->len_data, key1, key2);// Decrypting the encoded data from server

				//Packet Authentication
				if(s_pckt->pckt_index == exp_index){
					printf("Writing to the file\n" );
					FILE *fp;
					fp = fopen(filename,"ab");
					fwrite(s_pckt->data_buff, s_pckt->len_data, 1, fp);
					bzero(c_pckt->data_buff, sizeof(c_pckt->data_buff));
					fclose(fp);

					c_pckt->pckt_ack=exp_index;

					//Sending Acknowledgement if the expected packet is recieved
					nbytes = sendto(sock, (struct_pckt*) c_pckt, sizeof(struct_pckt), 0, (struct sockaddr *)&server, length);
					printf("Packet Size being sent to server: %d and ACK sent: %d\n",nbytes, c_pckt->pckt_ack);
					exp_index++;
				}
				else{
					c_pckt->pckt_ack=s_pckt->pckt_index;
					//Sending Acknowledgement if the expected packet not received
					nbytes = sendto(sock, (struct_pckt*)c_pckt, sizeof(struct_pckt), 0, (struct sockaddr *)&server, length);
					printf("Packet Size being sent to server: %d and ACK sent: %d\n",nbytes, c_pckt->pckt_ack);
				}
				if(s_pckt->len_data != MAXBUFSIZE){
					break;
				}
			}while(1); // check for remaining data to be recieved

			// Clearing the memory for the packet structures
			memset(c_pckt, 0, sizeof(struct_pckt));
			memset(s_pckt, 0, sizeof(struct_pckt));
		}

		/***** Condition: To send a file from the client to the server
		Given a valid file name by the user, which is available
		at the client *****/
		else if(!strcmp(cname, "put")){
			printf("Put File: \"%s\" on the server.\n", filename);

			//clear memory for packet structres
			memset(c_pckt, 0, sizeof(struct_pckt));
			memset(s_pckt, 0, sizeof(struct_pckt));

			//Initializing packet structure variables
			c_pckt->pckt_index = 1;
			s_pckt->pckt_ack = 0;

			// Setting timeout
			timeout.tv_sec = 0;
			timeout.tv_usec = 300000;
			setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

			FILE *fp;
			fp = fopen(filename, "rb");
			if(fp == NULL){
				perror("File does nor exist or Error opening file\n");
				char msg[] = "Error";
				nbytes = sendto(sock, msg, sizeof(msg), 0, (struct sockaddr *)&server, length);
				continue;
			}

			else{
				char msg[] = "Success";
				nbytes = sendto(sock, msg, sizeof(msg), 0, (struct sockaddr *)&server, length);
			}

			/* Loop for continous sending of valid packets */
			do{
				bzero(c_pckt->data_buff, sizeof(c_pckt->data_buff));
				read_length = fread(c_pckt->data_buff, 1, MAXBUFSIZE , fp);
				c_pckt->len_data = read_length;

				data_encryption(c_pckt->data_buff, c_pckt->len_data, key1, key2); //encrypting data to be sent on the server
				nbytes = sendto(sock, (struct_pckt*) c_pckt, sizeof(struct_pckt), 0, (struct sockaddr *)&server, length);
				printf("Packet Size sent to the server: %d\n",nbytes);

				bzero(s_pckt->data_buff, sizeof(s_pckt->data_buff));
				nbytes = recvfrom(sock, (struct_pckt*) s_pckt, sizeof(struct_pckt), 0, (struct sockaddr *)&remote, &length);

				// Check for Packet-loss or acknowledgement loss
				if(nbytes < 0){
					printf("---------------------Timeout--------------------------\n");
					fseek(fp, (-1)*read_length, SEEK_CUR);
					continue;
				}
				else{
					printf("Packet Size from server: %d and ack_index from server: %d \n", nbytes, s_pckt->pckt_ack);
					// Check for acknowledgement of the last packet sent
					if(s_pckt->pckt_ack == c_pckt->pckt_index){
						c_pckt->pckt_index++;
					}
					else{
						fseek(fp, (-1)*read_length, SEEK_CUR);
					}
				}
				if(read_length != MAXBUFSIZE){
					break;
				}
			}while(1);

			// Clear memory for packet structures
			memset(c_pckt, 0, sizeof(struct_pckt));
			memset(s_pckt, 0, sizeof(struct_pckt));
		}

		/***** Condition: To delete a file from the server
		Given a valid file name by the user, which is available
		at the server *****/
		else if(!strcmp(cname, "delete")){
			// Recieving acknowledgement whether the file is deleted.
			nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&remote, &length);
			printf(" %s \n", buffer );
		}

		/***** Condition: To list all the files available at the server
		Reference: http://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/ *****/
		else if(!strcmp(cname, "ls")){
			printf("\nListing Files from the server.\n");
			bzero(buffer, sizeof(buffer));
			nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&remote, &length);
			printf("%s\n", buffer ); //Printing data recieved from the buffer
		}

		/***** Condition: To Request the server to gracefully release the
		connection *****/
		else if(!strcmp(cname, "exit")){
			printf("Request server to release the connection.\n");
			nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&remote, &length);
			printf(" %s \n", buffer );
		}

		/****** Condition: If the Command entered
		by the user is not supported *****/
		else{
			printf("Unsupported Command\n");
		}

		bzero(buffer,sizeof(buffer)); //clearing the buffer
	}
}
