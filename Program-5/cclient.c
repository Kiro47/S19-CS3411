#include "cclient.h"
#include "protocol.h"
#include "utils.h"

#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

void printHelp()
{
    write(1, "Usage: cclient <remote> <port>\n", 32 );
}

/*
 * resolveConnection(char* hostname, char* port)
 *      Takes a hostname and port to establish a TCP connection on.
 *      Checks for and validates the hostname resolution and port first.
 *  args:
 *      hostname: The hostname to check for in IPV4 or DNS name
 *      port: The port to connect to (as a string)
 *
 *  Returns:
 *      Connection of the established stream, NULL if connection failed.
 *      This is allocated memory that needs to be freed if not NULL.
 */
conn_t *resolveConnection(char* hostname, char* port)
{
    struct addrinfo hints;
    struct addrinfo *result;
    int status;
    conn_t *serverConnection;

    memset(&hints, 0, sizeof(hints));
    memset(&result, 0, sizeof(result));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    serverConnection = (conn_t*) malloc(sizeof(conn_t));

    // Resolve information
    status = getaddrinfo(hostname, port, &hints, &result);
    if (status != 0)
    {
        write(STDERR, gai_strerror(status),
                strlen((gai_strerror(status))) * sizeof(char));
        write(STDERR, "\n", sizeof(char));
        free(serverConnection);
        free(result);
        return NULL;
    }

    // Establish connection
    serverConnection->sockfd = socket(result->ai_family, result->ai_socktype,
            result->ai_protocol);

    status = connect(serverConnection->sockfd, result->ai_addr,
            result->ai_addrlen);
    free(result);
    if (status != 0)
    {
        write(STDERR, strerror(errno), strlen(strerror(errno)) * sizeof(char));
        write(STDERR, "\n", sizeof(char));
        free(serverConnection);
        return NULL;
    }
    // Return the connection
    return serverConnection;
}

void disconnect(conn_t *connection)
{

}

// TODO: redefine for struct
int sendData(conn_t *connection, msgProto *msg)
{
    int returnValue;
    char* testString = "This is a test string";
    char* sendString;

    returnValue = 0;
    // Send data
    returnValue = send(connection->sockfd, msg, sizeof(msgProto), 0);

    if (returnValue >= 0)
    {
//        printf("%d\n", returnValue);
        // send returnValue * (char) data
        // or sent nothing
        returnValue = 0;
    }
    sleep(3);
//    printf("%d\n", returnValue);

    return returnValue;
}

int receiveData(conn_t *connection)
{
    int returnValue;
    msgProto *readBuffer;
    readBuffer = malloc(sizeof(msgProto));
    returnValue = recv(connection->sockfd, readBuffer, sizeof(msgProto), 0);

    if (returnValue <= 0)
    {
        // Error in reading or broke pipe
        return -1;
    }
    write(STDOUT, readBuffer->userName,
            strlen(readBuffer->userName) * sizeof(char));
    write(STDOUT, " ", sizeof(char));
    write(STDOUT, readBuffer->msg,
            strlen(readBuffer->userName) * sizeof(char));
    write(STDOUT, "\n", sizeof(char));
}

int establishedConnection(conn_t *connection, int userPipe)
{
    // All
    struct timeval timeout;
    fd_set readfds;
    int returnValue;
    int readValue;

    returnValue = 0;
    msgProto *msg = malloc(sizeof(msgProto));

    // Received
    // Sending

//    requestUsername();

    while (1 == 1)
    {
        // Update every 1 ms
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000;
        FD_ZERO(&readfds);
        FD_SET(connection->sockfd, &readfds);

        if (select(connection->sockfd + 1, &readfds, NULL, NULL, &timeout)==-1)
        {
            // error
            write(STDERR, strerror(errno), strlen(strerror(errno)) * sizeof(char));
            break;
        }

        // Receive data, if it exists
        if (FD_ISSET(connection->sockfd, &readfds))
        {
            //TODO finish this
            receiveData(connection);
        }

        // Send data if any exists
        // TODO: read struct
        readValue = read(connection->sockfd, msg, sizeof(msgProto));
        printf("Read: %d\n", readValue);
        if (readValue != -1)
        {
            // TODO: actually send stuff right.
            msg->connType = SERRECV;
            strcpy(msg->userName, "testuser");
            strcpy(msg->msg, "test message");
            msg->channel = 0;
            printf("help\n");
            returnValue = sendData(connection, msg);
            if (returnValue != 0)
            {
                break;
            }
        }
    }
    free(msg);
    disconnect(connection);
    return returnValue;
}

