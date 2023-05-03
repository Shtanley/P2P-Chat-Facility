#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/poll.h>
#include "listMonitor.h"

// UDP socket code inspired by beej's guide

// Struct to pass arguments to thread UDP methods
struct UDPThreadArgs {
    char * machineName;
    char * port;
    char * threadId;
};

MessageList * userMessages, * remoteMessages;


// Atomic boolean to determine whether program needs to shutdown or not
bool shutdownProgram;
pthread_mutex_t shutdownLock;
char endSymbol[] = "!";

void initializeShutdownLock(){
    shutdownProgram = false;
    pthread_mutex_init(&shutdownLock, NULL);
}

void destroyShutdownLock(){
    pthread_mutex_destroy(&shutdownLock);
}

// Getters / Setters for atomic shutdown variable
bool getShutdownValue() {
    bool valueToReturn;
    pthread_mutex_lock(&shutdownLock);
    valueToReturn = shutdownProgram;
    pthread_mutex_unlock(&shutdownLock);
    return valueToReturn;
}

void setShutdown(bool value){
    pthread_mutex_lock(&shutdownLock);
    shutdownProgram = value;
    pthread_mutex_unlock(&shutdownLock);
}

// Checks if a ! symbol is detected in the program
bool checkIfStopped(char * buf) {
    if(strcmp(buf, endSymbol) == 0) {
        setShutdown(true);
        return true;
    }
    else {
        return false;
    }
}

// ---- Thread methods ---- 

// -- Reading user messages --
void * readUserInput(void * threadId) {

    char * threadName = (char *)threadId;
    char buf[MAX_MESSAGE_LENGTH];
   
    // Use poll for non-blocking I/O
    // From: https://www.linuxtoday.com/blog/multiplexed-i0-with-poll.html
    struct pollfd pfds;
    pfds.fd = STDIN_FILENO;
    pfds.events = POLLIN;

    while(!getShutdownValue()) {
        if(poll(&pfds, 1 , -1)){
            int result = read(STDIN_FILENO, buf, MAX_MESSAGE_LENGTH);
            // Error 
            if(result == 0) {
                fprintf(stderr, "Thread %s ERROR: User input has an error!\n", threadName);
            }
            // For instances when an enter is pressed
            else if(result == 1) {
                memset(buf, 0, sizeof buf);
            }
            // Undefined behaviour
            else if(result >= MAX_MESSAGE_LENGTH * sizeof(char)){
                fprintf(stderr, "Thread %s: User input has undefined behaviour!\n", threadName);
            }
            // Truncate the message to a correct size to pass in list
            else {
                char * message = (char *)malloc(result);
                int sizeInInt = (result / sizeof(char));
                strncpy(message, buf, sizeInInt);
                message[sizeInInt - 1] = '\0';
                consume(userMessages, message, sizeInInt, threadName);
                if(checkIfStopped(message)) {
                    consume(remoteMessages, endSymbol, 1, threadName);
                }
                free(message);
            }
            // Reset buffer for every input attempt
            memset(buf, 0, sizeof buf);
        }
    }

    // Complete the thread method
    pthread_exit(NULL);
}

// -- Displaying received remote messages --
void * printRemoteMessage(void * threadId) {
    char * threadName = (char *)threadId;
    char buf[MAX_MESSAGE_LENGTH + 1];

    // Fetch from remote messages and print via write()
    while(!getShutdownValue()) {
        produce(remoteMessages, buf, MAX_MESSAGE_LENGTH, threadName);
        buf[MAX_MESSAGE_LENGTH] = '\n';
        if(strcmp(buf, endSymbol) == 0) {
            continue;
        }
        else {
            write(STDOUT_FILENO, buf, sizeof buf);
        }
        // resetting buffer of bytes
        memset(buf, 0, sizeof(buf));
    }

    // Complete the thread method
    pthread_exit(NULL);
}

