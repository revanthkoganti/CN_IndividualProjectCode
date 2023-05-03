# CN Peer to Peer and broadcast messaging application

A peer-to-peer messaging application also used for broadcasting purposes is built on C language. A TCP socket is used for peer-to-peer communication and an UDP socket for broadcasting message.
A main server running in file TCPEchoServer and TCP socket is created in CreateTCPServerSocket file.

Users using this application can be able to signup and login using username and password. The username and password are stored in mysql database with password being hashed. The password is hashed using the openssl SHA256. 
![signup login](https://user-images.githubusercontent.com/76093689/235829365-b0907e06-7a1d-42ac-a395-5c763671763f.png)

After a successful login, user is prompted is two options. One for the display of all active users and second for broadcasting the messages.
Implemented the login validation, when validation is failed , server allows for 3 attempts and then terminates the connection.
Signup is also implemented with password and confirmpassword and the socket is terminated if both are not matched.
User can select an option of his choice and can do peer to peer and broadcasting.
![Broadcast](https://user-images.githubusercontent.com/76093689/235829743-45bb5ba5-2bd5-49ff-90a3-96e45d4f97f4.png)

Broadcast is running on the port 4401 for receiving the messages. Faced few issues with handling multiple clients with this broadcast, hence i have implemented the feature like when a client selects the listen to broadcast messages, the broadcast receive will keep running port on 4401 and listen the messages.

I have faced issues with SQL connection in docker, hence i have integrated the P2P but i did not test the functionality.


Required packages:
gcc - 9.4.0
mysql - 8.0.32
libmysqlclient-dev
openssl

Steps to execute the project:

Run this command to start the server in port 4400

gcc -pthread -lssl -lcrypto -l:libssl.so.1.1 -l:libcrypto.so.1.1 -o TCPEchoServer TCPEchoServer-Thread.c DieWithError.c HandleTCPClient.c AcceptTCPConnection.c CreateTCPServerSocket.c $(mysql_config --libs --cflags)

After executing the above command, run this in the same terminal: ./TCPEchoServer 4400

For starting the Client: Execute this command in a new terminal

 gcc -pthread -lssl -o TCPEchoClient TCPEchoClient.c DieWithError.c TCPClientThread.c
 
 After executing the above command, run this in the same terminal: ./TCPEchoClient 127.0.0.1 "Hello" 4400
 
 Install mysql server and create a database mydb. Also change the root password in TCPEchoServer.h file based on your configuration.
 
In this command, we are giving the server information which is running on localhost(127.0.0.1) and passing a astring "Hello" to notify the server that a client is connecting and the port of the server 4400 in which it is already running and up.
