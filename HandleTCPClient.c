#include "TCPEchoServer.h" /* TCP echo server includes */
#include <stdio.h>         /* for printf() and fprintf() */
#include <sys/socket.h>    /* for recv() and send() */
#include <unistd.h>        /* for close() */
#include <string.h>
#include <mysql/mysql.h> /* for MySQL connection */

MYSQL *conn; /* MySQL connection handle */

#define RCVBUFSIZE 100 /* Size of receive buffer */
#define USERNAME_SIZE 32
#define PASSWORD_SIZE 32

void InsertDataInUsers(char username[USERNAME_SIZE], char password[PASSWORD_SIZE])
{
    ConnectToDB();
    char query[200];
    sprintf(query, "insert into users(username,password) values('%s','%s');", username, password);
    printf(query);
    if (mysql_query(conn, query))
        printf("Unable to insert data into Employee table\n");
    DisconnectFromDB();
}

void DieWithError(char *errorMessage); /* Error handling function */

void HandleTCPClient(int clntSocket)
{
    char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
    int recvMsgSize;             /* Size of received message */
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    char confirmpassword[PASSWORD_SIZE];

    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE - 1, 0)) < 0)
        DieWithError("recv() failed");
    echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */

    /* Send prompt message to client */
    char promptMessage[] = "Welcome! Please select your choice as 1 or 2. \n 1. login \n 2. signup \n";
    if (send(clntSocket, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
        DieWithError("send() failed");

    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
        DieWithError("recv() failed");
    echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */

    switch (atoi(echoBuffer))
    {
    case 1:
        strcpy(promptMessage, "Enter username: ");
        if (send(clntSocket, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
            DieWithError("send() failed");

        if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
            DieWithError("recv() failed");
        echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */

        strcpy(username, echoBuffer);

        strcpy(echoBuffer, "");

        strcpy(promptMessage, "Enter Password: ");
        if (send(clntSocket, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
            DieWithError("send() failed");
        echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */


        if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
            DieWithError("recv() failed");
        echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */

        strcpy(password, echoBuffer);

        InsertDataInUsers(username, password);

        break;
    case 2:
        strcpy(promptMessage, "Enter username: ");
        if (send(clntSocket, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
            DieWithError("send() failed");

        if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
            DieWithError("recv() failed");
        echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */

        strcpy(username, echoBuffer);

        strcpy(promptMessage, "Enter Password: ");
        if (send(clntSocket, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
            DieWithError("send() failed");

        if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
            DieWithError("recv() failed");

        echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */
        strcpy(password, echoBuffer);

        strcpy(promptMessage, "Confirm Password: ");
        if (send(clntSocket, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
            DieWithError("send() failed");

        if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
            DieWithError("recv() failed");
        echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */
        strcpy(confirmpassword, echoBuffer);

    default:
        break;
    }

    close(clntSocket); /* Close client socket */
}
