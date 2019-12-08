#ifndef PROGRAM_5_CCLIENT_H
#define PROGRAM_5_CCLIENT_H

#include <netinet/in.h>
/*
 * struct conn_t
 *
 *
 */
typedef struct
{
    int sockfd;
    struct sockaddr_in addr;
} conn_t;
#endif
