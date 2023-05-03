// In this assignment, you will be dealing with processes/threads on two levels. First, you
// will have two UNIX processes. Each one is started by one of the people who want to talk
// (as a result of executing s-talk). Second, within each s-talk process a pthreads
// environment will be running four threads that you will write.

// The first process is the sender. It will read from the keyboard and send the data to the
// remote peer. The second process is the receiver. It will receive data from the remote
// peer and write it to the screen. The two processes will communicate with each other

// using a shared memory segment. The shared memory segment will contain a single integer
// that will be used to signal the two processes to shut down. The two processes will
// communicate with each other using a single pipe. The pipe will be used to send a single

#include "UDPthreads.h"

void consume(MessageList *mList, char *message, int messageLength, char *threadId){
    
    pthread_mutex_lock(&mList->access);

    // Wait if there is no space to produce a new message
    while(mList->messages->num_nodes >= MAX_NUM_MESSAGES) {
        pthread_cond_wait(&mList->spaceAvailable, &mList->access);
    }
    char *newMessage = (char *)malloc(messageLength);
    strncpy(newMessage, message, messageLength);

    // Consume the new message;
    int result = List_prepend(mList->messages, newMessage);
    if(result == -1)
        fprintf(stderr, "Thread (%s) ERROR: Available nodes have been exhausted\n", threadId);

    // Signal process waiting on available messages
    pthread_cond_signal(&mList->availableMessage);
    pthread_mutex_unlock(&mList->access);
}

void produce(MessageList *mList, char *buff, int sizeOfBuff, char *threadId) {

    pthread_mutex_lock(&mList->access);

    // Wait if there are no messages to consume
    while(mList->messages->num_nodes == 0) {
        pthread_cond_wait(&mList->availableMessage, &mList->access);
    }
    // Produce the message
    char *message = (char *)List_trim(mList->messages);
    if(message != NULL) {
        strncpy(buff, message, sizeOfBuff);
        free(message);
    }
    else
        fprintf(stderr, "Thread (%s) ERROR: No message to produce\n", threadId);
    
    // Signal process waiting on available space
    pthread_cond_signal(&mList->spaceAvailable);
    pthread_mutex_unlock(&mList->access);
}

// ----------------------------------- MessageList methods -----------------------------------

MessageList createMsgList(){
    MessageList mList;
    memset(&mList, 0, sizeof mList);
    mList.messages = List_create();
    mList.maxMessages = MAX_NUM_MESSAGES;
    pthread_mutex_init(&mList.access, NULL);
    pthread_cond_init(&mList.availableMessage, NULL);
    pthread_cond_init(&mList.spaceAvailable, NULL);
    return mList;
}

MessageList *createMsgListPtr(){
    MessageList * mListPtr = (MessageList *)malloc(sizeof(struct MessageList_s));
    *mListPtr = createMsgList();
    return mListPtr;
}

void destroyMsgListPtr(MessageList *mList){
    List_free(mList->messages, free);
    pthread_mutex_destroy(&mList->access);
    pthread_cond_destroy(&mList->availableMessage);
    pthread_cond_destroy(&mList->spaceAvailable);
    free(mList);
}