

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
>>At Server - "./server <Port Number>"
>>At Client - "./client <Server IP> <Same Port Number as used on Server>"



**Commands expected from the user:**

>1. get <filename>
>> To get the ENCRYPTED file from the server if it exists and DECRYPT it.
>> If it is not available on the server then display the error message.

>2. put <filename>
>> To transfer the ENCRYPTED file from client onto the server, if it exists on the client, and DECRYPT it on the server.
>> Displays error message on the client terminal/console if the file does not exist
>> Check for the error on client and terminate the write operation if there is error at the client.

>3. ls
>> Display on the client console/terminal list of files available at the server
>> The server sends an error message if it is unable to read into the directory of the server.

>4. exit
>> The client requests the server to close the socket connection.
>> The server sends the release connection message to the client and gracefully exit the connection closing the socket.

>5. For other commands
>> Both the server and the client display Unsupported Command.




**Assumptions**
>> Spaces in file names is not supported,
>> Single Space between command and filename is expected, as the splitting of the two depends on a single space.




**Functionality**
>> Implemented Reliability accounting for loss in data packets and loss of acknowledgement.
>> Implemented Data Encryption and Data Decryption at both the client and the server.
