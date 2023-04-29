#include "TCPEchoServer.h" /* TCP echo server includes */
#include <pthread.h>       /* for POSIX threads */
#include <mysql/mysql.h> /* for MySQL connection */


MYSQL *conn; /* MySQL connection handle */

/* MySQL table creation query */
const char *CREATE_TABLE_QUERY = "CREATE TABLE IF NOT EXISTS users (id INT AUTO_INCREMENT PRIMARY KEY, username VARCHAR(255) NOT NULL, password VARCHAR(255) NOT NULL)";

void *ThreadMain(void *arg); /* Main program of a thread */

/* Structure of arguments to pass to client thread */
struct ThreadArgs {
  int clntSock; /* Socket descriptor for client */
};


/* Connect to MySQL database */
void ConnectToDB()
{
    /* Initialize MySQL connection */
    conn = mysql_init(NULL);
    if (conn == NULL)
        DieWithError("mysql_init() failed");

    /* Connect to MySQL database */
    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL)
        DieWithError(mysql_error(conn));

    printf("Connected to MySQL database %s\n", DB_NAME);
}

/* Disconnect from MySQL database */
void DisconnectFromDB()
{
    /* Close MySQL connection */
    mysql_close(conn);
    printf("Disconnected from MySQL database %s\n", DB_NAME);
}


/* Create users table in MySQL database */
void CreateUsersTable()
{
    if (mysql_query(conn, CREATE_TABLE_QUERY) != 0)
        DieWithError(mysql_error(conn));

    printf("Created table 'users' in MySQL database %s\n", DB_NAME);
}



int main(int argc, char *argv[]) {
  int servSock;                  /* Socket descriptor for server */
  int clntSock;                  /* Socket descriptor for client */
  unsigned short echoServPort;   /* Server port */
  pthread_t threadID;            /* Thread ID from pthread_create() */
  struct ThreadArgs *threadArgs; /* Pointer to argument structure for thread */

  if (argc != 2) /* Test for correct number of arguments */
  {
    fprintf(stderr, "Usage:  %s <SERVER PORT>\n", argv[0]);
    exit(1);
  }

  echoServPort = atoi(argv[1]); /* First arg:  local port */

  servSock = CreateTCPServerSocket(echoServPort);

  ConnectToDB();
  CreateUsersTable();

  for (;;) /* run forever */
  {
    printf("waiting for connection ....\n");
    clntSock = AcceptTCPConnection(servSock);

    /* Create separate memory for client argument */
    if ((threadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs))) ==
        NULL)
      DieWithError("malloc() failed");
    threadArgs->clntSock = clntSock;

    /* Create client thread */
    if (pthread_create(&threadID, NULL, ThreadMain, (void *)threadArgs) != 0)
      DieWithError("pthread_create() failed");
    printf("with thread %ld\n", (long int)threadID);
  }
  printf("at the end of main thread ....\n");
  /* NOT REACHED */
}

void *ThreadMain(void *threadArgs) {
  int clntSock; /* Socket descriptor for client connection */

  /* Guarantees that thread resources are deallocated upon return */
  pthread_detach(pthread_self());

  /* Extract socket file descriptor from argument */
  clntSock = ((struct ThreadArgs *)threadArgs)->clntSock;
  free(threadArgs); /* Deallocate memory for argument */
  HandleTCPClient(clntSock);
    
  return (NULL);
}

