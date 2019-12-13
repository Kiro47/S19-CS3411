// Simple example of server with select() and multiple clients.

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>

#include "message.h"
#include "utils.h"

// Mostly to prevent overflow issues
#define MAX_CLIENTS 256

#define NO_SOCKET -1

#define SERVER_NAME "server"

// global vars
int listener_socket;
peer_t connections[MAX_CLIENTS];
// stdin buffer
char readBuffer[1024];

// prototypes
void shutdownServer(int code);

/*
 * printPort(uint16_t listenerPort)
 *      Prints out the passed in port to STDOUT
 *
 *  Args:
 *      listenerPort: Port to print
 */
void printPort(uint16_t listenerPort)
{
    char* stdioBuffer;
    stdioBuffer = malloc(sizeof(listenerPort) + 21);
    sprintf(stdioBuffer, "Listening Port: %d\n", ntohs(listenerPort));
    write(STDOUT, stdioBuffer, strlen(stdioBuffer) * sizeof(char));
    free(stdioBuffer);
}

/*
 * handleSignal(int sigNumber)
 *      Performs actions for the various signals given, handler binding.
 *      This function as the potential to shutdown the program based on
 *      the signal received.
 *  Args:
 *      signalNumber: Signal received
 */
void handleSignalAction(int sig_number)
{
    int fail;
    fail = 0;
    if (sig_number == SIGINT) {
        fail = 1;
        write(STDOUT, "SIGINT Caught, Exiting.\n", 26 * sizeof(char));
    }

    // Room for more signals
    if (fail == 1)
    {
        shutdownServer(0);
    }
}

/*
 * setupSignalHandlers()
 *      Registers all required signal handlers for the program.
 *  Returns:
 *      -1: Error registering signal handler
 *       0: Sucess registering all handlers
 */
int setupSignalHandlers()
{
  struct sigaction sa;
  sa.sa_handler = handleSignalAction;
  if (sigaction(SIGINT, &sa, 0) != 0) {
      write(STDERR, strerror(errno), strlen(strerror(errno)) * sizeof(char));
    return -1;
  }

  return 0;
}

/*
 * startListener(int *listener)
 *      Allocates receiving socket and receives a randomized port
 *  Args:
 *      listener: empty integer to store socket listener File descriptor in
 *  Returns:
 *      -1: Failed in some way
 *       0: Success
 */
int startListener(int *listener)
{
    int listenerLength;
    struct sockaddr_in listenerSocket;

    // Obtain a file descriptor for our "listening" socket.
  *listener = socket(AF_INET, SOCK_STREAM, 0);
  if (*listener < 0) {
      write(STDERR, strerror(errno), strlen(strerror(errno)) * sizeof(char));
    return -1;
  }

  bzero((char *)&listenerSocket, sizeof(listenerSocket));
  listenerSocket.sin_family = (short) AF_INET;
  listenerSocket.sin_addr.s_addr = htonl(INADDR_ANY);
  listenerSocket.sin_port = htons(0);

  if (bind(*listener, (struct sockaddr*)&listenerSocket, sizeof(struct sockaddr)) != 0) {
      write(STDERR, strerror(errno), strlen(strerror(errno)) * sizeof(char));
    return -1;
  }

  listenerLength = sizeof(struct sockaddr);
    if (getsockname(*listener, (struct sockaddr *) &listenerSocket, &listenerLength)) {
        write(STDERR, strerror(errno), strlen(strerror(errno)) * sizeof(char));
        return -1;
    }

  // start accept client connections
  if (listen(*listener, 10) != 0) {
      write(STDERR, strerror(errno), strlen(strerror(errno)) * sizeof(char));
    return -1;
  }

  printPort(listenerSocket.sin_port);

    return 0;
}

/*
 * shutdownClient(int exitCode)
 *      Shuts down the server safely and disconnects clients
 *  Args:
 *      exitCode: Exit code to return with
 */
void shutdownServer(int code)
{
  int i;

  close(listener_socket);

  for (i = 0; i < MAX_CLIENTS; ++i)
    if (connections[i].socket != NO_SOCKET)
      close(connections[i].socket);

    for (i = 0; i < MAX_CLIENTS; ++i) {
        free(connections[i].send_buffer.data);
    }

    write(STDOUT, "Server shutdown.\n", sizeof(char) * 19);
  exit(code);
}

/*
 * buildFDs(peer_t *server, fd_set *readFds, fd_set *writeFDs,
 *              fd_set *exceptFDs)
 *      Builds file descriptor sets for usage with select()
 *  Args:
 *      server:     Server connection established
 *      readFDs:    file descriptors to read with
 *      writeFDs:   file descriptors to write with
 *      exceptFDs:  file descriptors to deal with exceptions
 */
