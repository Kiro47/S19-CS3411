#ifndef PROGRAM_5_MESSAGE_H
#define PROGRAM_5_MESSAGE_H

/*
 * I spent 30 mimutes trying to refactor this dumpster fire into multiple files, even just .c and .h
 * Lots of compiler errors, can't figure out why.  I don't care enough tonight to fix it.
 */

#include "message.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Per packet
#define MAX_SEND_SIZE 100

// Max queue size
#define MAX_MESSAGES_BUFFER_SIZE 10

#define SENDER_MAXSIZE 128
#define DATA_MAXSIZE 512

#define SERVER_IPV4_ADDR "127.0.0.1"
#define SERVER_LISTEN_PORT 0

#define MAX_USERNAME_SIZE 128
#define MAX_MSG_SIZE 128

// Messages

typedef struct {
    char sender[SENDER_MAXSIZE];
    char data[DATA_MAXSIZE];
}  message_t;

int prepareMessage(char *sender, char *data, message_t *message)
{
    sprintf(message->sender, "%s", sender);
    sprintf(message->data, "%s", data);
    return 0;
}

/*
 * printMessage(message_t *message)
 *      Prints the contents of a message
 *  Args:
 *      message: message to print
 *  return:
 *      0: Success
 */
int printMessage(message_t *message)
{
    char* buffer;
    buffer = malloc(sizeof(char) * (strlen(message->sender) + strlen(message->data) + 6));
    sprintf(buffer, "<%s>: %s\n" ,message->sender, message->data);
    write(STDOUT, buffer, sizeof(char) * (strlen(message->sender) + strlen(message->data) + 6));
    free(buffer);
    return 0;
}

// Message queue
typedef struct {
    int size;
    message_t *data;
    int current;
} message_queue_t;

/*
 * createMessageQueue(int queue_size, message_queue_t *queue)
 *      Creates the message queue and allocates space
 *  Args:
 *      queue_size: Size of the queue
 *      queue: Queue to initialize
 *  Returns:
 *      0
 */
int createMessageQueue(int queue_size, message_queue_t *queue)
{
    queue->data = calloc(queue_size, sizeof(message_t));
    queue->size = queue_size;
    queue->current = 0;
    return 0;
}

/*
 * deleteQueue(message_queue_t *queue)
 *      Deletes a Queue
 *   Args:
 *      queue: queue to delete
 */
void deleteQueue(message_queue_t *queue)
{
    free(queue->data);
    queue->data = NULL;
}

/*
 * enqueue(message_queue_t *queue, message_t *message)
 *      Adds a queue to a message
 *  Args:
 *      queue:      queue to add
 *      message:    Message to add
 *  Returns:
 *      -1: Failure to queue
 *       0: Success
 */
int enqueue(message_queue_t *queue, message_t *message)
{
    if (queue->current == queue->size)
        return -1;

    memcpy(&queue->data[queue->current], message, sizeof(message_t));
    queue->current++;

    return 0;
}

/*
 * dequeue(message_queue_t *queue, message_t *message)
 *      Adds a queue to a message
 *  Args:
 *      queue:      queue to remove
 *      message:    Message to remove
 *  Returns:
 *      -1: Failure to queue
 *       0: Success
 */
int dequeue(message_queue_t *queue, message_t *message)
{
    if (queue->current == 0)
        return -1;

    memcpy(message, &queue->data[queue->current - 1], sizeof(message_t));
    queue->current--;

    return 0;
}

/*
 * dequeueAll(message_queue_t *queue)
 *      Remove all from queue
 *  Args:
 *      queue: queue to clear
 *  Returns:
 *      0: Success
 */
int dequeueAll(message_queue_t *queue)
{
    queue->current = 0;

    return 0;
}

// Connect peer info
typedef struct {
    int socket;
    struct sockaddr_in addres;

    /* Messages that waiting for send. */
    message_queue_t send_buffer;

    /* Buffered sending message.
     *
     * In case we doesn't send whole message per one call send().
     * And current_sending_byte is a pointer to the part of data that will be send next call.
     */
    message_t sending_buffer;
    size_t current_sending_byte;

    /* The same for the receiving message. */
    message_t receiving_buffer;
    size_t current_receiving_byte;
} peer_t;

/*
 * deletePeer(peer_t *peer)
 *      Deletes a peer
 *  Args:
 *      peer: peer to delete
 */
