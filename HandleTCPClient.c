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
#define MAX_ROWS 1
#define MAX_COLS 1

void InsertDataInUsers(char username[USERNAME_SIZE], char password[PASSWORD_SIZE])
{
    ConnectToDB();
    char query[200];
    sprintf(query, "insert into users(username,password) values('%s',PASSWORD('%s'));", username, password);
    printf(query);
    if (mysql_query(conn, query))
        printf("Unable to insert data into Employee table\n");
    DisconnectFromDB();
}

void SelectDataFromUsers(char username[USERNAME_SIZE], char password[PASSWORD_SIZE], char **data)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    int num_fields, i, j;

    char query[100];
    sprintf(query, "select count(*) from users where username='%s' and password=PASSWORD('%s');", username, password);
    if (mysql_query(conn, query))
    {
        DieWithError("Error in executing SQL select query");
    }

    result = mysql_store_result(conn);

    data = result;
    mysql_free_result(result);
}

void DieWithError(char *errorMessage); /* Error handling function */

void HandleTCPClient(int clntSocket)
{
    char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
    int recvMsgSize;             /* Size of received message */
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    char confirmpassword[PASSWORD_SIZE];
    char *data[MAX_ROWS * MAX_COLS];
    int attempt = 0;

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
    printf("socket %d \n", clntSocket);
    switch (atoi(echoBuffer))
    {
    case 1:
        while (true)
        {
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

            SelectDataFromUsers(username, password, data);
            if (data == 0)
            {
                if (attempt == 2)
                    DieWithError("User authentication failed limit reached.");
                attempt = attempt + 1;

                strcpy(promptMessage, "Login Attempt failed");
                if (send(clntSocket, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
                    DieWithError("send() failed");
                echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */
            }
            else
            {
                strcpy(promptMessage, "Login Attempt failed");

                if (send(clntSocket, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
                    DieWithError("send() failed");
                echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */

                printf("login successful \n");
                break;
            }
        }
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

        InsertDataInUsers(username, password);

    default:
        break;
    }

    close(clntSocket); /* Close client socket */
}

