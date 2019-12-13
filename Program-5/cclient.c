#include "message.h"
#include "utils.h"

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

// Global server connection
peer_t server;

void shutdownClient(int exitCode);

/*
 * printPort(uint16_t listenerPort)
 *      Prints out the passed in port to STDOUT
 *
 *  Args:
 *      listenerPort: Port to print
 */
void printPort(uint16_t listenerPort) {
  char *stdioBuffer;
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
void handleSignal(int signalNumber) {
  int fail;
  fail = 0;
  if (signalNumber == SIGINT) {
    fail = 1;
    write(STDOUT, "SIGINT Caught, Exiting.\n", 26 * sizeof(char));
  }

  // Room for more signals
  if (fail == 1) {
    shutdownClient(0);
  }
}

/*
 * setupSignalHandlers()
 *      Registers all required signal handlers for the program.
 *  Returns:
 *      -1: Error registering signal handler
 *       0: Sucess registering all handlers
 */
int setupSignalHandlers() {
  struct sigaction signalAction;
  signalAction.sa_handler = handleSignal;
  if (sigaction(SIGINT, &signalAction, 0) != 0) {
    write(STDERR, strerror(errno), strlen(strerror(errno)) * sizeof(char));
    return -1;
  }

  return 0;
}

/*
 * getUsername(char **argv, char* username)
 *      Gets username from CLI args
 *  Args:
 *      argv:       arguments passed into program from main
 *      username:   variable to copy the string into
 */
void getUsername(char **argv, char *username) {
  // Used to have more stuff
  strcpy(username, argv[3]);
}

/*
 * establishConnection(peer_t *server, char* ipAddr, char* port)
 *      Resolves the connection and established a basic stream connection,
 *      storing the data into server
 *  Args:
 *      server: peer_t to store data into
 *      ipAddr: String of the IP/DNS name to attach to
 *      port:   String of the port to connect to
 *  Returns:
 *      -1:  Error establishing connection
 *       0:  Succesfully established connection
 */
int establishConnection(peer_t *server, char *ipAddr, char *port) {
  struct addrinfo hints;
  struct addrinfo *result;
  int status;

  bzero(&hints, sizeof(hints));
  bzero(&result, sizeof(result));

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  status = getaddrinfo(ipAddr, port, &hints, &result);
  if (status != 0) {
    write(STDERR, gai_strerror(status),
          strlen((gai_strerror(status))) * sizeof(char));
    write(STDERR, "\n", sizeof(char));
    free(result);
    return -1;
  }

  // Obtain socket
  server->socket =
      socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  // Error obtaining socket
  if (server->socket < 0) {
    write(STDERR, strerror(errno), strlen(strerror(errno)) * sizeof(char));
    free(result);
    return -1;
  }

  // initiate connection
  if (connect(server->socket, result->ai_addr, result->ai_addrlen) != 0) {
    write(STDERR, strerror(errno), strlen(strerror(errno)) * sizeof(char));
    free(result);
    return -1;
  }
  free(result);
  return 0;
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
void buildFDs(peer_t *server, fd_set *readFds, fd_set *writeFDs,
              fd_set *exceptFDs) {
  FD_ZERO(readFds);
  FD_SET(STDIN_FILENO, readFds);
  FD_SET(server->socket, readFds);

  FD_ZERO(writeFDs);
  // Message to send in queue
  if (server->send_buffer.current > 0) {
    FD_SET(server->socket, writeFDs);
  }

  FD_ZERO(exceptFDs);
  FD_SET(STDIN_FILENO, exceptFDs);
  FD_SET(server->socket, exceptFDs);
}

/*
 * readInput(peer_t *server, char *username)
 *      reads input from STDIN and sends it to server
 *      if input is equal to "exit" then the client shuts down.
 *  Args:
 *      server:     Server connection to send to
 *      username: Username of client
 *  Returns:
 *      -1: Error reading from STDIN
 *       0: Success
 */
int readInput(peer_t *server, char *username) {
  char read_buffer[DATA_MAXSIZE];
  message_t newMessage;

  if (readSTDIN(read_buffer, DATA_MAXSIZE) != 0)
    return -1;

  if (strcmp(read_buffer, "exit") == 0) {
    shutdownClient(EXIT_SUCCESS);
  }
  // Create new message and enqueue it.
  prepareMessage(username, read_buffer, &newMessage);

  if (peerAdd(server, &newMessage) != 0) {
    write(STDOUT, "Message Lost.\n", 16 * sizeof(char));
    return 0;
  }

  return 0;
}

/*
 * shutdownClient(int exitCode)
 *      Shuts down the client safely
 *  Args:
 *      exitCode: Exit code to return with
 */
void shutdownClient(int exitCode) {
  deletePeer(&server);
  write(STDOUT, "Client shutdown.\n", 18 * sizeof(char));
  exit(exitCode);
}

/*
 * handleReceivedMessages(message_t *message)
 *      Prints out the messages
 *  Args:
 *      message: Message to get print contents from
 */
int handleReceivedMessages(message_t *message) {
  printMessage(message);
  return 0;
}

/*
 * maintainConnection(char* username)
 *      Maintains the socket connect and performs the primary
 *      looping to do the program stuff. Self exiting program loop.
 *  Args:
 *      username: Client's username
 */
void maintainConnection(char *username) {
  int flag;
  int maxfd;
  fd_set readFDs;
  fd_set writeFDs;
  fd_set exceptFDs;

  // Don't block STDIN
  flag = fcntl(STDIN_FILENO, F_GETFL, 0);
  flag |= O_NONBLOCK;
  fcntl(STDIN_FILENO, F_SETFL, flag);

  maxfd = server.socket;

  while (0 == 0) {
    // Reset FDs for next run
    buildFDs(&server, &readFDs, &writeFDs, &exceptFDs);

    int activity = select(maxfd + 1, &readFDs, &writeFDs, &exceptFDs, NULL);

    if (activity == -1) {
      write(STDERR, strerror(errno), strlen(strerror(errno)) * sizeof(char));
      shutdownClient(EXIT_FAILURE);
    } else if (activity == 0) {
      write(STDERR, strerror(errno), strlen(strerror(errno)) * sizeof(char));
      shutdownClient(EXIT_FAILURE);
    }

    // Read STDIO input
    if (FD_ISSET(STDIN_FILENO, &readFDs)) {
      if (readInput(&server, username) != 0) {
        shutdownClient(EXIT_FAILURE);
      }
    }

    // Exception called
    if (FD_ISSET(STDIN_FILENO, &exceptFDs)) {
      write(STDERR, "Exception hit on STDIN\n", 25 * sizeof(char));
      shutdownClient(EXIT_FAILURE);
    }

    // Receive message
    if (FD_ISSET(server.socket, &readFDs)) {
      if (receivePeerMsg(&server, &handleReceivedMessages) != 0)
        shutdownClient(EXIT_FAILURE);
    }

    // Message to send
    if (FD_ISSET(server.socket, &writeFDs)) {
      if (sendPeerMsg(&server) != 0)
        shutdownClient(EXIT_FAILURE);
    }

    // Exception on server
    if (FD_ISSET(server.socket, &exceptFDs)) {
      write(STDERR, "Exception hit on server\n", 26 * sizeof(char));
      shutdownClient(EXIT_FAILURE);
    }
  }
}

/*
 * printHelp()
 *      Print basic help menu
 */
void printHelp() {
  write(STDOUT, "Usage: cclient <remote> <port> <username>\n", 42);
}

/*
 * main(int argc, char **argv)
 *      main method of program
 *  Args:
 *      argc: Number of Args
 *      argv: Arg array
 *  Returns:
 *      Program return code
 */
int main(int argc, char **argv) {
  char username[MAX_USERNAME_SIZE];
  char *buffer;
  if (argc != 4) {
    printHelp();
    exit(EXIT_SUCCESS);
  }
  // Setup Handlers
  if (setupSignalHandlers() != 0)
    exit(EXIT_FAILURE);

  // Get name
  getUsername(argv, username);

  // Start
  buffer = malloc(sizeof(char) * (strlen(username) + 30));
  sprintf(buffer, "Starting client as user: [%s]\n", username);
  write(STDOUT, buffer, sizeof(char) * (strlen(username) + 30));
  free(buffer);
  createPeer(&server);
  if (establishConnection(&server, argv[1], argv[2]) != 0)
    shutdownClient(EXIT_FAILURE);

  maintainConnection(username);
}