int deletePeer(peer_t *peer)
{
    close(peer->socket);
    deleteQueue(&peer->send_buffer);
}

/*
 * createPeer(peer_t *peer)
 *      Creates a peer
 *  Args:
 *      peer: peer to create
 */
int createPeer(peer_t *peer)
{
    createMessageQueue(MAX_MESSAGES_BUFFER_SIZE, &peer->send_buffer);

    peer->current_sending_byte   = -1;
    peer->current_receiving_byte = 0;

    return 0;
}

/*
 * peerGetAddr(peer_t *peer)
 *      Gets address of a peer
 *  Args:
 *      peer: peer to get address of
 */
char *peerGetAddr(peer_t *peer)
{
    static char ret[INET_ADDRSTRLEN + 10];
    char peer_ipv4_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &peer->addres.sin_addr, peer_ipv4_str, INET_ADDRSTRLEN);
    sprintf(ret, "%s:%d", peer_ipv4_str, peer->addres.sin_port);

    return ret;
}

/*
 * peerAdd(peer_t *peer, message_t *message)
 *      Adds message to a peers queue
 *  Args:
 *      peer: peer to send to
 *      message: Message to add to peer
 */
int peerAdd(peer_t *peer, message_t *message)
{
    return enqueue(&peer->send_buffer, message);
}

/*
 * receivePeerMsg(peer_t *peer, int (*message_handler)(message_t *))
 *      Receive message from peer and handle it with message_handler().
 *  Args:
 *      peer: Peer to receive at
 *      message_t: Local message function to function with
 *  Returns:
 *      -1: Failure
 *       0: Success
 */
int receivePeerMsg(peer_t *peer, int (*message_handler)(message_t *))
{
    size_t len_to_receive;
    ssize_t received_count;
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
                //
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


    return 0;
}

/*
 * sendPeerMsg(peer_t *peer)
 *      Sends a peer a message
 *  Args:
 *      peer: Peer to send message too
 *  Returns:
 *      -1: Failure
 *       0: Success
 */
int sendPeerMsg(peer_t *peer)
{
    size_t len_to_send;
    ssize_t send_count;
    size_t send_total = 0;
    do {
        // If sending message has completely sent and there are messages in queue, why not send them?
        if (peer->current_sending_byte < 0 || peer->current_sending_byte >= sizeof(peer->sending_buffer)) {
            if (dequeue(&peer->send_buffer, &peer->sending_buffer) != 0) {
                peer->current_sending_byte = -1;
                break;
            }
            peer->current_sending_byte = 0;
        }

        // Count bytes to send.
        len_to_send = sizeof(peer->sending_buffer) - peer->current_sending_byte;
        if (len_to_send > MAX_SEND_SIZE)
            len_to_send = MAX_SEND_SIZE;

        send_count = send(peer->socket, (char *)&peer->sending_buffer + peer->current_sending_byte, len_to_send, 0);
        if (send_count < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                //
            }
            else {
                write(STDERR, strerror(errno), strlen(strerror(errno)) * sizeof(char));
                return -1;
            }
        }
            // we have read as many as possible
        else if (send_count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            break;
        }
        else if (send_count == 0) {
            break;
        }
        else if (send_count > 0) {
            peer->current_sending_byte += send_count;
            send_total += send_count;
        }
    } while (send_count > 0);

    return 0;
}

/*
 * readSTDIN(char *read_buffer, size_t max_len)
 *      Reads text from stdin
 *  Args:
 *      readBuffer:  Buffer to store to
 *      maxLength: Length of the buffer
 *  Returns:
 *      -1: Failure
 *       0: Success
 */
int readSTDIN(char *readBuffer,  size_t maxLength)
{
    memset(readBuffer, 0, maxLength);

    ssize_t read_count = 0;
    ssize_t total_read = 0;

    do {
        read_count = read(STDIN_FILENO, readBuffer, maxLength);
        if (read_count < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            write(STDERR, strerror(errno), strlen(strerror(errno)) * sizeof(char));
            return -1;
        }
        else if (read_count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            break;
        }
        else if (read_count > 0) {
            total_read += read_count;
            if (total_read > maxLength) {
                fflush(STDIN_FILENO);
                break;
            }
        }
    } while (read_count > 0);

    size_t len = strlen(readBuffer);
    if (len > 0 && readBuffer[len - 1] == '\n')
        readBuffer[len - 1] = '\0';
    return 0;
}
#endif
