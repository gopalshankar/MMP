/* 
 * Message Queue Multicast Implementation
 * CS551 Group 21  
 * 
 * Multicast Message Queue Kernel Functions and Structures
 * 
 */
 #include <minix/libmsgque.h>
 #include "msgque_const.h"

/* This structure hold all MQ users who are registered
 * 
 */
typedef struct MQUser_t{   /* Dynamically allocated */
	int messageNo; 			 /* This is the last message recieve used */
	int proc_nr; 			 /* Receiver's process number */
	int state;				 /* Blocked or Active */
	int type; 				 /* Sender or reciever = has meaning only with 'state' */
	struct MQUser_t *next; /* If many are waiting to read message */
} MQUser;

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
	int messageNo;
	char *message;   
	struct MsgNode *next;
};
  
typedef struct {	  
	int token;	/* Unique identifier for this message queue, 
	             * user gives this */
	int queueLen;
	struct MsgNode *msgHead; 
	MQUser *userHead;
} MQueue;
#define INVALID_MQ( mq, tok ) (mq < mQueue[0] || mq > mQueue[MQ_MAX_MSGQUES] || mq->token != tok )
	
 