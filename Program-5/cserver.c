#include "utils.h"
#include "protocol.h"

#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>

void printPort(uint16_t listenerPort)
{
    char* stdioBuffer;
    stdioBuffer = malloc(sizeof(listenerPort) + 21);
    sprintf(stdioBuffer, "Listening Port: %d\n", ntohs(listenerPort));
    write(STDOUT, stdioBuffer, strlen(stdioBuffer));
    free(stdioBuffer);
}

void listeningService(struct sockaddr_in listenerSocket, int listener)
{
    char exit;
    int connection;
    //TODO: remove with array
    struct sockaddr_in s2;
    int ch;
    int length;
    // per new connection
    struct timeval timeout;
    fd_set readfds;
    int returnValue;
    msgProto *readBuffer;

    exit = 0;
    listen(listener, 1);
    // Loop til exit
    while (exit == 0)
    {
        connection = accept(listener, (struct sockaddr *)&s2, &length);
        printf("established connection\n");

        // TODO: probs fork around hereish
        while(exit == 0)
        {
            timeout.tv_sec = 0;
            timeout.tv_usec = 1000; // 1ms
            FD_ZERO(&readfds);
            FD_SET(connection, &readfds);

            // Checks socket
            if (select(connection + 1, &readfds, NULL, NULL, &timeout) == -1)
            {
                write(STDERR, strerror(errno),
                        strlen(strerror(errno)) * sizeof(char));
                break;
            }

            // Receive data if it exits
            if (FD_ISSET(connection, &readfds))
            {
                // TODO: struct me
                readBuffer = malloc(sizeof(msgProto));
                returnValue = recv(connection, readBuffer, sizeof(msgProto), 0);
                if (returnValue <= 0)
                {
                    // Error in reading or broke pipe
                    break;
                }

                write(STDOUT, readBuffer->userName,
                        strlen(readBuffer->userName) * sizeof(char));
                write(STDOUT, " ", sizeof(char));
                write(STDOUT, readBuffer->msg,
                        strlen(readBuffer->msg) * sizeof(char));
                write(STDOUT, "\n", sizeof(char));
            }

            // Check for sending data
            if (1 == 0)
            {

            }


        }
    }
    close(connection);

}

void printerror(int err)
{
    // TODO:
}

void initializeListener()
{

    int listener;
    int listenerLength;
    int status;
    struct sockaddr_in listenerSocket;

    // Construct and clean
    listener = socket(AF_INET, SOCK_STREAM, 0);
    bzero((char *) &listenerSocket, sizeof(listenerSocket));

    // Set listener metadata
    listenerSocket.sin_family = (short) AF_INET;
    listenerSocket.sin_addr.s_addr = htonl(INADDR_ANY);
    listenerSocket.sin_port = htons(0);
    // Bind to socket
    status = bind(listener, (struct sockaddr *)&listenerSocket,
            sizeof(listenerSocket));
    if (status == -1)
    {
        printerror(errno);
    }
    listenerLength = sizeof(listenerSocket);
    status = getsockname(listener, (struct sockaddr *)&listenerSocket,
            &listenerLength);
    if (status == -1)
    {
        printerror(errno);
    }

    printPort(listenerSocket.sin_port);
    listeningService(listenerSocket, listener);
}

int main()
{
    printf("Server Start\n");
    initializeListener();
}