int buildFDs(fd_set *read_fds, fd_set *write_fds, fd_set *except_fds)
{
  int i;

  FD_ZERO(read_fds);
  FD_SET(STDIN_FILENO, read_fds);
  FD_SET(listener_socket, read_fds);
  for (i = 0; i < MAX_CLIENTS; ++i)
    if (connections[i].socket != NO_SOCKET)
      FD_SET(connections[i].socket, read_fds);

  FD_ZERO(write_fds);
  for (i = 0; i < MAX_CLIENTS; ++i)
    if (connections[i].socket != NO_SOCKET && connections[i].send_buffer.current > 0)
      FD_SET(connections[i].socket, write_fds);

  FD_ZERO(except_fds);
  FD_SET(STDIN_FILENO, except_fds);
  FD_SET(listener_socket, except_fds);
  for (i = 0; i < MAX_CLIENTS; ++i)
    if (connections[i].socket != NO_SOCKET)
      FD_SET(connections[i].socket, except_fds);

  return 0;
}

/*
 *  newConnection
 *      Established a  new connection on the next available socket
 *   Returns:
 *      -1: Error occurred setting up new socket
 *       0: Success
 */
int newConnection()
{
  struct sockaddr_in clientAddr;
    int newClientSocket;
    char client_ipv4_str[INET_ADDRSTRLEN];
    int i;
    char *writeBuffer;


    memset(&clientAddr, 0, sizeof(clientAddr));
  socklen_t clientLength = sizeof(clientAddr);
    newClientSocket = accept(listener_socket, (struct sockaddr *)&clientAddr, &clientLength);
  if (newClientSocket < 0) {
      write(STDERR, strerror(errno), strlen(strerror(errno)) * sizeof(char));
    return -1;
  }

  inet_ntop(AF_INET, &clientAddr.sin_addr, client_ipv4_str, INET_ADDRSTRLEN);

  writeBuffer = malloc(sizeof(clientAddr.sin_port) + (sizeof(char) * (strlen(client_ipv4_str) + 32)) );
  sprintf(writeBuffer,"Incoming connection from %s:%d.\n", client_ipv4_str, clientAddr.sin_port);
  write(STDOUT, writeBuffer, sizeof(clientAddr.sin_port) + (sizeof(char) * (strlen(client_ipv4_str) + 32)) );
  free(writeBuffer);

  for (i = 0; i < MAX_CLIENTS; ++i) {
    if (connections[i].socket == NO_SOCKET) {
        connections[i].socket = newClientSocket;
        connections[i].addres = clientAddr;
        connections[i].current_sending_byte   = -1;
        connections[i].current_receiving_byte = 0;
      return 0;
    }
  }
    writeBuffer = malloc(sizeof(clientAddr.sin_port) + (sizeof(char) * (strlen(client_ipv4_str) + 37)) );
    sprintf(writeBuffer,"Too many connections, closing %s:%d.\n", client_ipv4_str, clientAddr.sin_port);
    write(STDOUT, writeBuffer, sizeof(clientAddr.sin_port) + (sizeof(char) * (strlen(client_ipv4_str) + 37)) );
    free(writeBuffer);
  close(newClientSocket);
  return -1;
}

/*
 * closeClientConnection(peer_t *client)
 *      Closes the clients connection and resets information.
 *  Args:
 *      client: Client connection to close.
 */
int closeClientConnection(peer_t *client)
{
  char* writeBuffer;
  writeBuffer = malloc((sizeof(char) * (32)) + sizeof(peerGetAddr(client)));
  sprintf(writeBuffer, "Close client socket for %s.\n", peerGetAddr(client));
  write(STDOUT, writeBuffer, (sizeof(char) * (32)) + sizeof(peerGetAddr(client)) );
  free(writeBuffer);
  close(client->socket);
  client->socket = NO_SOCKET;
    dequeueAll(&client->send_buffer);
  client->current_sending_byte   = -1;
  client->current_receiving_byte = 0;
}

/*
 * readin()
 *      reads input from STDIN and sends it to all clients
 *      if input is equal to "exit" then the server shuts down.
 *  Args:
 *      server:     Server connection to send to
 *      username: Username of client
 *  Returns:
 *      -1: Error reading from STDIN
 *       0: Success
 */int readin()
{
    int i;
    char readBuffer[DATA_MAXSIZE];
    message_t newMsg;

    if (readSTDIN(readBuffer, DATA_MAXSIZE) != 0)
    return -1;

  // Create new message and enqueue it.
    prepareMessage(SERVER_NAME, readBuffer, &newMsg);
    printMessage(&newMsg);

  /* enqueue message for all clients */
  for (i = 0; i < MAX_CLIENTS; ++i) {
    if (connections[i].socket != NO_SOCKET) {
      if (peerAdd(&connections[i], &newMsg) != 0) {
        continue;
      }
    }
  }

  return 0;
}

/*
 * handleReceived(message_t *message)
 *      Receives a message and sends it to all clients
 *  Args:
 *      message: Message to send
 *  Returns:
 *      0: Possible Success
 */
int handleReceived(message_t *message)
{
    printMessage(message);
  return 0;
}

