#include "listMonitor.h"
#include "unistd.h"


#include <sys/poll.h>
#include <sys/time.h>

// -- Testing synchronization on a messageList -- 
int numTimes = 1;
MessageList * messageList;

void * testConsume(void * id) {
    printf("Starting consumption\n");
    for (int i = 0; i < numTimes; i++) {
        char message[] = "Yo what's poppin";
        consume(messageList, message, strlen(message), (char *)id);
        printf("Successfully consumed message = %s\n", message);
    }
    printf("Finished consumption\n");
    // setShutdown(true);
}

void * testProduce(void * id) {
    printf("Starting production\n");
    for(int i = 0; i < numTimes; i++) {
        char receivedMessage[50];
        produce(messageList, receivedMessage, 50, (char *)id);
        printf("Successfully produced message = %s\n", receivedMessage);
    }
    printf("Finished consumption\n");
    // setShutdown(true);
}
int main() {
    // initializeShutdownLock();
    char * ids[4] = {"Message Printer 1", "Message Printer 2", "Message Consumer 1", "Message Consumer 2"};
    pthread_t messagePrinter, messageConsumer, messagePrinter1, messageConsumer1;
    messageList = createMsgListPtr();

    pthread_create(&messagePrinter, NULL, testProduce, (void *)ids[0]);
    pthread_create(&messageConsumer, NULL, testConsume,(void *)ids[2]);
    pthread_create(&messagePrinter1, NULL, testProduce,(void *)ids[1]);
    pthread_create(&messageConsumer1, NULL, testConsume,(void *)ids[3]);
    
    pthread_join(messagePrinter, NULL);
    pthread_join(messageConsumer, NULL);
    pthread_join(messagePrinter1, NULL);
    pthread_join(messageConsumer1, NULL);

    // destroyShutdownLock();
    destroyMsgListPtr(messageList);
    printf("\n ------ \nFinished test! \n ------ \n");
}