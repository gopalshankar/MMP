/* 
 * Message Queue Multicast Implementation
 * CS551 Group 21  
 * 
 * Multicast Message Queue Kernel Functions and Structures
 * 
 */
 #include <minix/libmsgque.h>
 #include <minix/ipc.h>
 #include "msgque_const.h"

 /* 
  * Linked list maintaining all the message posted by msend
  * 
  * Elements are removed by mrecv() if there are no more
  * recievers pending to read the message.
  * 
  * There exist maximum of MQ_MAX_MESSAGES MsgNode per MQueue.
  * 
  */
struct MsgNode {
	unsigned long messageNo;
	char *message;   
	int len;
	struct MsgNode *next;
};
  
/* This structure hold all MQ users who are registered
 * 
 */
struct MQUser {   /* Dynamically allocated */
	unsigned long messageNo; 	/* This is the last message recieve used */
	int proc_nr; 	/* Receiver's process number */
	int state;	/* Blocked or Active */
	int type; 	/* Sender or reciever = has meaning only with 'state' */
	message args;    /* User only when unpaused */
	struct MQUser *next; /* If many are waiting to read message */
};

struct MQueue{	  
	int token;	/* Unique identifier for this message queue, 
	             * user gives this */
	int queueLen;
	unsigned long msgCounter; /* next free no for new message */
	struct MsgNode *msgHead; 
	struct MsgNode *msgTail; 
	struct MQUser *userHead;
} ;
#define INVALID_MQ( mq, tok ) (mq < &mQueues_[0] || mq > &mQueues_[MQ_MAX_MSGQUES] || mq->token != tok )
	
 
