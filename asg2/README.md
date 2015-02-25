Assignment2 for cs654 version 1.0 15/Feb./2015

Author Information
-------------------
Name: Borui Ye
Email: b7ye@uwaterloo.ca
Phone number: (519)-722-9166

Compilation
------------
Run "make" to compile, you can see two executable files stringClient and stringServer.

Usage
------
1. Run "./stringServer", you can see hostname and port number like:
SERVER_ADDRESS hostname
SERVER_PORT port number
2. Set environmental variables:
In client's side, you should first set $SERVER_ADDRESS and $SERVER_PORT:
$ export SERVER_ADDRESS="hostname"                                                                                                                                           
$ export SERVER_PORT="port number"
Then run "./stringClient", it can automatically retrieve the two values using the "getenv()" call.
3. After successful connection, you can send message to server. The server can accept multiple clients.
