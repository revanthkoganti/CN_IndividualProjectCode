# Peer-to-peer messaging and broadcast message

A peer-to-peer messaging application also used for broadcasting purposes is built on C language. A TCP socket is used for peer-to-peer communication and an UDP socket for broadcasting message.
A main server running in file TCPEchoServer and TCP socket is created in CreateTCPServerSocket file.

Users using this application can be able to signup and login using username and password. The username and password are stored in mysql database with password being hashed.
![image](https://user-images.githubusercontent.com/76093689/235729012-e4b50a7b-e77a-44b7-8b75-fd7fa251a4ae.png)

After a successfull login, user is prompted is two options. One for the display of all active users and second for broadcasting the messages.

![image](https://user-images.githubusercontent.com/76093689/235813464-6808c1c0-6625-47e0-a6d9-98a3b0dfddd1.png)

User can select an option of his choice and can do peer to peer and broadcasting.

Required packages:
gcc - 9.4.0
mysql - 8.0.32

Steps to execute the project:

Run this command to start the server in port 4400

gcc -lpthread -lssl -lcrypto -l:libssl.so.1.1 -l:libcrypto.so.1.1 -o TCPEchoServer TCPEchoServer-Thread.c DieWithError.c HandleTCPClient.c AcceptTCPConnection.c CreateTCPServerSocket.c $(mysql_config --libs --cflags)

After executing the above command, run this in the same terminal: ./TCPEchoServer 4400

For starting the Client: Execute this command in a new terminal

 gcc -pthread -lssl -o TCPEchoClient TCPEchoClient.c DieWithError.c TCPClientThread.c
 
 After executing the above command, run this in the same terminal: ./TCPEchoClient 127.0.0.1 "Hello" 4400
 
In this command, we are giving the server information which is running on localhost(127.0.0.1) and passing a astring "Hello" to notify the server that a client is connecting and the port of the server 4400 in which it is already running and up.
