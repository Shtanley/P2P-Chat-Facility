#include "UDPsockets.h"

MessageList *hostMsg;  // host message list
MessageList *remoteMsg;    // remote message list
bool shutdownFlag;  // flag to shutdown the program
pthread_mutex_t shutdown_mutex; // mutex for shutdown flag
char escapeChar[] = ESCAPE_CHAR;    // escape character

// -------------------------------------- Helper functions --------------------------------------
void initShutdownFlag() {   // initialize shutdown flag
    shutdownFlag = false;
    pthread_mutex_init(&shutdown_mutex, NULL);
}

void destroyShutdownFlag() {    // destroy shutdown flag
    pthread_mutex_destroy(&shutdown_mutex);
}

bool getShutdownFlag() {    // get shutdown flag
    bool flag;
    pthread_mutex_lock(&shutdown_mutex);
    flag = shutdownFlag;
    pthread_mutex_unlock(&shutdown_mutex);
    return flag;
}

void setShutdownFlag(bool value) {  // set shutdown flag
    pthread_mutex_lock(&shutdown_mutex);
    shutdownFlag = value;
    pthread_mutex_unlock(&shutdown_mutex);
}

bool isEscapeChar(char *msg) { // check if escape character present
    if(strcmp(msg, escapeChar) == 0) {
        setShutdownFlag(true);
        return true;
    }
    else
        return false;
}

// -------------------------------------- Socket functions --------------------------------------

void *readUserInput(void *threadID) {   // read user input from stdin
    char *threadName = (char *)threadID;
    char buff[MAX_MESSAGE_LENGTH];

    // Use poll to check for user input
    // (Troan & By, 2021)
    struct pollfd pfds; // poll file descriptor
    pfds.fd = STDIN_FILENO; // stdin
    pfds.events = POLLIN;   // input event

    while(!getShutdownFlag()) {
        if(poll(&pfds, 1, -1)) {
            int result = read(STDIN_FILENO, buff, MAX_MESSAGE_LENGTH);

            if(result == 0) // error
                fprintf(stderr, "Thread (%s) ERROR: User input has an error!\n", threadName);

            else if(result == 1)    // enter key pressed
                memset(buff, 0, sizeof(buff));

            else if(result >= MAX_MESSAGE_LENGTH * sizeof(char))    // buffer overflow
                fprintf(stderr, "Thread (%s) ERROR: User input has an undefined behaviour!\n", threadName);

            else {   // valid input
                char *input = (char *)malloc(result);   // allocate memory for input
                int size = result/sizeof(char); // get size of input in bytes
                strncpy(input, buff, size); // copy input to buffer
                input[size - 1] = '\0'; // add null terminator
                consume(hostMsg, input, size, threadName); // add input to hostMsg list
                if(isEscapeChar(input)) // check if escape character present
                    consume(remoteMsg, escapeChar, 1, threadName);
                free(input);    // free allocated memory
            }
            // clear buffer
            memset(buff, 0, sizeof(buff));
        }
    }

    pthread_exit(NULL);
}

// -- Displaying received remote messages --
void *writeUserInput(void *threadId) {
    char *threadName = (char *)threadId;
    char buf[MAX_MESSAGE_LENGTH + 1];

    // Fetch from remote messages and print via write()
    while(!getShutdownFlag()) {
        produce(remoteMsg, buf, MAX_MESSAGE_LENGTH, threadName);
        buf[MAX_MESSAGE_LENGTH] = '\n';
        if(strcmp(buf, escapeChar) == 0)
            continue;

        else
            write(STDOUT_FILENO, buf, sizeof buf);

        // resetting buffer of bytes
        memset(buf, 0, sizeof(buf));
    }

    // Complete the thread method
    pthread_exit(NULL);
}

