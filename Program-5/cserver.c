#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>


#include <stdio.h>

void printPort(uint16_t listenerPort)
{
    char* stdioBuffer;
    stdioBuffer = malloc(sizeof(listenerPort) + 21);
    sprintf(stdioBuffer, "Listening Port: %d\n", listenerPort);
    write(1, stdioBuffer, strlen(stdioBuffer));
    free(stdioBuffer);
}

void listeningService(struct sockaddr_in *listenerSocket)
{
    printf("Port: %d\n", listenerSocket->sin_port);
}

void initializeListener()
{
    int listener;
    int listenerLength;
    int socketSize;
    struct sockaddr_in *listenerSocket;
    char* stdioBuffer;

    socketSize = sizeof(struct sockaddr_in);

    // Construct and clean
    listenerSocket = malloc(socketSize);
    listener = socket(AF_INET, SOCK_STREAM, 0);
    bzero((char *) listenerSocket, socketSize);

    // Set listener metadata
    listenerSocket->sin_family = (short) AF_INET;
    listenerSocket->sin_addr.s_addr = htonl(INADDR_ANY);
    listenerSocket->sin_port = htons(0);

    // Bind to socket
    bind(listener, (struct sockaddr *)listenerSocket, socketSize);
    getsockname(listener, (struct sockaddr *)listenerSocket, &socketSize);

    // Print connection information
    printPort(listenerSocket->sin_port);

    // Start listening, neverending loop
    listeningService(listenerSocket);
    free(listenerSocket);
    exit(0);
}

int main()
{
    printf("Server Start\n");
    initializeListener();
}
