# Network Systems
### Programming Assignment 3
### Author: *Anirudh Tiwari*
### University of Colorado Boulder  

**Distributed File System**
>A Distributed File System is a client/server-based application that allows client to store and retrieve files on multiple servers. One of the features of Distributed file system is that each file can be divided in to pieces and stored on different servers and can be retrieved even if one server is not active.

**Project Description and Implementation**  
>1. "make" for dfc.c and dfs.c to generate executables.
>2. Run ./dfc <configuration-file> at client and ./dfs /DFS<number> <port-number> at server.
>3. Run Command "list": To list files from the users main folder.
>4. Run Command "list <subfolder>":  To list files from the users sub folder.   
>5. Run Command "put <filename>": To put files as per the requirements on the Distributed file server.
>6. Run Command "put <filename> <subfolder>": To put files in the users sub-folder as per the requirements on the Distributed file server.
>7. Run Command "get <filename>": To get the file from the distributed file server.
>8. Run Command "get <filename> <subfolder>": To get the file from the users sub-folder in distributed file server.
>9. Run Command "mkdir <sub-folder name>": To create a sub-folder in the users main folder in the distributed file server.

**Miscellaneous Tasks**
>1. Time out or server not available: A client tries for 1 second to connect to the server. If DFS server does not response in 1 second, we consider that server is not available.
>2. Handle multiple connections: DFS servers is able to handle simultaneous connections from different DFC clients say Alice and Anirudh at the same time.
>3. Successfully created a distributed file system for reliable and secure file storage.

**Extra Credits**
>1. Encryption and Decryption for sent file through "put" and recieved file through "get" respectively.
>2. Implemented Sub-folder (With MKDIR command) and stretched "put" "get" and "list" for the subfolders.
>3.  Traffic optimization: The default GET command gets all available pieces from all available servers which actually consumes twice the amount of the actual data needed. Thus upgraded GET command so that it can reduce traffic consumption by making the client code request only those parts which it requires to rebuild the file at the client end when requesting a file through the "get" command.