char* readInput()
{
    // Get user input
    char* userInput;

    userInput = malloc(sizeof(char) * MAX_MSG_SIZE);
    userInput = input(userInput);

    if (strlen(userInput) >= MAX_MSG_SIZE)
    {
        // We're just going to clip any messages over the max
        userInput[MAX_MSG_SIZE - 1] = '\0';
    }
    return userInput;
}

char* getUsername()
{
    char* userInput;

    write(STDOUT, "What would you like your username to be? ",
            sizeof(char) * 42);

    userInput = malloc(sizeof(char) * MAX_USERNAME_SIZE);
    userInput = input(userInput);

    if (strlen(userInput) >= MAX_USERNAME_SIZE)
    {
        userInput[MAX_USERNAME_SIZE] = '\0';
    }
    return userInput;
}

char* constructPrompt(char* username)
{
    char* prompt;
    int i, j;

    prompt = malloc(sizeof(char) * (MAX_USERNAME_SIZE + 4));
    i =0;
    prompt[i++] = '<';
    for (j = 0; j < strlen(username); j++)
    {
        // Shouldn't ever happen, but prevention
        if (username[j] == '\0')
        {
            break;
        }
        prompt[i++] = username[j];
    }
    prompt[i++] = '>';
    prompt[i++] = ' ';
    prompt[i] = '\0';
    return prompt;
}


void launchUser(int writePipe)
{
    char* username;
    char* prompt;
    char* input;
    msgProto *tempProto;

    username = getUsername();
    prompt = constructPrompt(username);

    tempProto = malloc(sizeof(msgProto));
    printf("%s\n", username);

    while(1 == 1)
    {
        // Ask user for input
        write(STDOUT, prompt, strlen(prompt) * sizeof(char));
        // Keep looking for read input
        input = readInput();
        tempProto->connType = SERRECV;
        strcpy(tempProto->userName, username);
        strcpy(tempProto->msg, input);
        tempProto->channel = 0;
        printf("writing\n");
        write(writePipe, tempProto, sizeof(msgProto));

        bzero(tempProto, sizeof(msgProto));
        free(input);
    }
    free(username);
    free(prompt);
    free(tempProto);
}

void genericHandler(int sig)
{
    return;
}

int main(int argc, char** argv)
{
    pid_t pid;
    int pipes[2];

    struct sigaction newTTYHandler;
    struct sigaction oldTTYHandler;

//    key_t key;
//    int shmid;
//    int mutex; // close enough for this

//    key = ftok("test", 'R');
//    shmid = shmget(key, sizeof(int), 0600 | IPC_CREATE);
//    mutex = shmat(shmid, NULL, 0);

    conn_t *connection;
    printf("client\n");

    if (argc != 3)
    {
        printHelp(argv[0]);
        return 0;
    }
    // Resolve server
    connection = resolveConnection(argv[1], argv[2]);
    if (connection == NULL)
    {
        write(STDERR, "Error establishing connection.\n", 32);
        return -1;
    }

    // Establish pipe
    pipe2(pipes, O_NONBLOCK|O_CLOEXEC);
    // Store and wipe TTY sighandler
    memset(&newTTYHandler, 0, sizeof(struct sigaction));
    sigemptyset(&newTTYHandler.sa_mask);
//    newHandler.sa_handler = SIG_TTIN;
    sigaction(SIGTTIN, &newTTYHandler, &oldTTYHandler);

    // Begin processing
    pid = fork();
    if (pid == 0)
    {
        // Restore TTY input
        sigaction(SIGTTIN, &oldTTYHandler, NULL);
        // Close read on pipe
        close(pipes[0]);
        // Launch child reader
        launchUser(pipes[1]);
    }
    else
    {
        close(STDIN);
        // Close write on pipe
        close(pipes[1]);
        // Establish networking maintence
        printf("tf");
        establishedConnection(connection, pipes[0]);
        free(connection);
    }
    return 0;
}
