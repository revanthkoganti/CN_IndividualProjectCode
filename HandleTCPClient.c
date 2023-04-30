#include "TCPEchoServer.h" /* TCP echo server includes */
#include <stdio.h>         /* for printf() and fprintf() */
#include <sys/socket.h>    /* for recv() and send() */
#include <unistd.h>        /* for close() */
#include <string.h>
#include <mysql/mysql.h> /* for MySQL connection */
#include <openssl/sha.h>

MYSQL *conn; /* MySQL connection handle */

#define RCVBUFSIZE 250 /* Size of receive buffer */
#define USERNAME_SIZE 100
#define PASSWORD_SIZE 100
#define MAX_ROWS 100
#define MAX_COLS 100

void InsertDataInUsers(char username[USERNAME_SIZE], char password[PASSWORD_SIZE], char clntAddr[20])
{
    char hex_hashed_password[2 * SHA256_DIGEST_LENGTH + 1];
    HashPassword(password, hex_hashed_password);

    ConnectToDB();
    char query[200];
    sprintf(query, "insert into users(username,password,ipaddress,isactive) values('%s','%s', '%s',FALSE);", username, hex_hashed_password, clntAddr);
    printf("%s \n", query);
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
    sprintf(query, "select password from users where username='%s';", username);
    printf("%s \n", query);
    if (mysql_query(conn, query))
    {
        DieWithError("Error in executing SQL select query");
    }

    result = mysql_store_result(conn);

    if (result)
    {
        if ((row = mysql_fetch_row(result)))
        {
            *data = row[0];
        }
    }
    else
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    mysql_free_result(result);
}
void HashPassword(char password[PASSWORD_SIZE], char *hex_hashed_password)
{
    unsigned char hashed_password[SHA256_DIGEST_LENGTH];

    char *password_to_hash = password;
    SHA256((unsigned char *)password_to_hash, strlen(password_to_hash), hashed_password);

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(hex_hashed_password + 2 * i, "%02x", hashed_password[i]);
    }
}
void SetUserActive(char username[USERNAME_SIZE])
{
    ConnectToDB();
    char query[200];
    sprintf(query, "update users set isactive=TRUE where username='%s';", username);
    printf("%s \n", query);
    if (mysql_query(conn, query))
        printf("Unable to insert data into Employee table\n");
    DisconnectFromDB();
}
void ShowActiveClients(int clntSocket, char echoBuffer[RCVBUFSIZE], char **activeUsers)
{

    MYSQL_RES *result;
    MYSQL_ROW row;
    int num_fields, i, j;
    char query[100];
    ConnectToDB();
    sprintf(query, "select username from users where isactive=TRUE;");

    printf("%s \n", query);
    if (mysql_query(conn, query))
    {
        DieWithError("Error in executing SQL select query");
    }
    printf("Executing SQL");
    result = mysql_store_result(conn);

    if (result)
    {
        if ((row = mysql_fetch_row(result)))
        {
            *activeUsers = row[0];
        }
    }
    else
    {
        printf("error in sql \n");
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    mysql_free_result(result);
    DisconnectFromDB();
}

void DieWithError(char *errorMessage); /* Error handling function */

void BroadcastSender()
{
    int sock;                         /* Socket */
    struct sockaddr_in broadcastAddr; /* Broadcast address */
    char *broadcastIP;                /* IP broadcast address */
    unsigned short broadcastPort;     /* Server port */
    char *sendString;                 /* String to broadcast */
    int broadcastPermission;          /* Socket opt to set permission to broadcast */
    unsigned int sendStringLen;       /* Length of string to broadcast */

    broadcastIP = "127.0.0.1"; /* First arg:  broadcast IP address */
    broadcastPort = 4400;      /* Second arg:  broadcast port */
    sendString = "Hello";      /* Third arg:  string to broadcast */

    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Set socket to allow broadcast */
    broadcastPermission = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&broadcastPermission,
                   sizeof(broadcastPermission)) < 0)
        DieWithError("setsockopt() failed");

    /* Construct local address structure */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));       /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;                     /* Internet address family */
    broadcastAddr.sin_addr.s_addr = inet_addr(broadcastIP); /* Broadcast IP address */
    broadcastAddr.sin_port = htons(broadcastPort);          /* Broadcast port */

    sendStringLen = strlen(sendString); /* Find length of sendString */
    for (;;)                            /* Run forever */
    {
        /* Broadcast sendString in datagram to clients every 3 seconds*/
        if (sendto(sock, sendString, sendStringLen, 0, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr)) != sendStringLen)
            DieWithError("sendto() sent a different number of bytes than expected");

        sleep(3); /* Avoids flooding the network */
    }
    /* NOT REACHED */
}

void HandleTCPClient(int clntSocket, char clntAddr[20])
{
    char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
    int recvMsgSize;             /* Size of received message */
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    char confirmpassword[PASSWORD_SIZE];
    char *data[MAX_ROWS * MAX_COLS];
    int attempt = 0;
    char hashed_password[PASSWORD_SIZE];
    char hex_hashed_password[2 * SHA256_DIGEST_LENGTH + 1];
    char choice[10];
    char *activeUsers[MAX_ROWS * MAX_COLS];

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
    strcpy(choice, echoBuffer);
Login:
    printf("Label reached");
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

            if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
                DieWithError("recv() failed");
            echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */

            strcpy(password, echoBuffer);

            SelectDataFromUsers(username, password, data);
            printf("Hashed password: %s\n", *data);

            HashPassword(password, hex_hashed_password);
            if (strcmp(hex_hashed_password, *data) == 0)
            {
                /*Send Login success message to client*/
                strcpy(promptMessage, "LoginSuccess");
                if (send(clntSocket, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
                    DieWithError("send() failed");
                SetUserActive(username);

                if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
                    DieWithError("recv() failed");
                echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */
                switch (atoi(echoBuffer))
                {
                case 1:
                    printf("Entered Switch \n");
                    ShowActiveClients(clntSocket, echoBuffer, activeUsers);
                    printf("%s", activeUsers);
                    strcpy(promptMessage, "ActiveClients");
                    if (send(clntSocket, *activeUsers, sizeof(*activeUsers), 0) != sizeof(*activeUsers))
                        DieWithError("send() failed");

                    break;
                case 2:
                    printf("selected 2");

                    break;
                default:
                    break;
                }
                break;
            }
            else
            {
                if (attempt == 2)
                    DieWithError("User authentication failed limit reached.");
                attempt = attempt + 1;

                strcpy(promptMessage, "Login Attempt failed");
                if (send(clntSocket, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
                    DieWithError("send() failed");
                echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */
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

        InsertDataInUsers(username, password, clntAddr);

        strcpy(promptMessage, "Signup Successful Please login to continue");
        if (send(clntSocket, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
            DieWithError("send() failed");
        strcpy(choice, "1");
        goto Login;
        break;
    default:
        break;
    }

    close(clntSocket); /* Close client socket */
}
