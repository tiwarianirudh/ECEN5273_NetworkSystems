# Network Systems
### Programming Assignment 2: HTTP Based Web Server
### Author: *Anirudh Tiwari*
### University of Colorado Boulder


**Instructions**

>a. For Execution File
>1. Run "make" with .c files to generate executbles for "server"
>>For removal of object files Run "make clean"

>2. TO Run
>>Run "./server"


**Assumptions**
>1. ws.conf has no blank lines

**Functionality**

>1. Basic functionality is implemented with error handling for:
  >>a. 400 BAD REQUEST for METHOD, URL and VERSION
  >>b. 404 NOT FOUND
  >>c. 501 NOT IMPLEMENTED METHOD
  >>d. 500 INTERNAL SERVER ERROR

>2. ws.conf The configuration file
	>>a. Check for port number
	>>b. Checks for document root
	>>c. Checks for file requested, its content type and the extension

>3. POST is implemented and can be tested by using telnet and typing in a valid request

>4. Keep alive is implemented with lesser accuracy
  >>a. Could not complete the Pipelining but stil the code checks for all the basic functionalities



**Testing**

>1. Check Website and other files
	>> http://localhost:5567/index.html

>2. 404 Error
	>> Can be seen within the initial reuest for the index.html

>3. Bad URL (Error 400 - Invalid URL)
	>> Can be seen within the initial reuest for the index.html

>4. HTTP Version (Error 400 - Invalid HTTP)
  >> Can be seen within the initial reuest for the index.html

>5. BAD Method Request
  >> SCAN / HTTP/1.1

>6. to test POST
	>> telnet localhost 5567 ---------> POST /.index.html HTTP/1.1

>7. to test method not implemented
  >> OPTIONS / HTTP/1.1
