# Network Systems
### Programming Assignment 4 - Simple Web Proxy Server
### Author: *Anirudh Tiwari*
### University of Colorado Boulder  

**Objective**
>1. In this programming assignment, a simple web proxy server is built that is capable of accepting HTTP requests from clients, pass them to HTTP server and handle returning traffic from the HTTP server back to clients.
>2. As a part of Extra Credit, build on top of caching and content transformation code, the last piece of functionality that is called link prefetching.

**Instruction for Running the code**
>1. Step 1: make clean
>2. Step 2: make
>3. Step 3: "./webproxy <port number> <Cache Expiration time> & "  {For My Mozilla browser Cache setting is for port number 10001}

**Features Implemented**
>1. Multi-threaded Proxy.
>2. Caching (Page Cache and Hostname IP address cache)
>3. Blocking few websites as per the blocked files list.

**Extra Credits Feature Implemented**
>1. Link Prefetching.

**Implementation**
>a. **Proxy Implementation**
>1. Browser sends request to the acting middle-man, the proxy server.
>2. The Proxy Server checks for a Valid Get Request (ONLY Get are supported) and throws a 400 Bad Request error on being Invalid.
>3. The Proxy Server Then checks for a Valid HTTP version, HTTP 1.0/1.1 and throws a 400 Bad Request Error on the version being Invalid.
>4. The Proxy Server checks for cache files for the request made. If the cache is present within the specified cache expiration time, the cache pages are immediately served to the client.
>5. Else, the Proxy Server then checks for Hostname formatting, whether its a URL or an IP with a port number. If its a URL it checks for its presence in the hostname cache file to eliminate a DNS query if present (Default Port 80), else it connects to the provide IP on the provided port number.
>6. If the hostname is not present in cache file, the Proxy Server checks for Hostname Validity, whether the IP generated is Valid or not.
>7. Upon Validation, the request is sent to the host.

>b. **Caching and Timeout Implementation**
>1. MD5 hashing is used to generated the filename for cached files from their respective request url. Thus each URL request is stored seperatedly in cache.
>2. The response from the host is written into the cache file with the current-time being written to the start of the file for cache-expiration check.
>3. These cached files if requested within the specified time for expiration are served to the client else are removed from the cache and are requested again from the Host to create a fresh cache.

>c. **Link Prefetching (Extra-Credit Part)**
>1. The cached files for a particular request are opened in order to check for any links to be pre-fetched. If there are any, the process is forked and the particular link is prefetched.
>2. The prefetched links are stored in the same manner with their time of creation written at the starting of the files and same cache check Implementation.
>3. Link prefetching (If tested by disconnecting the Internet Service) would not display any embed images because the prefetching would only get data from the URL.

**Websites Tested**
>a. Browser
>1. morse.colorado.edu
>2. caida.org
>3. cplusplus.com

>b. Telnet
>1. For Single request, tested: Telnet: (echo -en "GET http://www.caida.org/home/ HTTP/1.0\n\n"; sleep 1) | telnet 127.0.0.1 10001
