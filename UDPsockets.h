// UDP sockets Stream server by Beej's Guide to Network Programming

#ifndef _UDPSOCKETS_H_
#define _UDPSOCKETS_H_
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/poll.h>
#include "UDPthreads.h"

struct ThreadArgs{
    char *hostName;
    char *port;
    char *threadId;
    // int sock;   // socket
    // struct sockaddr_in *client_addr;
    // int size;   // size of the client_addr
    // List *shared_data;  // shared list for thread
};

// Global variables
extern MessageList *hostMsg;  // host message list
extern MessageList *remoteMsg;    // remote message list
extern bool shutdownFlag;  // flag to shutdown the program
extern pthread_mutex_t shutdown_mutex; // mutex for shutdown flag
extern char escapeChar[];    // escape character

// Helper functions
void initShutdownFlag();

void destroyShutdownFlag();

bool getShutdownFlag();

void setShutdownFlag(bool flag);

bool isEscapeChar(char *msg);

// Socket functions
void *readUserInput(void *threadID);

void *writeUserInput(void *threadID);

void *sendData(void *args);

void *recvData(void *args);

#endif

// References:

// 1. Beej's Guide to Network Programming. (n.d.). Retrieved February 25, 2023, 
//    from https://beej.us/guide/bgnet/html/#client-server-background 

// 2. Troan, E., &amp; By. (2021, June 22). Multiplexed I/0 with poll() tutorial. 
//    Linux Today. Retrieved February 22, 2023, 
//    from https://www.linuxtoday.com/blog/multiplexed-i0-with-poll/ 