void *sendData(void *args) {    // send data to socket using sendto()
    // get arguments for socket
    struct ThreadArgs *threadArgs = (struct ThreadArgs *)args;
    char *threadName = threadArgs->threadId;
    char *hostName = threadArgs->hostName;
    char *port = threadArgs->port;

    // create socket variables
    struct addrinfo *rp;    // pointer to result
    struct addrinfo hints;  // hints for getaddrinfo()
    struct addrinfo *remoteServer;  // remote server address info
    int result; // result of getaddrinfo()
    int socketFD;   // socket file descriptor

    // Goto: https://github.com/Microsoft/vscode-cpptools/issues/2025
    // for socket.h include errors
    memset(&hints, 0, sizeof(hints));   // clear hints
    hints.ai_family = AF_UNSPEC;    // IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; // UDP socket

    // create socket
    result = getaddrinfo(hostName, port, &hints, &remoteServer);
    if(result != 0) {
        fprintf(stderr, "(%s) sendData() ERROR: getaddrinfo() failed: %s\n", threadName, gai_strerror(result));
        freeaddrinfo(remoteServer); // free allocated memory
        exit(EXIT_FAILURE);
    }

    for(rp = remoteServer; rp != NULL; rp = rp->ai_next) {
        socketFD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(socketFD == -1) {
            perror("Thread (%s) ERROR: socket() failed");
            continue;
        }
        break;
    }

    if(rp == NULL) {
        fprintf(stderr, "Thread (%s) ERROR: Could not create socket!\n", threadName);
        freeaddrinfo(remoteServer); // free allocated memory
        exit(EXIT_FAILURE);
    }

    // send data
    char msg[MAX_MESSAGE_LENGTH];
    while(!getShutdownFlag()) {
        produce(hostMsg, msg, MAX_MESSAGE_LENGTH, threadName);
        result = sendto(socketFD, msg, MAX_MESSAGE_LENGTH, 0, rp->ai_addr, rp->ai_addrlen);
        if(result == -1)
            fprintf(stderr, "Thread (%s) ERROR: sendto() failed!\n", threadName);
        else {
            if(isEscapeChar(msg)) // check if escape character present
                consume(remoteMsg, escapeChar, 1, threadName);
        }
        // memset(msg, 0, sizeof(msg));    // clear buffer
    }

    freeaddrinfo(remoteServer); // free allocated memory
    pthread_exit(NULL);
}

void *recvData(void *args) { // receive data from socket using recvfrom()
    // get arguments for socket
    struct ThreadArgs *newArgs = (struct ThreadArgs *)args;
    char *threadName = newArgs->threadId;
    // char *hostName = threadArgs->hostName;
    char *userPort = newArgs->port;

    // Declaring and initialzing socket vars
    struct addrinfo *p; // pointer to result
    struct addrinfo *serverInfo;    // user server address info
    struct addrinfo hints;  // hints for getaddrinfo()

    int socketFD; // socket file descriptor
    int result; // result of getaddrinfo()

    // Goto: https://github.com/Microsoft/vscode-cpptools/issues/2025
    // for socket.h include errors
    memset(&hints, 0, sizeof(hints));   // clear hints
    hints.ai_family = AF_UNSPEC;    // IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; // UDP socket
    hints.ai_flags = AI_PASSIVE;    // fill in my IP for me

    // create socket and bind to port
    result = getaddrinfo(NULL, userPort, &hints, &serverInfo);
    if(result != 0) {
        fprintf(stderr, "(%s) recieveData() ERROR: getaddrinfo() failed: %s\n", threadName, gai_strerror(result));
        freeaddrinfo(serverInfo);
        exit(EXIT_FAILURE);
    }

    for(p = serverInfo; p != NULL; p = p->ai_next) {
        if((socketFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("recieveData() ERROR: socket() failed");
            continue;
        }

        if(bind(socketFD, p->ai_addr, p->ai_addrlen) == -1) {
            close(socketFD);
            perror("recieveData() ERROR: bind() failed");
            continue;
        }

        break;
    }

    if(p == NULL) {
        fprintf(stderr, "Thread (%s) ERROR: Could not create socket!\n", threadName);
        freeaddrinfo(serverInfo); // free allocated memory
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(serverInfo);   // free allocated memory

    // receive data
    char msg[MAX_MESSAGE_LENGTH];
    struct sockaddr_storage client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Track user messages and consume them to remote messages when they appear
    while(!getShutdownFlag()) {
        result = recvfrom(socketFD, msg, MAX_MESSAGE_LENGTH - 1, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (result == -1) 
            printf("Thread (%s) ERROR: recvfrom() failed!\n", threadName);
        else {
            consume(remoteMsg, msg, MAX_MESSAGE_LENGTH, threadName);
            if(isEscapeChar(msg)){
                printf("Remote connection terminated, press [Enter] to continue\n");
                consume(hostMsg, escapeChar, 1, threadName);
            }
            memset(msg, 0, sizeof msg);
        }
    }

    // Complete thread method
    pthread_exit(NULL);
}