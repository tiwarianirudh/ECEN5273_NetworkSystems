

**ANIRUDH TIWARI
	PROGRAMMING ASSIGNMENT 1
  Reliable UDP File Transfer
	DATE: September 24, 2017**


**Folder Structure**
client
	----> client.c
	----> Makefile
server
	----> server.c
	----> Makefile
README.txt

**Instructions to Execute the files.**

>A. "make" .c files to generate the executables for  both server.c and client.c
>B. "make clean" to remove object files
>C. IP of server can be found by running "ifconfig" on the server or running with "localhost" for local machines in place of before mentioned IP
>1. At Server - "./server <Port Number>"
>2. At Client - "./client <Server IP> <Same Port Number as used on Server>"



**Commands expected from the user:**

>a. get <filename> .
>1. To get the ENCRYPTED file from the server if it exists and DECRYPT it.
>2. If it is not available on the server then display the error message.

>b. put <filename> .
>1. To transfer the ENCRYPTED file from client onto the server, if it exists on the client, and DECRYPT it on the server.
>2. Displays error message on the client terminal/console if the file does not exist
>3. Check for the error on client and terminate the write operation if there is error at the client.

>c. ls .
>1. Display on the client console/terminal list of files available at the server
>2. The server sends an error message if it is unable to read into the directory of the server.

>d. exit .
>1. The client requests the server to close the socket connection.
>2. The server sends the release connection message to the client and gracefully exit the connection closing the socket.

>e. delete .
>1. The client requests the server to delete the file.
>2. The server deletes the file if available.

>f. For other commands.
>1. Both the server and the client display Unsupported Command.




**Assumptions**
> Spaces in file names is not supported,
> Single Space between command and filename is expected, as the splitting of the two depends on a single space.




**Functionality**
> Implemented Reliability accounting for loss in data packets and loss of acknowledgement.
> Implemented Data Encryption and Data Decryption at both the client and the server.
