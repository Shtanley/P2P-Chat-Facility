#include "UDPthreads.h"
#include "UDPsockets.h"

// Global variables
// MessageList *hostMsg;  // host message list
// MessageList *remoteMsg;    // remote message list

int main(int argc, char *argv[]){
    // Check arguments
    if(argc != 4){
        printf("Usage: ./s-talk <port> <remote-host> <remote-port>");
        return 0;
    }
    
    char *threadIDs[4] = {
        "Sending",
        "Receiving",
        "Reading",
        "Displaying"
    };

    initShutdownFlag();
    hostMsg = createMsgListPtr();
    remoteMsg = createMsgListPtr();

    // Create threads for sending and receiving data
    struct ThreadArgs sendingArgs;
    sendingArgs.threadId = threadIDs[1];
    sendingArgs.hostName = argv[2];
    sendingArgs.port = argv[3];

    struct ThreadArgs receivingArgs;
    receivingArgs.threadId = threadIDs[0];
    receivingArgs.hostName = "";
    receivingArgs.port = argv[1];

    pthread_t sendingThread, receivingThread, readUserMsg, writeUserMsg;  // thread variables

    pthread_create(&sendingThread, NULL, sendData, (void *)&sendingArgs);
    pthread_create(&receivingThread, NULL, recvData, (void *)&receivingArgs);
    pthread_create(&readUserMsg, NULL, readUserInput, (void *)threadIDs[2]);
    pthread_create(&writeUserMsg, NULL, writeUserInput, (void *)threadIDs[3]);

    pthread_join(sendingThread, NULL);
    pthread_join(receivingThread, NULL);
    pthread_join(readUserMsg, NULL);
    pthread_join(writeUserMsg, NULL);

    destroyShutdownFlag();

    destroyMsgListPtr(hostMsg);
    destroyMsgListPtr(remoteMsg);
    return 0;
}
