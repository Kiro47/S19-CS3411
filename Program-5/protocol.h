#ifndef PROGRAM_5_PROTOCOL_H
#define PROGRAM_5_PROTOCOL_H

#define MAX_MSG_SIZE 128
#define MAX_USERNAME_SIZE 128

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
    enum connectionType connType;
    char userName[MAX_USERNAME_SIZE];
    char msg[MAX_MSG_SIZE];
    int channel; // might not be needed, designed for thread user
} msgProto;

int validateProto(msgProto *packet);

#endif