// -- Sending input from user -> remote --
void * sendUserMessages(void * args) {
    // Deconstructing the arguments for UDP operations
    struct UDPThreadArgs * newArgs = (struct UDPThreadArgs *)args;
    char * threadName = newArgs->threadId;
    char * remoteHostName = newArgs->machineName;
    char * remoteHostPort = newArgs->port;

    // Declaring and initializing socket vars
    struct addrinfo hints, *remoteServer, *p;
    int sockfd;
    int result;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;


    // Creating a socket given the hostname for sending
    if((result = getaddrinfo(remoteHostName, remoteHostPort, &hints, &remoteServer)) != 0) {
        fprintf(stderr, "getaddrinfo sender: %s\n", gai_strerror(result));
        freeaddrinfo(remoteServer);
        exit(1);
    }

    for(p = remoteServer; p != NULL; p = p->ai_next) {
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("s-talk sender: socket\n");
            continue;
        }
        break;
    }


    if(p == NULL) {
        fprintf(stderr, "s-talk sender: unable to create socket\n");
        freeaddrinfo(remoteServer);
        exit(1);
    }
    
    // Continue to fetch user messages and send them to remote peer
    char message[MAX_MESSAGE_LENGTH]; 
    while(!getShutdownValue()) {
        produce(userMessages, message, MAX_MESSAGE_LENGTH, threadName);
        if(sendto(sockfd, message, strlen(message), 0, p->ai_addr, p->ai_addrlen) == -1){
            fprintf(stderr, "Thread %s ERROR: Unable to send user message %s\n", threadName, message);
        } else {
            if(checkIfStopped(message)){
                consume(remoteMessages, endSymbol, 1, threadName);
            }
        }
    }

    // Freeing allocated data
    freeaddrinfo(remoteServer);
    // Complete the thread method
    pthread_exit(NULL);
}


// -- Awaiting remote messages from peer --
void * listenForRemoteMessages(void * args) {
    // Deconstructing UDP arguments
    struct UDPThreadArgs * newArgs = (struct UDPThreadArgs *)args;
    char * threadName = newArgs->threadId;
    char * userPort = newArgs->port;

    // Declaring and initialzing socket vars
    struct addrinfo hints, *userServerInfo, *p;
    int sockfd;
    int result;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    // Finding the appropriate host socket to listen for remote messages
    if((result = getaddrinfo(NULL, userPort, &hints, &userServerInfo)) != 0) {
        fprintf(stderr, "getaddrinfo listener: %s\n", gai_strerror(result));
        freeaddrinfo(userServerInfo);
        exit(1);
    }

    for(p = userServerInfo; p != NULL; p = p->ai_next) {
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("s-talk listener: socket");
            continue;
        }

        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("s-talk listener: bind");
            continue;
        }

        break;
    }

    if(p == NULL) {
        fprintf(stderr, "s-talk listener: unable to create and bind socket\n");
        freeaddrinfo(userServerInfo);
        exit(1);
    }

    // Freeing unwanted server info
    freeaddrinfo(userServerInfo);

    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    char buf[MAX_MESSAGE_LENGTH];

    // Track user messages and consume them to remote messages when they appear
    while(!getShutdownValue()) {
        if ((recvfrom(sockfd, buf, MAX_MESSAGE_LENGTH - 1, 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                printf("%s: Unable to get message, resulted in -1 bytes\n", threadName);
        }
        else {
            consume(remoteMessages, buf, MAX_MESSAGE_LENGTH, threadName);
            if(checkIfStopped(buf)){
                printf("Remote peer has shutdown, hit [Enter] to terminate program\n");
                consume(userMessages, endSymbol, 1, threadName);
            }
            memset(buf, 0, sizeof buf);
        }
    }

    // Complete thread method
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

    if(argc != 4) {
        printf("Usage: s-talk <port> <remote-hostname> <remote-port>");
        return;
    }

    char * threadIds[4] = {
        "Receiving Messages Thread",
        "Sending User Messages Thread",
        "Reading User Input Thread",
        "Displaying Messages Thread"
    };

    initializeShutdownLock();
    userMessages = createMsgListPtr();
    remoteMessages = createMsgListPtr();

    struct UDPThreadArgs listenArgs, sendingArgs;
    
    listenArgs.machineName = "";
    listenArgs.port = argv[1];
    listenArgs.threadId = threadIds[0];

    sendingArgs.machineName = argv[2];
    sendingArgs.port = argv[3];
    sendingArgs.threadId = threadIds[1];

    pthread_t sendingThread, receivingThread, getUserInput, printMessages;

    pthread_create(&sendingThread, NULL, sendUserMessages, (void *)&sendingArgs);
    pthread_create(&receivingThread, NULL, listenForRemoteMessages,(void *)&listenArgs);
    pthread_create(&getUserInput, NULL, readUserInput,(void *)threadIds[2]);
    pthread_create(&printMessages, NULL, printRemoteMessage,(void *)threadIds[3]);

    pthread_join(sendingThread, NULL);
    pthread_join(receivingThread, NULL);
    pthread_join(getUserInput, NULL);
    pthread_join(printMessages, NULL);

    destroyShutdownLock();

    destroyMsgListPtr(userMessages);
    destroyMsgListPtr(remoteMessages);
    return 0;
}