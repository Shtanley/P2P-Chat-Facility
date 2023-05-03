#include "listMonitor.h"

void freeString(void * item){
    free(item);
}


MessageList createMsgList() {
    MessageList messageList;
    memset(&messageList, 0, sizeof messageList);
    messageList.messages = List_create();
    messageList.maxMessages = MAX_NUM_MESSAGES;
    pthread_mutex_init(&messageList.access, NULL);
    pthread_cond_init(&messageList.availableMessage, NULL);
    pthread_cond_init(&messageList.spaceAvailable, NULL);
    return messageList;
}

MessageList * createMsgListPtr(){
    MessageList * messageListPtr = (MessageList *)malloc(sizeof(struct MessageList_s));
    *messageListPtr = createMsgList();
    return messageListPtr;
}

void destroyMsgListPtr(MessageList * messageList){
    List_free(messageList->messages, freeString);
    pthread_mutex_destroy(&messageList->access);
    pthread_cond_destroy(&messageList->availableMessage);
    pthread_cond_destroy(&messageList->spaceAvailable);
    free(messageList);
}

// -- Consume and Produce methods --

void consume(MessageList * messageList, char * message, int messageLength, char * threadId){
    
    pthread_mutex_lock(&messageList->access);

    // Do a wait if there is no space for the new message
    // - Use a while loop to prevent concurrent consumption
    while(messageList->messages->num_nodes >= MAX_NUM_MESSAGES) {
        pthread_cond_wait(&messageList->spaceAvailable, &messageList->access);
    }
    char * newMessage = (char *)malloc(messageLength);
    strncpy(newMessage, message, messageLength);

    // Consume the new message;
    int result = List_prepend(messageList->messages, newMessage);
    if(result == -1) {
        fprintf(stderr, "Thread %s ERROR: Available nodes have been exhausted\n", threadId);
    }
    // Signal process waiting on available messages
    pthread_cond_signal(&messageList->availableMessage);
    pthread_mutex_unlock(&messageList->access);
}

void produce(MessageList * messageList, char * buf, int sizeOfBuf, char * threadId) {

    pthread_mutex_lock(&messageList->access);

     // Do a wait if there is no available message
     // - Use a while to prevent concurrent productions
    while(messageList->messages->num_nodes <= 0) {
        pthread_cond_wait(&messageList->availableMessage, &messageList->access);
    }

    // Get a new message from the messageList
    char * message = List_trim(messageList->messages);
    if(message != NULL) {
        strncpy(buf, message, sizeOfBuf);
        free(message);
    }
    else {
        fprintf(stderr, "Thread %s ERROR: Unable to produce message unto buffer", threadId);
    }
    // Signal process waiting for available space to add messages
    pthread_cond_signal(&messageList->spaceAvailable);
    pthread_mutex_unlock(&messageList->access);
}