/*
 *  receiveServerMsg(peer_t *peer, int (*message_handler)(message_t *))
 *      Primary receive function, handles all messages sent to the server and
 *      processes them
 *  Args:
 *      peer: Peer connection receiving from
 *      message_t: Localized method function address of the message_handler
 *  Returns:
 *      -1: Failure
 *       0: Success
 */
int receiveServerMsg(peer_t *peer, int (*message_handler)(message_t *))
{
    size_t len_to_receive;
    ssize_t received_count;
    int i;

    size_t received_total = 0;
    do {
        // Is completely received?
        if (peer->current_receiving_byte >= sizeof(peer->receiving_buffer)) {
            message_handler(&peer->receiving_buffer);
            peer->current_receiving_byte = 0;
        }

        // Count bytes to send.
        len_to_receive = sizeof(peer->receiving_buffer) - peer->current_receiving_byte;
        if (len_to_receive > MAX_SEND_SIZE)
            len_to_receive = MAX_SEND_SIZE;

        received_count = recv(peer->socket, (char *)&peer->receiving_buffer + peer->current_receiving_byte, len_to_receive, MSG_DONTWAIT);
        if (received_count < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
            }
            else {
                write(STDERR, strerror(errno), strlen(strerror(errno)) * sizeof(char));
                return -1;
            }
        }
        else if (received_count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            break;
        }
            // If recv() returns 0, it means that peer gracefully shutdown. Shutdown client.
        else if (received_count == 0) {
            return -1;
        }
        else if (received_count > 0) {
            peer->current_receiving_byte += received_count;
            received_total += received_count;
        }
    } while (received_count > 0);

    // Send to the rest of the clients
    for (i = 0; i < MAX_CLIENTS; ++i) {
        if (connections[i].socket != NO_SOCKET) {
            if (connections[i].socket != peer->socket)
            {

                if (peerAdd(&connections[i], &peer->receiving_buffer) != 0) {
                    continue;
                }
            }
        }
    }
    return 0;
}

/*
 * main(int argc, char **argv)
 *      main method of program, handles the endless receiving loop
 *  Args:
 *      argc: Number of Args
 *      argv: Arg array
 *  Returns:
 *      Program return code
 */
int main(int argc, char **argv)
{
    int i;
    int flag;
    int highestSocket;
    int activity;

    fd_set read_fds;
    fd_set write_fds;
    fd_set except_fds;

    if (setupSignalHandlers() != 0)
    exit(EXIT_FAILURE);

  if (startListener(&listener_socket) != 0)
    exit(EXIT_FAILURE);

  /* Set nonblock for stdin. */
   flag = fcntl(STDIN_FILENO, F_GETFL, 0);
  flag |= O_NONBLOCK;
  fcntl(STDIN_FILENO, F_SETFL, flag);

  for (i = 0; i < MAX_CLIENTS; ++i) {
      connections[i].socket = NO_SOCKET;
    createPeer(&connections[i]);
  }

  highestSocket = listener_socket;

  while (1) {
      buildFDs(&read_fds, &write_fds, &except_fds);

    highestSocket = listener_socket;
    for (i = 0; i < MAX_CLIENTS; ++i) {
      if (connections[i].socket > highestSocket)
        highestSocket = connections[i].socket;
    }

    activity = select(highestSocket + 1, &read_fds, &write_fds, &except_fds, NULL);

    switch (activity) {
      case -1:
          write(STDERR, strerror(errno), strlen(strerror(errno)) * sizeof(char));
            shutdownServer(EXIT_FAILURE);

      case 0:
        // you should never get here
            shutdownServer(EXIT_FAILURE);

      default:
        /* All set fds should be checked. */
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
          if (readin() != 0)
              shutdownServer(EXIT_FAILURE);
        }

        if (FD_ISSET(listener_socket, &read_fds)) {
          newConnection();
        }

        if (FD_ISSET(STDIN_FILENO, &except_fds)) {
            shutdownServer(EXIT_FAILURE);
        }

        if (FD_ISSET(listener_socket, &except_fds)) {
            shutdownServer(EXIT_FAILURE);
        }

        for (i = 0; i < MAX_CLIENTS; ++i) {
          if (connections[i].socket != NO_SOCKET && FD_ISSET(connections[i].socket, &read_fds)) {
            if (receiveServerMsg(&connections[i], &handleReceived) != 0) {
                closeClientConnection(&connections[i]);
              continue;
            }
          }

          if (connections[i].socket != NO_SOCKET && FD_ISSET(connections[i].socket, &write_fds)) {
            if (sendPeerMsg(&connections[i]) != 0) {
                closeClientConnection(&connections[i]);
              continue;
            }
          }

          if (connections[i].socket != NO_SOCKET && FD_ISSET(connections[i].socket, &except_fds)) {
              closeClientConnection(&connections[i]);
            continue;
          }
        }
    }
  }

  return 0;
}
