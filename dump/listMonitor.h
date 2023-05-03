// Required threads (in each of the processes):
// • One of the threads does nothing other than await input from the keyboard.
// • Another thread does nothing other than await a UDP datagram.
// • There will also be a thread which prints characters to the screen.
// • Finally a thread which sends data to the remote UNIX process over the network using UDP

// The threads that await input from the keyboard and await a UDP datagram will be
// referred to as the “producer” threads. The thread that prints characters to the screen
// will be referred to as the “consumer” thread. The thread that sends data to the remote
// UNIX process over the network using UDP will be referred to as the “sender” thread.

// The producer threads will be responsible for reading data from the keyboard and
// sending it to the sender thread. The sender thread will be responsible for sending the
// data to the remote UNIX process over the network using UDP. The consumer thread
// will be responsible for reading data from the sender thread and printing it to the screen.

#ifndef LISTMONITOR_H
#define LISTMONITOR_H
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_NUM_MESSAGES 10
#define MAX_MESSAGE_LENGTH 500

// #define NUM_THREADS_IO 2 //input and output
// #define NUM_THREADS_SR 2 //sender and reciever
#define ESCAPE_CHAR "!" //escape character
#define SIZE 1024

typedef struct MessageList_s MessageList;
struct MessageList_s {
    List *messages;
    int maxMessages;    // Max number of messages

    // For one process access
    pthread_mutex_t access;

    // To notify blocked producers
    pthread_cond_t availableMessage;

    // To notify blocked consumers
    pthread_cond_t spaceAvailable;
};

void freeString(void *item);   // free string

MessageList createMsgList();    // create message list

MessageList *createMsgListPtr();   // create message list pointer

void destroyMsgListPtr(MessageList *mList); // destroy message list pointer

void consume(MessageList *mList, char *message, int messageLength, char *threadId); // consume message

void produce(MessageList *mList, char *buff, int sizeOfBuff, char *threadId); // produce message

// enum thread_index_id{
//     keyboard = 0,
//     sender = 1,
//     screen = 2,
//     reciever = 3
// };

// pthread_t thread[NUM_THREADS_IO + NUM_THREADS_SR];

// #define NUM_THREADS_IO 2 //input and output
// #define NUM_THREADS_SR 2 //sender and reciever
// #define ESCAPE_CHAR "!\n\0" //escape character

// pthread_mutex_t msg_mutex;
// pthread_cond_t msg_recieved;
// pthread_cond_t msg_ready_to_output;

// pthread_mutex_t sent_mutex;
// pthread_cond_t msg_ready_to_send;

// struct screen_thread{
//     List* list_m; // shared list for thread
// };

// // data for sender and reciever to communicate
// struct send_recv_data_thread{ 
//     List* shared_data;  // shared list for thread
//     struct sockaddr_in* client_addr;    // client address
//     int socket; // socket
//     char* msg;  // message to be sent
//     int msg_len;    // length of the message
//     int size;    // size of the client_addr
// };

// pthread_t* get_threads(); // returns ID of threads

// int get_keyboard_id(); // returns ID of the keyboard thread

// int get_send_id(); // returns ID of the sender thread

// int get_recv_id(); // returns ID of the receieved thread 

// int get_screen_id(); // returns ID of the screen thread

// void* keyboard_input_thread(void* data); // adds the input to the list of messages that need to be sent to the remote s-talk client

// void* send_thread(void* data); // takes each message off this list and send it over the network to the remote client 

// void* screen_output_thread(void* data); // takes each message off this list and output it to the screen

// void* recv_thread(void* data); // places the message in the list of message to be prined to the local screen

#endif

// References:

// 1. Beej's Guide to Network Programming. (n.d.). Retrieved February 25, 2023, 
//    from https://beej.us/guide/bgnet/html/#client-server-background 

// 2. Troan, E., &amp; By. (2021, June 22). Multiplexed I/0 with poll() tutorial. 
//    Linux Today. Retrieved February 22, 2023, 
//    from https://www.linuxtoday.com/blog/multiplexed-i0-with-poll/ 