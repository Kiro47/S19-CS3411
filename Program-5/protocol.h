#ifndef PROGRAM_5_PROTOCOL_H
#define PROGRAM_5_PROTOCOL_H

#ifndef MAX_USERNAME_SIZE
#define MAX_USERNAME_SIZE 128
#endif

#ifndef MAX_MSG_SIZE
#define MAX_MSG_SIZE 128
#endif

/*
 * Cruft from original attempt, was a simplistic idea, but did it fail hard.
 */

enum connectionType{
    ESTCONN, // Establish client connection to server
    SUCCONN, // Connection succesfully established
    BADCONN, // Connection attempt failed
    SERRECV, // Server should receive this message to distribute
    CLIERECV, // Single client should receive this message
    CLOCONN // Close connection
    };

typedef struct
{
    enum connectionType connection_type;
    char userName[MAX_USERNAME_SIZE];
    char msg[MAX_MSG_SIZE];
    int channel; // might not be needed, designed for thread user
} msgProto;

int validateProto(msgProto *packet);

#endif
